#include "stdafx.h"
#include "../../xrEngine/igame_persistent.h"
#include "../../xrEngine/environment.h"

//////////////////////////////////////////////////////////////////////////
// tables to calculate view-frustum bounds in world space
// note: D3D uses [0..1] range for Z
static Fvector3		corners [8]			= {
	{ -1, -1,  0.7 },	{ -1, -1, +1},
	{ -1, +1, +1 },		{ -1, +1,  0.7},
	{ +1, +1, +1 },		{ +1, +1,  0.7},
	{ +1, -1, +1 },		{ +1, -1,  0.7}
};
static u16			facetable[16][3]		= {
	{ 3, 2, 1 },  
	{ 3, 1, 0 },		
	{ 7, 6, 5 }, 
	{ 5, 6, 4 },		
	{ 3, 5, 2 },
	{ 4, 2, 5 },		
	{ 1, 6, 7 },
	{ 7, 0, 1 },

	{ 5, 3, 0 },
	{ 7, 5, 0 },

	{ 1, 4, 6 },
	{ 2, 4, 1 },
};

void CRenderTarget::accum_direct		(u32 sub_phase)
{
	// Choose normal code-path or filtered
	phase_accumulator					();
	if (RImplementation.o.sunfilter)	{
		accum_direct_f	(sub_phase);
		return			;
	}

	// *** assume accumulator setted up ***
	light*			fuckingsun			= (light*)RImplementation.Lights.sun._get()	;

	// Common calc for quad-rendering
	u32		Offset;
	u32		C					= color_rgba	(255,255,255,255);
	float	_w					= float			(Device.dwWidth);
	float	_h					= float			(Device.dwHeight);
	Fvector2					p0,p1;
	p0.set						(.5f/_w, .5f/_h);
	p1.set						((_w+.5f)/_w, (_h+.5f)/_h );
	float	d_Z	= EPS_S, d_W = 1.f;

	// Common constants (light-related)
	Fvector		L_dir,L_clr;	float L_spec;
	L_clr.set					(fuckingsun->color.r,fuckingsun->color.g,fuckingsun->color.b);
	L_spec						= u_diffuse2s	(L_clr);
	CastToGSCMatrix(Device.mView).transform_dir	(L_dir,fuckingsun->direction);
	L_dir.normalize				();

	// Perform masking (only once - on the first/near phase)
	RCache.set_CullMode			(CULL_NONE	);
	if (SE_SUN_NEAR==sub_phase)	//.
	{
		// Fill vertex buffer
		FVF::TL* pv					= (FVF::TL*)	RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y);	pv++;
		pv->set						(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y);	pv++;
		pv->set						(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y);	pv++;
		pv->set						(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);
		RCache.set_Geometry			(g_combine);

		// setup
		float	intensity			= 0.3f*fuckingsun->color.r + 0.48f*fuckingsun->color.g + 0.22f*fuckingsun->color.b;
		Fvector	dir					= L_dir;
				dir.normalize().mul	(- _sqrt(intensity+EPS));
		RCache.set_Element			(s_accum_mask->E[SE_MASK_DIRECT]);		// masker
		RCache.set_c				("Ldynamic_dir",		dir.x,dir.y,dir.z,0		);

		// if (stencil>=1 && aref_pass)	stencil = light_id
		RCache.set_ColorWriteEnable	(FALSE		);
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0x01,0xff,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

	// recalculate d_Z, to perform depth-clipping
	Fvector	center_pt;			center_pt.mad	(Device.vCameraPosition,Device.vCameraDirection,ps_r_sun_near);
	CastToGSCMatrix(Device.mFullTransform).transform(center_pt)	;
	d_Z							= center_pt.z	;

	// nv-stencil recompression
	if (RImplementation.o.nvstencil  && (SE_SUN_NEAR==sub_phase))	u_stencil_optimize();	//. driver bug?

	// Perform lighting
	{
		phase_accumulator					()	;
		RCache.set_CullMode					(CULL_NONE);
		RCache.set_ColorWriteEnable			()	;

		// texture adjustment matrix
		float			fTexelOffs			= (.5f / float(RImplementation.o.smapsize));
		float			fRange				= (SE_SUN_NEAR==sub_phase)?ps_r_sun_depth_near_scale:ps_r_sun_depth_far_scale;
		//float			fBias				= (SE_SUN_NEAR==sub_phase)?ps_r_sun_depth_near_bias:ps_r_sun_depth_far_bias;
		//	Use this when triangle culling is not inverted.
		float			fBias				= (SE_SUN_NEAR==sub_phase)?(-ps_r_sun_depth_near_bias):ps_r_sun_depth_far_bias;
		Matrix4x4			m_TexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				fRange,			0.0f,
			0.5f + fTexelOffs,	0.5f + fTexelOffs,	fBias,			1.0f
		};

		// compute xforms
		FPU::m64r			();
		Matrix4x4				xf_invview;		xf_invview.InvertMatrixByMatrix	(Device.mView);

		// shadow xform
		Matrix4x4				m_shadow;
		{
			Matrix4x4			xf_project;		xf_project.Multiply(fuckingsun->X.D.combine, m_TexelAdjust);
			m_shadow.Multiply(xf_project, xf_invview);

			// tsm-bias
			if ((SE_SUN_FAR == sub_phase) && (RImplementation.o.HW_smap))
			{
				Fvector		bias;	bias.mul(L_dir, ps_r_sun_tsm_bias);
				Matrix4x4		bias_t;	bias_t.Translate(bias);
				m_shadow.Multiply(bias_t, m_shadow);
			}
			FPU::m24r();
		}

		// clouds xform
		Matrix4x4				m_clouds_shadow;
		{
			static	float	w_shift		= 0;
			Matrix4x4			m_xform;
			Fvector			direction	= fuckingsun->direction	;
			float	w_dir				= Environment().CurrentEnv->wind_direction	;
			Fvector			normal	;	normal.setHP(w_dir,0);
							w_shift		+=	0.003f*Device.fTimeDelta;
			Fvector			position;	position.set(0,0,0);
			m_xform.BuildCamDir	(position,direction,normal)	;
			Fvector			localnormal;
			XRay::Math::TransformDirByMatrix(m_xform, localnormal, normal);
			localnormal.normalize();
			m_clouds_shadow.Multiply(xf_invview, m_xform)		;
			m_xform = DirectX::XMMatrixScaling				(0.002f,0.002f,1.f)			;
			m_clouds_shadow.Multiply(m_clouds_shadow, m_xform)					;
			m_xform.Translate			(localnormal.mul(w_shift))	;
			m_clouds_shadow.Multiply(m_clouds_shadow, m_xform)					;
		}

		// Make jitter texture
		Fvector2					j0,j1;
		float	scale_X				= float(Device.dwWidth)	/ float(TEX_jitter);
		float	offset				= (.5f / float(TEX_jitter));
		j0.set						(offset,offset);
		j1.set						(scale_X,scale_X).add(offset);

		// Fill vertex buffer
		FVF::TL2uv* pv				= (FVF::TL2uv*) RCache.Vertex.Lock	(4,g_combine_2UV->vb_stride,Offset);
		pv->set						(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y, j0.x, j1.y);	pv++;
		pv->set						(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y, j0.x, j0.y);	pv++;
		pv->set						(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y, j1.x, j1.y);	pv++;
		pv->set						(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y, j1.x, j0.y);	pv++;
		RCache.Vertex.Unlock		(4,g_combine_2UV->vb_stride);
		RCache.set_Geometry			(g_combine_2UV);

		// setup
		RCache.set_Element			(s_accum_direct->E[sub_phase]);
		RCache.set_c				("Ldynamic_dir",		L_dir.x,L_dir.y,L_dir.z,0		);
		RCache.set_c				("Ldynamic_color",		L_clr.x,L_clr.y,L_clr.z,L_spec	);
		RCache.set_c				("m_shadow",			m_shadow						);
		RCache.set_c				("m_sunmask",			m_clouds_shadow					);
		
		// nv-DBT
		float zMin,zMax;
		if (SE_SUN_NEAR==sub_phase)
		{
			zMin = 0;
			zMax = ps_r_sun_near;
		}
		else
		{
			zMin = ps_r_sun_near;
			zMax = ps_r_sun_far;
		}
		center_pt.mad(Device.vCameraPosition,Device.vCameraDirection,zMin);	CastToGSCMatrix(Device.mFullTransform).transform	(center_pt);
		zMin = center_pt.z	;

		center_pt.mad(Device.vCameraPosition,Device.vCameraDirection,zMax);	CastToGSCMatrix(Device.mFullTransform).transform	(center_pt);
		zMax = center_pt.z	;

		if (u_DBT_enable(zMin,zMax))	{
			// z-test always
			HW.pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			HW.pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}

		// Fetch4 : enable
		if (RImplementation.o.HW_smap_FETCH4)	{
			//. we hacked the shader to force smap on S0
#			define FOURCC_GET4  MAKEFOURCC('G','E','T','4') 
			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET4 );
		}

		// setup stencil
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0xff,0x00);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);

		// Fetch4 : disable
		if (RImplementation.o.HW_smap_FETCH4)	{
			//. we hacked the shader to force smap on S0
#			define FOURCC_GET1  MAKEFOURCC('G','E','T','1') 
			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET1 );
		}

		// disable depth bounds
		u_DBT_disable	();

		//	Igor: draw volumetric here
		if ( RImplementation.o.advancedpp&&(ps_r_sun_shafts>0))
			accum_direct_volumetric	(sub_phase, Offset, m_shadow);
	}
}

