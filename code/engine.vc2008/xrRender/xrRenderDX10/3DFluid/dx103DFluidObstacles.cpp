#include "stdafx.h"
#include "dx103DFluidObstacles.h"

#include "../../xrRender/dxRenderDeviceRender.h"
#include "../../../xrEngine/xr_object.h"
#include "../../../xrEngine/IPhysicsShell.h"
#include "../../../xrEngine/IObjectPhysicsCollision.h"
#include "../../../xrEngine/IPhysicsGeometry.h"

#include "dx103DFluidBlenders.h"
#include "dx103DFluidData.h"
#include "dx103DFluidGrid.h"

namespace
{
	//	For OOBB
	shared_str	strOOBBClipPlane("OOBBClipPlane");

	//	For velocity calculation
	shared_str	strWorldToLocal("WorldToLocal");
	shared_str	strLocalToWorld("LocalToWorld");
	shared_str	strMassCenter("MassCenter");
	shared_str	strOOBBWorldAngularVelocity("OOBBWorldAngularVelocity");
	shared_str	strOOBBWorldTranslationVelocity("OOBBWorldTranslationVelocity");

	Fvector4	UnitClipPlanes[] = 
	{
		{-1.f,	0.0f,	0.0f,	0.5f},	//	
		{1.f,	0.0f,	0.0f,	0.5f},	//	
		{0.0f,	-1.f,	0.0f,	0.5f},	//	Top
		{0.0f,	1.f,	0.0f,	0.5f},	//	Bottom
		{0.0f,	0.0f,	-1.f,	0.5f},	//	
		{0.0f,	0.0f,	1.f,	0.5f},	//	
	};
}

dx103DFluidObstacles::dx103DFluidObstacles(int gridWidth, int gridHeight, int gridDepth, dx103DFluidGrid *pGrid)
	:	m_pGrid(pGrid)
{
	VERIFY(m_pGrid);

	m_vGridDim[0] = float(gridWidth);
	m_vGridDim[1] = float(gridHeight);
	m_vGridDim[2] = float(gridDepth);

	InitShaders();
}

dx103DFluidObstacles::~dx103DFluidObstacles()
{
	DestroyShaders();

	m_pGrid = 0;
}

void dx103DFluidObstacles::InitShaders()
{
	{
		CBlender_fluid_obst		Blender;
		ref_shader				shader;
		shader.create(&Blender, "null");
		for (int i=0; i<2; ++i)
			m_ObstacleTechnique[OS_OOBB+i] = shader->E[i];
	}
}

void dx103DFluidObstacles::DestroyShaders()
{
	for (int i=0; i<OS_NumShaders; ++i)
	{
		//	Release shader's element.
		m_ObstacleTechnique[i] = 0;
	}
}

void dx103DFluidObstacles::ProcessObstacles( const dx103DFluidData &FluidData, float timestep )
{
	PIX_EVENT(ProcessObstacles);

	//	Prepare world-space to grid transform
	Matrix4x4 WorldToFluid;
	{
		Matrix4x4 InvFluidTranform;
		Matrix4x4 Scale;
		Matrix4x4 Translate;
		Matrix4x4 TranslateScale;

		//	Convert to 0..intDim space since it is used by simulation
		//Scale.scale((float)m_iTextureWidth-1, (float)m_iTextureHeight-1, (float)m_iTextureDepth-1);
		//Translate.translate(0.5, 0.5, 0.5);
		//It seems that y axis is inverted in fluid simulation, so shange maths a bit
		Scale = DirectX::XMMatrixScaling(m_vGridDim.x, -(m_vGridDim.y), m_vGridDim.z );
		Translate.Translate({ 0.5, -0.5, 0.5 });

		//	Actually it is mul(Translate, Scale).
		//	Our matrix multiplication is not correct.
		TranslateScale.Multiply(Translate, Scale);
		InvFluidTranform.InvertMatrixByMatrix(FluidData.GetTransform());
		WorldToFluid.Multiply(InvFluidTranform, TranslateScale);
	}

	ProcessDynamicObstacles( FluidData, WorldToFluid, timestep );

	//	Render static obstacles last
	//	to override speed where bounding shapes of dynamic and
	//	static objects intersect.
	ProcessStaticObstacles( FluidData, WorldToFluid );
}

void dx103DFluidObstacles::RenderStaticOOBB( const Matrix4x4 &Transform)
{
	PIX_EVENT(RenderObstacle);

	//	Shader must be already set up!
	Matrix4x4	InvTransform;
	Matrix4x4 ClipTransform;
	InvTransform.InvertMatrixByMatrix(Transform);
	ClipTransform = DirectX::XMMatrixTranspose(InvTransform);

	for ( int i=0; i<6; ++i)
	{
		Fvector4	TransformedPlane;
		ClipTransform.Transform(TransformedPlane, UnitClipPlanes[i]);
		TransformedPlane.normalize_as_plane();
		RCache.set_ca(strOOBBClipPlane, i, TransformedPlane);
	}

	m_pGrid->DrawSlices();
}

void dx103DFluidObstacles::ProcessStaticObstacles( const dx103DFluidData &FluidData, const Matrix4x4 &WorldToFluid )
{
	RCache.set_Element(m_ObstacleTechnique[OS_OOBB]);

	const xr_vector<Matrix4x4> &Obstacles = FluidData.GetObstaclesList();
	int iObstNum = Obstacles.size();
	for (int i=0; i<iObstNum; ++i)
	{
		Matrix4x4 Transform;
		Transform.Multiply(Obstacles[i], WorldToFluid);

		RenderStaticOOBB(Transform);
	}
}

