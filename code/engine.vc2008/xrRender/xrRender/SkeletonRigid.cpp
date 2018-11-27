//---------------------------------------------------------------------------
#include 	"stdafx.h"
#pragma hdrstop

#include 	"SkeletonCustom.h"

#ifdef DEBUG
void check_kinematics(CKinematics* _k, LPCSTR s);
#endif

void CKinematics::CalculateBones			(BOOL bForceExact)
{
	// early out.
	// check if the info is still relevant
	// skip all the computations - assume nothing changes in a small period of time :)
	if		(RDEVICE.dwTimeGlobal == UCalc_Time)										return;	// early out for "fast" update
	xrCriticalSectionGuard guard(UCalc_Mutex);
	OnCalculateBones		();
	if		(!bForceExact && (RDEVICE.dwTimeGlobal < (UCalc_Time + UCalc_Interval)))	return;	// early out for "slow" update
	if		(Update_Visibility)									Visibility_Update	();

	_DBG_SINGLE_USE_MARKER;
	// here we have either:
	//	1:	timeout elapsed
	//	2:	exact computation required
	UCalc_Time			= RDEVICE.dwTimeGlobal;

	// exact computation
	// Calculate bones
#ifdef DEBUG
	RDEVICE.Statistic->Animation.Begin();
#endif
	Matrix4x4 MIdent = DirectX::XMMatrixIdentity();
	Bone_Calculate					(bones->at(iRoot), &MIdent);
#ifdef DEBUG
	check_kinematics				(this, dbg_name.c_str() );
	RDEVICE.Statistic->Animation.End	();
#endif
	VERIFY( LL_GetBonesVisible()!=0 );
	// Calculate BOXes/Spheres if needed
	UCalc_Visibox++; 
	if (UCalc_Visibox>= ps_r_SkeletonUpdate)
	{
		// mark
		UCalc_Visibox		= -(::Random.randI(ps_r_SkeletonUpdate - 1));

		// the update itself
		Fbox	Box; Box.invalidate();
		for (u32 b=0; b<bones->size(); b++)
		{
			if			(!LL_GetBoneVisible(u16(b)))		continue;
			Fobb&		obb		= (*bones)[b]->obb;
			Matrix4x4&	Mbone	= bone_instances[b].mTransform;
			Matrix4x4		Mbox;	obb.xform_get(XRay::Math::CastToGSCMatrix(Mbox));
			Matrix4x4		X;		X.Multiply43(Mbox, Mbone);
			Fvector&	S		= obb.m_halfsize;

			Fvector			P,A;
			A.set( -S.x,	-S.y,	-S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set( -S.x,	-S.y,	 S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set(  S.x,	-S.y,	 S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set(  S.x,	-S.y,	-S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set( -S.x,	 S.y,	-S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set( -S.x,	 S.y,	 S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set(  S.x, 	 S.y,	 S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
			A.set(  S.x, 	 S.y,	-S.z ); XRay::Math::TransformTiny(X, P, A); Box.modify(P);
		}
	if(bones->size())
	{
		// previous frame we have updated box - update sphere
		vis.box.min			= (Box.min);
		vis.box.max			= (Box.max);
		vis.box.getsphere	(vis.sphere.P,vis.sphere.R);
	}
#ifdef DEBUG
		// Validate
		VERIFY3	(_valid(vis.box.min)&&_valid(vis.box.max),	"Invalid bones-xform in model", dbg_name.c_str());
		if(vis.sphere.R>1000.f)
		{
			for(u16 ii=0; ii<LL_BoneCount();++ii){
				Matrix4x4 tr;
				tr = LL_GetTransform(ii);
				Log("bone ",LL_BoneName_dbg(ii));
				Log("bone_matrix",tr);
			}
			Log("end-------");
		}
		VERIFY3	(vis.sphere.R<1000.f,						"Invalid bones-xform in model", dbg_name.c_str());
#endif
	}

	//
	if (Update_Callback)	Update_Callback(this);
}

#ifdef DEBUG
void check_kinematics(CKinematics* _k, LPCSTR s)
{
	CKinematics* K = _k;
	Matrix4x4&	MrootBone		= K->LL_GetBoneInstance(K->LL_GetBoneRoot()).mTransform;
	if(MrootBone.c.y >10000)
	{	
		Msg("all bones transform:--------[%s]",s);
		
		for(u16 ii=0; ii<K->LL_BoneCount();++ii){
			Matrix4x4 tr;

			tr = K->LL_GetTransform(ii);
			Log("bone ",K->LL_BoneName_dbg(ii));
			Log("bone_matrix",tr);
		}
		Log("end-------");
		VERIFY3(0,"check_kinematics failed for ", s);
	}
}
#endif

void	CKinematics::			BuildBoneMatrix			( const CBoneData* bd, CBoneInstance &bi, const Matrix4x4 *parent, u8 channel_mask/* = (1<<0)*/ )
{
	bi.mTransform.Multiply43(bd->bind_transform, *parent);
}

void CKinematics::CLBone( const CBoneData* bd, CBoneInstance &bi, const Matrix4x4 *parent, u8 channel_mask /*= (1<<0)*/)
{
	
	u16							SelfID				= bd->GetSelfID();

	if ( LL_GetBoneVisible(SelfID) ){
		if ( bi.callback_overwrite() ){
			if ( bi.callback() )	bi.callback()( &bi );
		} else {

			BuildBoneMatrix( bd, bi, parent, channel_mask );
#ifndef MASTER_GOLD
			R_ASSERT2( _valid( bi.mTransform ), "anim kils bone matrix" ); 
#endif // #ifndef MASTER_GOLD
			if (bi.callback())
			{
				bi.callback()(&bi);
#ifndef MASTER_GOLD
				R_ASSERT_FORMAT(_valid( bi.mTransform ), "callback kils bone matrix bone: %s ", bd->name.c_str()); 
#endif // #ifndef MASTER_GOLD
			}
		}
		bi.mRenderTransform.Multiply43(bd->m2b_transform, bi.mTransform);
	}
}

void	CKinematics::Bone_GetAnimPos(Matrix4x4& pos,u16 id,u8 mask_channel, bool ignore_callbacks)
{
	R_ASSERT(id<LL_BoneCount());
	CBoneInstance bi = LL_GetBoneInstance(id);
	BoneChain_Calculate(&LL_GetData(id),bi,mask_channel,ignore_callbacks);
#ifndef MASTER_GOLD
	R_ASSERT( _valid( bi.mTransform ) );
#endif
	pos =  bi.mTransform;
}

void CKinematics::Bone_Calculate(CBoneData* bd, Matrix4x4 *parent)
{

	u16							SelfID				= bd->GetSelfID();
	CBoneInstance				&BONE_INST			= LL_GetBoneInstance(SelfID);
	CLBone( bd, BONE_INST, parent, u8(-1) );
	// Calculate children
	for (xr_vector<CBoneData*>::iterator C=bd->children.begin(); C!=bd->children.end(); C++)
		Bone_Calculate( *C, &BONE_INST.mTransform );

}

void	CKinematics::BoneChain_Calculate		(const CBoneData* bd, CBoneInstance &bi, u8 mask_channel, bool ignore_callbacks)
{
	u16 SelfID					= bd->GetSelfID();
	//CBlendInstance& BLEND_INST	= LL_GetBlendInstance(SelfID);
	//CBlendInstance::BlendSVec &Blend = BLEND_INST.blend_vector();
//ignore callbacks
    CBoneInstance::BoneCallback bc = bi.callback();
	BOOL		 ow = bi.callback_overwrite();
	if(ignore_callbacks)
	{
		bi.set_callback( bi.callback_type(), 0, bi.callback_param(), 0 );

	}
	if(SelfID==LL_GetBoneRoot())
	{
		Matrix4x4 MIdent = DirectX::XMMatrixIdentity();
		CLBone( bd, bi, &MIdent, mask_channel );
		//restore callback	
		bi.set_callback( bi.callback_type(), bc, bi.callback_param(), ow );
		return;
	}
	u16 ParentID				= bd->GetParentID();
	R_ASSERT( ParentID != BI_NONE );
	CBoneData* ParrentDT		= &LL_GetData(ParentID);
	CBoneInstance parrent_bi	= LL_GetBoneInstance(ParentID);
	BoneChain_Calculate(ParrentDT, parrent_bi, mask_channel, ignore_callbacks);
	CLBone( bd, bi, &parrent_bi.mTransform, mask_channel );
	//restore callback
	bi.set_callback( bi.callback_type(), bc, bi.callback_param(), ow );

}