void CRenderTarget::accum_direct_cascade	( u32 sub_phase, Matrix4x4& xform, Matrix4x4& xform_prev, float fBias )
{
	// Choose normal code-path or filtered
	phase_accumulator					();
	if (RImplementation.o.sunfilter)	{
		accum_direct_f	(sub_phase);
		return			;
	}
	Matrix4x4 mView = (Device.mView);

	// *** assume accumulator setted up ***
	light*			fuckingsun			= (light*)RImplementation.Lights.sun._get()	;

	// Common calc for quad-rendering
	u32		Offset;
	u32		C					= color_rgba	(255,255,255,255);
	float	_w					= float			(Device.dwWidth);
	float	_h					= float			(Device.dwHeight);
	Fvector2					p0,p1;
	p0.set						(.5f/_w, .5f/_h);
	p1.set						((_w+.5f)/_w, (_h+.5f)/_h );
	float	d_Z	= EPS_S, d_W = 1.f;

	// Common constants (light-related)
	Fvector		L_dir,L_clr;	float L_spec;
	L_clr.set					(fuckingsun->color.r,fuckingsun->color.g,fuckingsun->color.b);
	L_spec						= u_diffuse2s	(L_clr);

	XRay::Math::TransformDirByMatrix(mView, L_dir, fuckingsun->direction);
	L_dir.normalize				();

	// Perform masking (only once - on the first/near phase)
	RCache.set_CullMode			(CULL_NONE	);
	if (SE_SUN_NEAR==sub_phase)	//.
	{
		// Fill vertex buffer
		FVF::TL* pv					= (FVF::TL*)	RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y);	pv++;
		pv->set						(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y);	pv++;
		pv->set						(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y);	pv++;
		pv->set						(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);
		RCache.set_Geometry			(g_combine);

		// setup
		float	intensity			= 0.3f*fuckingsun->color.r + 0.48f*fuckingsun->color.g + 0.22f*fuckingsun->color.b;
		Fvector	dir					= L_dir;
				dir.normalize().mul	(-_sqrt(intensity+EPS));
		RCache.set_Element			(s_accum_mask->E[SE_MASK_DIRECT]);		// masker
		RCache.set_c				("Ldynamic_dir",		dir.x,dir.y,dir.z,0		);

		// if (stencil>=1 && aref_pass)	stencil = light_id
		RCache.set_ColorWriteEnable	(FALSE		);
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0x01,0xff,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

	// recalculate d_Z, to perform depth-clipping
	Fvector	center_pt;			center_pt.mad	(Device.vCameraPosition,Device.vCameraDirection,ps_r_sun_near);
	CastToGSCMatrix(Device.mFullTransform).transform(center_pt)	;
	d_Z							= center_pt.z	;

	// nv-stencil recompression
	if (RImplementation.o.nvstencil  && (SE_SUN_NEAR==sub_phase))	u_stencil_optimize();	//. driver bug?

	// Perform lighting
	{
		phase_accumulator					()	;
		RCache.set_CullMode					(CULL_CCW); //******************************************************************
		RCache.set_ColorWriteEnable			()	;

		// texture adjustment matrix
		float			fTexelOffs			= (0.5f / float(RImplementation.o.smapsize));
		float			fRange				= (SE_SUN_NEAR==sub_phase)?ps_r_sun_depth_near_scale:ps_r_sun_depth_far_scale;
		//	Use this when triangle culling is not inverted.
//		float			fBias				= (SE_SUN_NEAR==sub_phase)?(-ps_r_sun_depth_near_bias):ps_r_sun_depth_far_bias;
		Matrix4x4			m_TexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				fRange,			0.0f,
			0.5f + fTexelOffs,	0.5f + fTexelOffs,	fBias,			1.0f
		};

		// compute xforms
		FPU::m64r			();
		Matrix4x4				xf_invview;		xf_invview.InvertMatrixByMatrix	(mView)	;

		// shadow xform
		Matrix4x4				m_shadow;
		{
			Matrix4x4			xf_project;		xf_project.Multiply		(fuckingsun->X.D.combine, m_TexelAdjust);
			m_shadow.Multiply	(xf_invview, xf_project);

			// tsm-bias
			if ( (SE_SUN_FAR == sub_phase) && (RImplementation.o.HW_smap) )
			{
				Fvector		bias;	bias.mul		(L_dir,ps_r_sun_tsm_bias);
				Matrix4x4		bias_t;	bias_t.Translate(bias);
				m_shadow.Multiply	(bias_t, m_shadow);
			}
			FPU::m24r		();
		}

		// clouds xform
		Matrix4x4				m_clouds_shadow;
		{
			static	float	w_shift		= 0;
			Matrix4x4			m_xform;
			Fvector			direction	= fuckingsun->direction	;
			float	w_dir				= Environment().CurrentEnv->wind_direction	;
			Fvector			normal	;	normal.setHP(w_dir,0);
							w_shift		+=	0.003f*Device.fTimeDelta;
			Fvector			position;	position.set(0,0,0);
			m_xform.BuildCamDir	(position,direction,normal)	;
			Fvector			localnormal;
			XRay::Math::TransformDirByMatrix(m_xform, localnormal, normal);
			localnormal.normalize();
			m_clouds_shadow.Multiply(xf_invview, m_xform)		;
			m_xform = DirectX::XMMatrixScaling(0.002f,0.002f,1.f)			;
			m_clouds_shadow.Multiply(m_clouds_shadow, m_xform);
			m_xform.Translate			(localnormal.mul(w_shift))	;
			m_clouds_shadow.Multiply(m_clouds_shadow, m_xform);
		}

		Matrix4x4			m_Texgen;
		m_Texgen.Identity();
 		RCache.xforms.set_W( m_Texgen );
 		RCache.xforms.set_V( (Device.mView ));
 		RCache.xforms.set_P( (Device.mProject ));
 		u_compute_texgen_screen	( m_Texgen );

		// Make jitter texture
		Fvector2					j0,j1;
		float	scale_X				= float(Device.dwWidth)	/ float(TEX_jitter);
		float	offset				= (.5f / float(TEX_jitter));
		j0.set						(offset,offset);
		j1.set						(scale_X,scale_X).add(offset);

		// Fill vertex buffer
		u32		i_offset;
		{
			u16*	pib					= RCache.Index.Lock	(sizeof(facetable)/sizeof(u16),i_offset);
            std::memcpy(pib,&facetable,sizeof(facetable));
			RCache.Index.Unlock			(sizeof(facetable)/sizeof(u16));

			//corners

			u32 ver_count = sizeof(corners)/ sizeof(Fvector3);
			FVF::L* pv				= (FVF::L*)	RCache.Vertex.Lock	( ver_count,g_combine_cuboid.stride(),Offset);
			

			Matrix4x4 inv_XDcombine;
			if(sub_phase == SE_SUN_FAR )
				inv_XDcombine.InvertMatrixByMatrix(xform_prev);
			else
				inv_XDcombine.InvertMatrixByMatrix(xform);
				

			for ( u32 i = 0; i < ver_count; ++i )
			{
				Fvector3 tmp_vec;
				XRay::Math::TransformDirByMatrix(inv_XDcombine, tmp_vec, corners[i]);
				pv->set						(tmp_vec, C);	
				pv++;
			}
			RCache.Vertex.Unlock		(ver_count,g_combine_cuboid.stride());
		}

		RCache.set_Geometry			(g_combine_cuboid);

		// setup
		RCache.set_Element			(s_accum_direct_cascade->E[sub_phase]);

		RCache.set_c				("m_texgen",			m_Texgen);
		RCache.set_c				("Ldynamic_dir",		L_dir.x,L_dir.y,L_dir.z,0		);
		RCache.set_c				("Ldynamic_color",		L_clr.x,L_clr.y,L_clr.z,L_spec	);
		RCache.set_c				("m_shadow",			m_shadow						);
		RCache.set_c				("m_sunmask",			m_clouds_shadow					);

		// Pass view vector projected in shadow space to far pixel shader
		// Needed for shadow fading.
		if(sub_phase == SE_SUN_FAR)
		{
			Fvector3 view_viewspace;	view_viewspace.set( 0, 0, 1 );

			XRay::Math::TransformDirByMatrix(m_shadow, view_viewspace);
			Fvector4 view_projlightspace;
			view_projlightspace.set( view_viewspace.x, view_viewspace.y, 0, 0 );
			view_projlightspace.normalize();

			RCache.set_c				("view_shadow_proj",	view_projlightspace);
		}
		Matrix4x4 &mTransform = (Device.mFullTransform);
		// nv-DBT
		float zMin,zMax;
		if (SE_SUN_NEAR==sub_phase)
		{
			zMin = 0;
			zMax = ps_r_sun_near;
		}
		else
		{
			zMin = ps_r_sun_near;
			zMax = ps_r_sun_far;
		}
		center_pt.mad(Device.vCameraPosition,Device.vCameraDirection,zMin);	TransformByMatrix(mTransform, center_pt);
		zMin = center_pt.z	;

		center_pt.mad(Device.vCameraPosition,Device.vCameraDirection,zMax);	TransformByMatrix(mTransform, center_pt);
		zMax = center_pt.z	;

		if (u_DBT_enable(zMin,zMax))	
		{
			// z-test always
			HW.pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			HW.pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}

		// Enable Z function only for near and middle cascades, the far one is restricted by only stencil.
		if( (SE_SUN_NEAR==sub_phase || SE_SUN_MIDDLE==sub_phase) )
			HW.pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
		else
		{
			if (!ps_r_flags.is(R_FLAG_SUN_ZCULLING))
				HW.pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			else
				HW.pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
		}

		// Fetch4 : enable
		if (RImplementation.o.HW_smap_FETCH4)	{
			//. we hacked the shader to force smap on S0
#			define FOURCC_GET4  MAKEFOURCC('G','E','T','4') 
			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET4 );
		}

		// setup stencil
		if( SE_SUN_NEAR==sub_phase || sub_phase==SE_SUN_MIDDLE /*|| SE_SUN_FAR==sub_phase*/ ) 
			RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0xff,0xFE, D3DSTENCILOP_KEEP, D3DSTENCILOP_ZERO, D3DSTENCILOP_KEEP);
		else
			RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0xff,0x00);

		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,8,0,16);

		// Fetch4 : disable
		if (RImplementation.o.HW_smap_FETCH4)	{
			//. we hacked the shader to force smap on S0
#			define FOURCC_GET1  MAKEFOURCC('G','E','T','1') 
			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET1 );
		}

		// disable depth bounds
		u_DBT_disable	();

		//	Igor: draw volumetric here
		if ( RImplementation.o.advancedpp&&(ps_r_sun_shafts>0) && sub_phase == SE_SUN_FAR)
			accum_direct_volumetric	(sub_phase, Offset, m_shadow);
	}
}

