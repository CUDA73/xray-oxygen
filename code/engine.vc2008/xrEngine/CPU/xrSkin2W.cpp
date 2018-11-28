#include "stdafx.h"
#pragma hdrstop
#include "xrCPU_Pipe.h"
#ifdef _EDITOR
#	include "skeletonX.h"
#	include "skeletoncustom.h"
#endif // _EDITOR

void __stdcall xrSkin1W_x86(vertRender* D, vertBoned1W* S, u32 vCount, CBoneInstance* Bones)
{
	// Prepare
	int U_Count			= vCount/8;
	vertBoned1W*	V	= S;
	vertBoned1W*	E	= V+U_Count*8;

	// Unrolled loop
	for (; S!=E; )
	{
		Matrix4x4& M0		= Bones[S->matrix].mRenderTransform;
		M0.TransformTiny(D->P,S->P);
		M0.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
		
		Matrix4x4& M1		= Bones[S->matrix].mRenderTransform;
		M1.TransformTiny(D->P,S->P);
		M1.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
		
		Matrix4x4& M2		= Bones[S->matrix].mRenderTransform;
		M2.TransformTiny(D->P,S->P);
		M2.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
		
		Matrix4x4& M3		= Bones[S->matrix].mRenderTransform;
		M3.TransformTiny(D->P,S->P);
		M3.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++; 
		
		Matrix4x4& M4		= Bones[S->matrix].mRenderTransform;
		M4.TransformTiny(D->P,S->P);
		M4.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
		
		Matrix4x4& M5		= Bones[S->matrix].mRenderTransform;
		M5.TransformTiny(D->P,S->P);
		M5.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
		
		Matrix4x4& M6		= Bones[S->matrix].mRenderTransform;
		M6.TransformTiny(D->P,S->P);
		M6.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
		
		Matrix4x4& M7		= Bones[S->matrix].mRenderTransform;
		M7.TransformTiny(D->P,S->P);
		M7.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++; 
	}
	
	// The end part
	vertBoned1W* E2 = V+vCount;
	for (; S!=E2; )
	{
		Matrix4x4& M		= Bones[S->matrix].mRenderTransform;
		M.TransformTiny(D->P,S->P);
		M.TransformDir (D->N,S->N);
		D->u			= S->u;
		D->v			= S->v;
		S++; D++;
	}
}
 
void __stdcall xrSkin2W_x86(vertRender*		D,
							vertBoned2W*	S,
							u32				vCount,
							CBoneInstance*	Bones) 
{
	// Prepare
	int U_Count			= vCount;
	vertBoned2W*	V	= S;
	vertBoned2W*	E	= V+U_Count;
	Fvector			P0,N0,P1,N1;

	// NON-Unrolled loop
	for (; S!=E; )
	{
    	if (S->matrix1!=S->matrix0){
            Matrix4x4& M0		= Bones[S->matrix0].mRenderTransform;
            Matrix4x4& M1		= Bones[S->matrix1].mRenderTransform;
            M0.TransformTiny(P0,S->P);
            M0.TransformDir (N0,S->N);
            M1.TransformTiny(P1,S->P);
            M1.TransformDir (N1,S->N);
            D->P.lerp		(P0,P1,S->w);
            D->N.lerp		(N0,N1,S->w);
            D->u			= S->u;
            D->v			= S->v;
        }
		else
		{
            Matrix4x4& M0		= Bones[S->matrix0].mRenderTransform;
            M0.TransformTiny(D->P,S->P);
            M0.TransformDir (D->N,S->N);
            D->u			= S->u;
            D->v			= S->v;
        }
		S++; D++;
	}
}



void __stdcall xrSkin3W_x86(vertRender*		D,
							vertBoned3W*	S,
							u32				vCount,
							CBoneInstance*	Bones) 
{
	// Prepare
	int U_Count			= vCount;
	vertBoned3W*	V	= S;
	vertBoned3W*	E	= V+U_Count;
	Fvector			P0,N0,P1,N1,P2,N2;

	// NON-Unrolled loop
	for (; S!=E; )
	{
		Matrix4x4& M0		= Bones[ S->m[0] ].mRenderTransform;
        Matrix4x4& M1		= Bones[ S->m[1] ].mRenderTransform;
        Matrix4x4& M2		= Bones[ S->m[2] ].mRenderTransform;

		M0.TransformTiny(P0,S->P); P0.mul(S->w[0]);
        M0.TransformDir (N0,S->N); N0.mul(S->w[0]);

        M1.TransformTiny(P1,S->P); P1.mul(S->w[1]);
        M1.TransformDir (N1,S->N); N1.mul(S->w[1]);

        M2.TransformTiny(P2,S->P); P2.mul(1.0f-S->w[0]-S->w[1]);
        M2.TransformDir (N2,S->N); N2.mul(1.0f-S->w[0]-S->w[1]);

		P0.add(P1);
		P0.add(P2);

		D->P			= P0;

		N0.add(N1);
		N0.add(N2);

		D->N			= N0;
		
		D->u			= S->u;
        D->v			= S->v;

		S++; 
		D++;
	}
}



void __stdcall xrSkin4W_x86(vertRender*		D,
							vertBoned4W*	S,
							u32				vCount,
							CBoneInstance*	Bones) 
{
	// Prepare
	int U_Count			= vCount;
	vertBoned4W*	V	= S;
	vertBoned4W*	E	= V+U_Count;
	Fvector			P0,N0,P1,N1,P2,N2,P3,N3;

	// NON-Unrolled loop
	for (; S!=E; )
	{
		Matrix4x4& M0		= Bones[ S->m[0] ].mRenderTransform;
        Matrix4x4& M1		= Bones[ S->m[1] ].mRenderTransform;
        Matrix4x4& M2		= Bones[ S->m[2] ].mRenderTransform;
        Matrix4x4& M3		= Bones[ S->m[3] ].mRenderTransform;

		M0.TransformTiny(P0,S->P); P0.mul(S->w[0]);
        M0.TransformDir (N0,S->N); N0.mul(S->w[0]);

        M1.TransformTiny(P1,S->P); P1.mul(S->w[1]);
        M1.TransformDir (N1,S->N); N1.mul(S->w[1]);

        M2.TransformTiny(P2,S->P); P2.mul(S->w[2]);
        M2.TransformDir (N2,S->N); N2.mul(S->w[2]);

		M3.TransformTiny(P3,S->P); P3.mul(1.0f-S->w[0]-S->w[1]-S->w[2]);
        M3.TransformDir (N3,S->N); N3.mul(1.0f-S->w[0]-S->w[1]-S->w[2]);

		P0.add(P1);
		P0.add(P2);
		P0.add(P3);

		D->P			= P0;
		
		N0.add(N1);
		N0.add(N2);
		N0.add(N3);

		D->N			= N0;
		
		D->u			= S->u;
        D->v			= S->v;

		S++; 
		D++;
	}
}