void dx103DFluidObstacles::ProcessDynamicObstacles( const dx103DFluidData &FluidData, const Matrix4x4 &WorldToFluid, float timestep )
{
	m_lstRenderables.clear();
	m_lstShells.clear();
	m_lstElements.clear();

	Fbox	box;
	box.min = Fvector3().set(-0.5f, -0.5f, -0.5f);
	box.max = Fvector3().set( 0.5f,  0.5f,  0.5f);
	box.xform(FluidData.GetTransform());
	Fvector3	center;
	Fvector3	size;
	box.getcenter(center);
	box.getradius(size);


	// Traverse object database
	g_SpatialSpace->q_box
		(
		m_lstRenderables,
		0,
		STYPE_RENDERABLE,
		center,
		size
		);

	// Determine visibility for dynamic part of scene
	for (u32 i=0; i<m_lstRenderables.size(); ++i)
	{
		ISpatial*	spatial		= m_lstRenderables[i];
		CObject*		pObject = spatial->dcast_CObject();
		if (!pObject) continue;

		const IObjectPhysicsCollision* pCollision = pObject->physics_collision();
		if (!pCollision) continue;

		const IPhysicsShell*		pShell = pCollision->physics_shell();
		const IPhysicsElement*	pElement = pCollision->physics_character();
		if (pShell)
		{
			//	Push shell here
			m_lstShells.push_back(pShell);
		}
		else if (pElement)
		{
			m_lstElements.push_back(pElement);
		}
	}

	if (! (m_lstShells.size() || m_lstElements.size()))
		return;

	RCache.set_Element(m_ObstacleTechnique[OS_DynamicOOBB]);

	Matrix4x4	FluidToWorld;
	FluidToWorld.InvertMatrixByMatrix(WorldToFluid);

	RCache.set_c(strWorldToLocal, WorldToFluid);	
	RCache.set_c(strLocalToWorld, FluidToWorld);

	int iShellsNum = m_lstShells.size();
	for (int i=0; i<iShellsNum; ++i)
	{
		RenderPhysicsShell( m_lstShells[i], WorldToFluid, timestep );
	}

	int iElementsNum = m_lstElements.size();
	for (int i=0; i<iElementsNum; ++i)
	{
		RenderPhysicsElement( *m_lstElements[i], WorldToFluid, timestep );
	}
	
}

//	TODO: DX10: Do it using instancing.
void dx103DFluidObstacles::RenderPhysicsShell( const IPhysicsShell *pShell, const Matrix4x4 &WorldToFluid, float timestep )
{
	u16 iObstNum = pShell->get_ElementsNumber();
	for (u16 i=0; i<iObstNum; ++i)
	{
		const IPhysicsElement &Element = pShell->Element(i);

		RenderPhysicsElement( Element, WorldToFluid, timestep );
	}
}

void dx103DFluidObstacles::RenderPhysicsElement( const IPhysicsElement &Element, const Matrix4x4 &WorldToFluid, float timestep)
{
	//	Shader must be already set up!
	const Fvector3	&MassCenter3 = Element.mass_Center();
	Fvector3	AngularVelocity3;
	Fvector3	TranslationVelocity3;
	Element.get_AngularVel(AngularVelocity3);
	Element.get_LinearVel(TranslationVelocity3);

	Fvector4	MassCenter;
	Fvector4	AngularVelocity;
	Fvector4	TranslationVelocity;
	MassCenter.set( MassCenter3.x, MassCenter3.y, MassCenter3.z, 0.0f);
	AngularVelocity.set( AngularVelocity3.x, AngularVelocity3.y, AngularVelocity3.z, 0.0f);
	TranslationVelocity.set( TranslationVelocity3.x, TranslationVelocity3.y, TranslationVelocity3.z, 0.0f);

	float	fVelocityScale;

	VERIFY(timestep!=0);

	fVelocityScale = 1/timestep;

	//	Convert speed
	fVelocityScale /= 30.0f * 2.0f;

	//	Emphasize velocity influence on the fog
	fVelocityScale *= 6;

	AngularVelocity.mul( fVelocityScale );
	TranslationVelocity.mul( fVelocityScale );

	RCache.set_c(strMassCenter, MassCenter);
	RCache.set_c(strOOBBWorldAngularVelocity, AngularVelocity);
	RCache.set_c(strOOBBWorldTranslationVelocity, TranslationVelocity);

	int iShapeNum = Element.numberOfGeoms();

	for ( u16 i=0; i<iShapeNum; ++i)
	{
		if (Element.geometry(i)->collide_fluids())
			RenderDynamicOOBB( *Element.geometry(i), WorldToFluid, timestep);
	}
}

void dx103DFluidObstacles::RenderDynamicOOBB( const IPhysicsGeometry &Geometry, const Matrix4x4 &WorldToFluid, float timestep)
{
	PIX_EVENT(RenderDynamicObstacle);

	Matrix4x4 Transform;

	Fvector3 BoxSize;
	Matrix4x4 OOBBTransform;
	Geometry.get_Box( OOBBTransform, BoxSize );

	Transform.Multiply(OOBBTransform, WorldToFluid);

	//	Shader must be already set up!
	//	DynOOBBData must be already set up!
	Matrix4x4	InvTransform;
	Matrix4x4 ClipTransform;
	InvTransform.InvertMatrixByMatrix(Transform);
	ClipTransform = DirectX::XMMatrixTranspose(InvTransform);

	for ( int i=0; i<6; ++i)
	{
		Fvector4	UpdatedPlane = UnitClipPlanes[i];
		UpdatedPlane.w *= BoxSize[i/2];
		Fvector4	TransformedPlane;
		ClipTransform.Transform(TransformedPlane, UpdatedPlane);
		TransformedPlane.normalize_as_plane();
		RCache.set_ca(strOOBBClipPlane, i, TransformedPlane);
	}

	m_pGrid->DrawSlices();
}