void CRenderTarget::accum_direct_blend	()
{
	// blend-copy
	if (!RImplementation.o.fp16_blend)	{
		u_setrt						(rt_Accumulator,NULL,NULL,HW.pBaseZB);

		// Common calc for quad-rendering
		u32		Offset;
		u32		C					= color_rgba	(255,255,255,255);
		float	_w					= float			(Device.dwWidth);
		float	_h					= float			(Device.dwHeight);
		Fvector2					p0,p1;
		p0.set						(.5f/_w, .5f/_h);
		p1.set						((_w+.5f)/_w, (_h+.5f)/_h );
		float	d_Z	= EPS_S, d_W = 1.f;

		// Fill vertex buffer
		FVF::TL2uv* pv				= (FVF::TL2uv*) RCache.Vertex.Lock	(4,g_combine_2UV->vb_stride,Offset);
		pv->set						(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y, p0.x, p1.y);	pv++;
		pv->set						(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y, p0.x, p0.y);	pv++;
		pv->set						(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y, p1.x, p1.y);	pv++;
		pv->set						(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y, p1.x, p0.y);	pv++;
		RCache.Vertex.Unlock		(4,g_combine_2UV->vb_stride);
		RCache.set_Geometry			(g_combine_2UV);
		RCache.set_Element			(s_accum_mask->E[SE_MASK_ACCUM_2D]	);
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0xff,0x00);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2	);
	}

	increment_light_marker();
}

