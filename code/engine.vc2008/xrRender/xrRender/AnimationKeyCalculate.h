#pragma once



//------------------------------------------------------------------------------
// calculate
//------------------------------------------------------------------------------
IC void	KEY_Interp	(CKey& D,const CKey& K1,const CKey& K2, float delta)
{
	VERIFY			(_valid(delta));
	VERIFY			(delta>=0.f && delta<=1.f);
	D.Q.slerp		(K1.Q,K2.Q,delta);
	D.T.lerp		(K1.T,K2.T,delta);
}
struct ConsistantKey
{
	const CKey*	K;
	float	w;

	IC void	set(const CKey* _K, float _w)
	{	K = _K; w = _w; }
};

IC bool operator < (const ConsistantKey& A, const ConsistantKey& B)	// note: inverse operator
{	return A.w>B.w; }

IC void MakeKeysSelected(ConsistantKey *keys, int count)
{
	// sort in decreasing order
	std::sort(keys,keys+count);
}

IC	void QR2Quat(const CKeyQR &K,Fquaternion &Q)
{
	Q.x		= float(K.x)*KEY_QuantI;
	Q.y		= float(K.y)*KEY_QuantI;
	Q.z		= float(K.z)*KEY_QuantI;
	Q.w		= float(K.w)*KEY_QuantI;
}

IC void QT8_2T(const CKeyQT8& K, const CMotion& M, Fvector &T)
{
	T.x		= float(K.x1)*M._sizeT.x+M._initT.x;
	T.y		= float(K.y1)*M._sizeT.y+M._initT.y;
	T.z		= float(K.z1)*M._sizeT.z+M._initT.z;
}
                            
IC void QT16_2T(const CKeyQT16& K, const CMotion& M, Fvector &T)
{
	T.x		= float(K.x1)*M._sizeT.x+M._initT.x;
	T.y		= float(K.y1)*M._sizeT.y+M._initT.y;
	T.z		= float(K.z1)*M._sizeT.z+M._initT.z;
}                             

IC void Dequantize(CKey& K,const CBlend& BD,const CMotion& M)
{
	CKey*			D		=	&K;
	const CBlend*	B		=	&BD;
	float			time	=	B->timeCurrent*float(SAMPLE_FPS);
	VERIFY			(time >= 0.f);
	u32				frame	=	iFloor(time);
	float			delta	=	time-float(frame);
	u32				count	=	M.get_count();
	// rotation
	if (M.test_flag(flRKeyAbsent)){
		const CKeyQR *		K		=	&M._keysR[0];
		QR2Quat(*K,D->Q);
	}else{
		const CKeyQR*		K1r		=	&M._keysR[(frame+0)%count];
		const CKeyQR*		K2r		=	&M._keysR[(frame+1)%count];
		Fquaternion	Q1,Q2;
		QR2Quat(*K1r,Q1);
		QR2Quat(*K2r,Q2);
		D->Q.slerp	(Q1,Q2,clampr(delta,0.f,1.f));
	}

	// translate
	if (M.test_flag(flTKeyPresent))
	{
       Fvector T1,T2;
       if(M.test_flag(flTKey16IsBit))
       {
            const CKeyQT16*	K1t	= &M._keysT16[(frame+0)%count];
            const CKeyQT16*	K2t	= &M._keysT16[(frame+1)%count];
        

            QT16_2T(*K1t,M,T1);
            QT16_2T(*K2t,M,T2);
        }else
        {
            const CKeyQT8*	K1t	= &M._keysT8[(frame+0)%count];
            const CKeyQT8*	K2t	= &M._keysT8[(frame+1)%count];
        
            QT8_2T(*K1t,M,T1);
            QT8_2T(*K2t,M,T2);
        }

		D->T.lerp	(T1,T2,delta);

	}
	else
	{
		D->T.set	(M._initT);
	}
}




IC void MixInterlerp( CKey &Result, const CKey	*R, const CBlend* const BA[MAX_BLENDED], int b_count )
{

	VERIFY( MAX_BLENDED >= b_count );
	switch (b_count)
	{
	case 0:
		Result.Q.set	(0,0,0,0);
		Result.T.set	(0,0,0);
		break;
	case 1: 
		Result			= R[0];
		break;
	case 2:
		{
			float w0 = BA[0]->blendAmount;
			float w1 = BA[1]->blendAmount;
			float ws = w0+w1;
			float w;
			if (fis_zero(ws))	w = 0;
			else				w = w1/ws;

			KEY_Interp	(Result,R[0],R[1], clampr(w,0.f,1.f));
		}
		break;
	default:
		{
			float   total 	= 0;
			ConsistantKey		S[MAX_BLENDED];
			for (int i=0; i<b_count; i++)					
				S[i].set	( R+i, BA[i]->blendAmount );

			std::sort	(S,S+b_count);
			CKey		tmp;
			total		= S[0].w;
			tmp			= *S[0].K;
			for 		(int cnt=1; cnt<b_count; cnt++){
				total	+= S[cnt].w;
				float	d;
				if (fis_zero(total))	d = 0.0f;
				else d	= S[cnt].w/total;

				clampr(d,0.f,1.f);

				KEY_Interp	(Result,tmp, *S[cnt].K, d );
				tmp 		= Result;
			}
		}
		break;
	}
}



IC void key_sub(CKey &rk, const CKey &k0, const CKey& k1)//sub right
{
	Fquaternion q;
	q.inverse(k1.Q);
	rk.Q.mul(k0.Q,q);
	//rk.Q.normalize();//?
	rk.T.sub(k0.T,k1.T);
}

IC void key_identity(CKey &k)
{
	k.Q.identity();
	k.T.set(0,0,0);
}
IC void key_add(CKey &res, const CKey &k0, const CKey &k1)//add right
{
	
	res.Q.set(Fquaternion().mul(k0.Q,k1.Q));
	//res.Q.normalize();
	res.T.add(k0.T,k1.T);
}
IC void q_scale(Fquaternion &q, float v)
{
	float angl;Fvector ax;
	q.get_axis_angle(ax,angl);
	q.rotation(ax,angl*v);
	//q.normalize();
}
IC void key_scale(CKey &res, const CKey &k, float v)
{
	res = k;
	q_scale(res.Q,v);
	res.T.mul(v);
}
IC void key_mad(CKey &res, const CKey &k0, const CKey& k1, float v)
{
	CKey k ;
	key_scale(k,k1,v);
	key_add(res,k,k0);
}

IC void keys_substruct(CKey	*R, const CKey	*BR, int b_count )
{
	for (int i=0; i<b_count; i++)
	{
		CKey r;
		key_sub (r,R[i],BR[i]);
		R[i] = r;
	}
}
 
IC void q_scalem(Matrix4x4 &m, float v)
{
	Fquaternion q;
	q.set(m);
	q_scale(q,v);
	m = DirectX::XMMatrixRotationQuaternion({ q.x, q.y, q.z, q.w });
}

//sclale base' * q by scale_factor returns result in matrix  m_res
IC void q_scale_vs_basem(Matrix4x4 &m_res,const Fquaternion &q, const Fquaternion &base,float scale_factor)
{
	Matrix4x4 mb,imb;
	mb = DirectX::XMMatrixRotationQuaternion({ base.x, base.y, base.z, base.w });
	imb.InvertMatrixByMatrix(mb);

	Matrix4x4 m;m = DirectX::XMMatrixRotationQuaternion({ q.x, q.y, q.z, q.w });
	m_res.Multiply(m, imb);
	q_scalem(m_res,scale_factor);
}


IC void q_add_scaled_basem( Fquaternion &q, const Fquaternion &base, const Fquaternion &q0, const Fquaternion &q1, float v1 )
{
	Matrix4x4 m0 = DirectX::XMMatrixRotationQuaternion({ q0.x, q0.y, q0.z, q0.w });
	Matrix4x4 m,ml1;
	q_scale_vs_basem( ml1, q1, base, v1 );
	m.Multiply(ml1, m0);
	q.set(m);
	q.normalize();
}

IC float DET(const Matrix4x4 &a)
{
	return
		((a.x[0] * (a.y[1] * a.z[2] - a.y[2] * a.z[1]) -
			a.x[1] * (a.y[0] * a.z[2] - a.y[2] * a.z[0]) +
			a.x[2] * (a.y[0] * a.z[1] - a.y[1] * a.z[0])));
}

IC bool check_scale(const Matrix4x4 &m)
{
	float det = DET(m);
	return (0.8f<det&&det<1.3f);
}

IC bool check_scale(const Fquaternion &q)
{
	Matrix4x4 m = DirectX::XMMatrixRotationQuaternion({ q.x, q.y, q.z, q.w });
	return check_scale(m);
}

IC void MixFactors(float *F,int b_count)
{
	float sum = 0;
	for (int i=0; i<b_count; i++)
		sum+=F[i];
	for (int i2=0; i2<b_count; i2++)
		F[i2]/=sum;
}
IC void MixinAdd(CKey &Result,const CKey	*R,const float* BA,int b_count)
{
	for (int i=0; i<b_count; i++)
		key_mad(Result,Result,R[i],BA[i]);
}
IC void MixAdd(CKey &Result,const CKey	*R,const float* BA,int b_count)
{
	key_identity(Result);
	MixinAdd(Result,R,BA,b_count);
}
IC void process_single_channel( CKey &Result,const animation::channel_def &ch, const CKey *R, const CBlend* const BA[MAX_BLENDED], int b_count )
{

	MixInterlerp( Result, R, BA, b_count );
	VERIFY( _valid( Result.T ) );
	VERIFY( _valid( Result.Q ) );
}
IC void MixChannels(CKey &Result,const CKey	*R,const animation::channel_def* BA,int b_count)
{
	VERIFY(b_count>0);
	Result = R[0];

	float	lerp_factor_sum = 0.f;
	for ( int i=1; i<b_count; i++ )
		switch( BA[i].rule.extern_ )
	{
		case animation::add:	
			key_mad( Result, CKey( Result ), R[i], BA[i].factor ); break;

		case animation::lerp:
			lerp_factor_sum +=BA[i].factor;
			KEY_Interp	(Result,CKey( Result ),R[i], BA[i].factor/lerp_factor_sum ); break;
		default: NODEFAULT;
	}
	VERIFY( _valid( Result.T ) );
	VERIFY( _valid( Result.Q ) );
}