void CRenderTarget::accum_direct_f		(u32 sub_phase)
{
	// Select target
	if (SE_SUN_LUMINANCE==sub_phase)	{
		accum_direct_lum	();
		return				;
	}
	phase_accumulator					();
	u_setrt								(rt_Generic_0,NULL,NULL,HW.pBaseZB);

	// *** assume accumulator setted up ***
	light*			fuckingsun			= (light*)RImplementation.Lights.sun._get()	;

	// Common calc for quad-rendering
	u32		Offset;
	u32		C					= color_rgba	(255,255,255,255);
	float	_w					= float			(Device.dwWidth);
	float	_h					= float			(Device.dwHeight);
	Fvector2					p0,p1;
	p0.set						(.5f/_w, .5f/_h);
	p1.set						((_w+.5f)/_w, (_h+.5f)/_h );
	float	d_Z	= EPS_S, d_W = 1.f;

	Matrix4x4 &mView = (Device.mView);
	Matrix4x4 &mFullTransform = (Device.mFullTransform);

	// Common constants (light-related)
	Fvector		L_dir,L_clr;	float L_spec;
	L_clr.set					(fuckingsun->color.r,fuckingsun->color.g,fuckingsun->color.b);
	L_spec						= u_diffuse2s	(L_clr);
	XRay::Math::TransformDirByMatrix(mView, L_dir,fuckingsun->direction);
	L_dir.normalize				();

	// Perform masking (only once - on the first/near phase)
	RCache.set_CullMode			(CULL_NONE	);
	if (SE_SUN_NEAR==sub_phase)	//.
	{
		// For sun-filter - clear to zero
		RCache.Clear(0L, nullptr, D3DCLEAR_TARGET, 0, 1.0f, 0L);

		// Fill vertex buffer
		FVF::TL* pv					= (FVF::TL*)	RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y);	pv++;
		pv->set						(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y);	pv++;
		pv->set						(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y);	pv++;
		pv->set						(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);
		RCache.set_Geometry			(g_combine);

		// setup
		float	intensity			= 0.3f*fuckingsun->color.r + 0.48f*fuckingsun->color.g + 0.22f*fuckingsun->color.b;
		Fvector	dir					= L_dir;
		dir.normalize().mul	(- _sqrt(intensity+EPS));
		RCache.set_Element			(s_accum_mask->E[SE_MASK_DIRECT]);		// masker
		RCache.set_c				("Ldynamic_dir",		dir.x,dir.y,dir.z,0		);

		// if (stencil>=1 && aref_pass)	stencil = light_id
		RCache.set_ColorWriteEnable	(FALSE		);
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0x01,0xff,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

	// recalculate d_Z, to perform depth-clipping
	Fvector	center_pt;			center_pt.mad	(Device.vCameraPosition,Device.vCameraDirection,ps_r_sun_near);
	Fvector desc;
	XRay::Math::TransformVectorsByMatrix(mFullTransform, desc, center_pt)	;
	center_pt.set(desc);
	d_Z							= center_pt.z	;

	// nv-stencil recompression
	if (RImplementation.o.nvstencil  && (SE_SUN_NEAR==sub_phase))	u_stencil_optimize();	//. driver bug?

	// Perform lighting
	{
		u_setrt								(rt_Generic_0,NULL,NULL,HW.pBaseZB);  // enshure RT setup
		RCache.set_CullMode					(CULL_NONE	);
		RCache.set_ColorWriteEnable			();

		// texture adjustment matrix
		float			fTexelOffs			= (.5f / float(RImplementation.o.smapsize));
		float			fRange				= (SE_SUN_NEAR==sub_phase)?ps_r_sun_depth_near_scale:ps_r_sun_depth_far_scale;
		float			fBias				= (SE_SUN_NEAR==sub_phase)?ps_r_sun_depth_near_bias:ps_r_sun_depth_far_bias;
		Matrix4x4			m_TexelAdjust		= 
		{
			0.5f,				0.0f,				0.0f,			0.0f,
			0.0f,				-0.5f,				0.0f,			0.0f,
			0.0f,				0.0f,				fRange,			0.0f,
			0.5f + fTexelOffs,	0.5f + fTexelOffs,	fBias,			1.0f
		};

		// compute xforms
		Matrix4x4				m_shadow;
		{
			FPU::m64r		();
			Matrix4x4			xf_invview;		xf_invview.InvertMatrixByMatrix	(mView)	;
			Matrix4x4			xf_project;		xf_project.Multiply		(fuckingsun->X.D.combine, m_TexelAdjust);
			m_shadow.Multiply	(xf_invview, xf_project);

			// tsm-bias
			if (SE_SUN_FAR == sub_phase)
			{
				Fvector		bias;	bias.mul		(L_dir,ps_r_sun_tsm_bias);
				Matrix4x4		bias_t;	bias_t.Translate(bias);
				m_shadow.Multiply	(bias_t, m_shadow);
			}
			FPU::m24r		();
		}

		// Make jitter texture
		Fvector2					j0,j1;
		float	scale_X				= float(Device.dwWidth)	/ float(TEX_jitter);
		float	offset				= (.5f / float(TEX_jitter));
		j0.set						(offset,offset);
		j1.set						(scale_X,scale_X).add(offset);

		// Fill vertex buffer
		FVF::TL2uv* pv				= (FVF::TL2uv*) RCache.Vertex.Lock	(4,g_combine_2UV->vb_stride,Offset);
		pv->set						(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y, j0.x, j1.y);	pv++;
		pv->set						(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y, j0.x, j0.y);	pv++;
		pv->set						(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y, j1.x, j1.y);	pv++;
		pv->set						(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y, j1.x, j0.y);	pv++;
		RCache.Vertex.Unlock		(4,g_combine_2UV->vb_stride);
		RCache.set_Geometry			(g_combine_2UV);

		// setup
		RCache.set_Element			(s_accum_direct->E[sub_phase]);
		RCache.set_c				("Ldynamic_dir",		L_dir.x,L_dir.y,L_dir.z,0		);
		RCache.set_c				("Ldynamic_color",		L_clr.x,L_clr.y,L_clr.z,L_spec	);
		RCache.set_c				("m_shadow",			m_shadow						);

		// setup stencil
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0xff,0x00);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);

		//	Igor: draw volumetric here
	}
}

void CRenderTarget::accum_direct_lum	()
{
	// Select target
	phase_accumulator					();

	// *** assume accumulator setted up ***
	light*			fuckingsun			= (light*)RImplementation.Lights.sun._get()	;

	// Common calc for quad-rendering
	u32		Offset;
	// u32		C					= color_rgba	(255,255,255,255);
	float	_w					= float			(Device.dwWidth);
	float	_h					= float			(Device.dwHeight);
	Fvector2					p0,p1;
	p0.set						(.5f/_w, .5f/_h);
	p1.set						((_w+.5f)/_w, (_h+.5f)/_h );
	float	d_Z	= EPS_S;		//, d_W = 1.f;

	// Common constants (light-related)
	Fvector		L_dir,L_clr;	float L_spec;
	L_clr.set					(fuckingsun->color.r,fuckingsun->color.g,fuckingsun->color.b);
	L_spec						= u_diffuse2s	(L_clr);
	CastToGSCMatrix(Device.mView).transform_dir	(L_dir,fuckingsun->direction);
	L_dir.normalize				();

	// recalculate d_Z, to perform depth-clipping
	Fvector	center_pt;			center_pt.mad	(Device.vCameraPosition,Device.vCameraDirection,ps_r_sun_near);
	CastToGSCMatrix(Device.mFullTransform).transform(center_pt)	;
	d_Z							= center_pt.z	;

	// Perform lighting
		RCache.set_CullMode					(CULL_NONE	);
		RCache.set_ColorWriteEnable			();

		// Make jitter texture
		Fvector2					j0,j1;
		float	scale_X				= float(Device.dwWidth)	/ float(TEX_jitter);
		float	offset				= (.5f / float(TEX_jitter));
		j0.set						(offset,offset);
		j1.set						(scale_X,scale_X).add(offset);

		struct v_aa	{
			Fvector4	p;
			Fvector2	uv0;
			Fvector2	uvJ;
			Fvector2	uv1;
			Fvector2	uv2;
			Fvector2	uv3;
			Fvector4	uv4;
			Fvector4	uv5;
		};
		float	smooth				= 0.6f;
		float	ddw					= smooth/_w;
		float	ddh					= smooth/_h;

		// Fill vertex buffer
		VERIFY	(sizeof(v_aa)==g_aa_AA->vb_stride);
		v_aa* pv					= (v_aa*) RCache.Vertex.Lock	(4,g_aa_AA->vb_stride,Offset);
		pv->p.set(EPS,			float(_h+EPS),	EPS,1.f); pv->uv0.set(p0.x, p1.y);pv->uvJ.set(j0.x, j1.y);pv->uv1.set(p0.x-ddw,p1.y-ddh);pv->uv2.set(p0.x+ddw,p1.y+ddh);pv->uv3.set(p0.x+ddw,p1.y-ddh);pv->uv4.set(p0.x-ddw,p1.y+ddh,0,0);pv->uv5.set(0,0,0,0);pv++;
		pv->p.set(EPS,			EPS,			EPS,1.f); pv->uv0.set(p0.x, p0.y);pv->uvJ.set(j0.x, j0.y);pv->uv1.set(p0.x-ddw,p0.y-ddh);pv->uv2.set(p0.x+ddw,p0.y+ddh);pv->uv3.set(p0.x+ddw,p0.y-ddh);pv->uv4.set(p0.x-ddw,p0.y+ddh,0,0);pv->uv5.set(0,0,0,0);pv++;
		pv->p.set(float(_w+EPS),float(_h+EPS),	EPS,1.f); pv->uv0.set(p1.x, p1.y);pv->uvJ.set(j1.x, j1.y);pv->uv1.set(p1.x-ddw,p1.y-ddh);pv->uv2.set(p1.x+ddw,p1.y+ddh);pv->uv3.set(p1.x+ddw,p1.y-ddh);pv->uv4.set(p1.x-ddw,p1.y+ddh,0,0);pv->uv5.set(0,0,0,0);pv++;
		pv->p.set(float(_w+EPS),EPS,			EPS,1.f); pv->uv0.set(p1.x, p0.y);pv->uvJ.set(j1.x, j0.y);pv->uv1.set(p1.x-ddw,p0.y-ddh);pv->uv2.set(p1.x+ddw,p0.y+ddh);pv->uv3.set(p1.x+ddw,p0.y-ddh);pv->uv4.set(p1.x-ddw,p0.y+ddh,0,0);pv->uv5.set(0,0,0,0);pv++;
		RCache.Vertex.Unlock		(4,g_aa_AA->vb_stride);
		RCache.set_Geometry			(g_aa_AA);

		// setup
		RCache.set_Element			(s_accum_direct->E[SE_SUN_LUMINANCE]);
		RCache.set_c				("Ldynamic_dir",		L_dir.x,L_dir.y,L_dir.z,0		);
		RCache.set_c				("Ldynamic_color",		L_clr.x,L_clr.y,L_clr.z,L_spec	);

		// setup stencil
		RCache.set_Stencil			(TRUE,D3DCMP_LESSEQUAL,dwLightMarkerID,0xff,0x00);
		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
}

void CRenderTarget::accum_direct_volumetric	(u32 sub_phase, const u32 Offset, const Matrix4x4 &mShadow)
{
	if ((sub_phase != SE_SUN_NEAR) && (sub_phase != SE_SUN_MIDDLE) && (sub_phase != SE_SUN_FAR)) return;

	if (!need_to_render_sunshafts() || ps_r_sunshafts_mode != SS_VOLUMETRIC)
		return;

	phase_vol_accumulator();

	RCache.set_ColorWriteEnable();

	//	Assume everything was recalculated before this call by accum_direct

	//	Set correct depth surface
	//	It's slow. Make this when shader is created
	{
		char*		pszSMapName;
		BOOL		b_HW_smap	= RImplementation.o.HW_smap;
		BOOL		b_HW_PCF	= RImplementation.o.HW_smap_PCF;
		if (b_HW_smap)		{
				pszSMapName = r2_RT_smap_depth;
		}
		else				pszSMapName = r2_RT_smap_surf;
		//s_smap

		STextureList* _T = &*s_accum_direct_volumetric_cascade->E[0]->passes[0]->T;
		
		if (ps_r_flags.is(R_FLAG_SUN_OLD))
			_T = &*s_accum_direct_volumetric->E[0]->passes[0]->T;

		STextureList::iterator	_it		= _T->begin	();
		STextureList::iterator	_end	= _T->end	();
		for (; _it!=_end; _it++)
		{
			std::pair<u32,ref_texture>&		loader	=	*_it;
			u32			load_id	= loader.first;
			//	Shadowmap texture always uses 0 texture unit
			if (load_id==0)		
			{
				//	Assign correct texture
				loader.second.create(pszSMapName);
			}
		}
	}

	// Perform lighting
	{

		// *** assume accumulator setted up ***
		light*			fuckingsun			= (light*)RImplementation.Lights.sun._get()	;

		// Common constants (light-related)
		Fvector		L_clr;
		L_clr.set					(fuckingsun->color.r,fuckingsun->color.g,fuckingsun->color.b);
		
		if (ps_r_flags.is(R_FLAG_SUN_OLD))
			RCache.set_Element			(s_accum_direct_volumetric->E[0]);
		else
		{
			RCache.set_Element			(s_accum_direct_volumetric_cascade->E[0]);
			RCache.set_CullMode			(CULL_CCW); 
		}

		RCache.set_c				("Ldynamic_color",		L_clr.x,L_clr.y,L_clr.z,0);
		RCache.set_c				("m_shadow",			mShadow);
		Matrix4x4			m_Texgen;
		m_Texgen.Identity();
 		RCache.xforms.set_W( m_Texgen );
 		RCache.xforms.set_V( (Device.mView ));
 		RCache.xforms.set_P( (Device.mProject ));
 		u_compute_texgen_screen	( m_Texgen );

		RCache.set_c				("m_texgen",			m_Texgen);

		// nv-DBT
		float zMin,zMax;
		if (SE_SUN_NEAR==sub_phase)
		{
			zMin = 0;
			zMax = ps_r_sun_near;
		}
		else
		{
			if (ps_r_flags.is(R_FLAG_SUN_OLD))
				zMin = ps_r_sun_near;
			else
				zMin = 0;

			zMax = ps_r_sun_far;
		}

		RCache.set_c("volume_range", zMin, zMax, 0, 0);

		Fvector	center_pt;
		center_pt.mad(Device.vCameraPosition,Device.vCameraDirection,zMin);	
		CastToGSCMatrix(Device.mFullTransform).transform(center_pt);
		zMin = center_pt.z	;

		center_pt.mad(Device.vCameraPosition,Device.vCameraDirection,zMax);	
		CastToGSCMatrix(Device.mFullTransform).transform	(center_pt);
		zMax = center_pt.z	;


		if (u_DBT_enable(zMin,zMax))	{
			// z-test always
			HW.pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			HW.pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}
		else
		{
			if (SE_SUN_NEAR==sub_phase)
				HW.pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER);
			else
				HW.pDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS);
		}

		// Fetch4 : enable
		if (RImplementation.o.HW_smap_FETCH4)	{
			//. we hacked the shader to force smap on S0
#			define FOURCC_GET4  MAKEFOURCC('G','E','T','4') 
			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET4 );
		}

		if (ps_r_flags.is(R_FLAG_SUN_OLD))
			RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
		else
			RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,8,0,16);

		// Fetch4 : disable
		if (RImplementation.o.HW_smap_FETCH4)	{
			//. we hacked the shader to force smap on S0
#			define FOURCC_GET1  MAKEFOURCC('G','E','T','1') 
			HW.pDevice->SetSamplerState	( 0, D3DSAMP_MIPMAPLODBIAS, FOURCC_GET1 );
		}

		// disable depth bounds
		u_DBT_disable	();
	}
}