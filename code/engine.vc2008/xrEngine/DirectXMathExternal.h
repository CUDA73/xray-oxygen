#pragma once
#include <DirectXMath.h>
/***************************************************************************
 *   Copyright (C) 2018 - ForserX & Oxydev
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
namespace XRay
{
	namespace Math
	{
		/// <summary>Convert DirectX::XMMATRIX to Fmatrix</summary>
		inline Fmatrix CastToGSCMatrix(const DirectX::XMMATRIX &m)
		{
			return
			{
				m.r[0].m128_f32[0], m.r[0].m128_f32[1], m.r[0].m128_f32[2], m.r[0].m128_f32[3],
				m.r[1].m128_f32[0], m.r[1].m128_f32[1], m.r[1].m128_f32[2], m.r[1].m128_f32[3],
				m.r[2].m128_f32[0], m.r[2].m128_f32[1], m.r[2].m128_f32[2], m.r[2].m128_f32[3],
				m.r[3].m128_f32[0], m.r[3].m128_f32[1], m.r[3].m128_f32[2], m.r[3].m128_f32[3]
			};
		};

		/// <summary>GSC Transform func for DirectX::XMMATRIX</summary>
		inline void TransformVectorsByMatrix(const DirectX::XMMATRIX &m, Fvector &dest, const Fvector &v)
		{
			float iw = 1.f / (v.x*m.r[0].m128_f32[3] + v.y*m.r[1].m128_f32[3] + v.z*m.r[2].m128_f32[3] + m.r[3].m128_f32[3]);

			dest.x = (v.x* m.r[0].m128_f32[0] + v.y* m.r[1].m128_f32[0] + v.z* m.r[2].m128_f32[0] + m.r[3].m128_f32[0])*iw;
			dest.y = (v.x* m.r[0].m128_f32[1] + v.y* m.r[1].m128_f32[1] + v.z* m.r[2].m128_f32[1] + m.r[3].m128_f32[1])*iw;
			dest.z = (v.x* m.r[0].m128_f32[2] + v.y* m.r[1].m128_f32[2] + v.z* m.r[2].m128_f32[2] + m.r[3].m128_f32[2])*iw;
		}

		inline void TransformVectorsByMatrix(const DirectX::XMMATRIX &m, Fvector4 &dest, const Fvector4 &v)
		{
			float iw = 1.f / (v.x*m.r[0].m128_f32[3] + v.y*m.r[1].m128_f32[3] + v.z*m.r[2].m128_f32[3] + v.w*m.r[3].m128_f32[3]);

			dest.x = (v.x* m.r[0].m128_f32[0] + v.y* m.r[1].m128_f32[0] + v.z* m.r[2].m128_f32[0] + m.r[3].m128_f32[0])*iw;
			dest.y = (v.x* m.r[0].m128_f32[1] + v.y* m.r[1].m128_f32[1] + v.z* m.r[2].m128_f32[1] + m.r[3].m128_f32[1])*iw;
			dest.z = (v.x* m.r[0].m128_f32[2] + v.y* m.r[1].m128_f32[2] + v.z* m.r[2].m128_f32[2] + m.r[3].m128_f32[2])*iw;
			dest.w = (v.x* m.r[0].m128_f32[3] + v.y* m.r[1].m128_f32[3] + v.z* m.r[2].m128_f32[3] + m.r[3].m128_f32[3])*iw;
		}


		/// <summary>GSC Transform func for DirectX::XMMATRIX</summary>
		inline void TransformVector4ByMatrix(const DirectX::XMMATRIX &m, Fvector4 &dest, const Fvector &v)
		{
			dest.w = v.x*m.r[0].m128_f32[3] + v.y*m.r[1].m128_f32[3] + v.z*m.r[2].m128_f32[3] + m.r[3].m128_f32[3];
			dest.x = (v.x*m.r[0].m128_f32[0] + v.y*m.r[1].m128_f32[0] + v.z*m.r[2].m128_f32[0] + m.r[3].m128_f32[0]) / dest.w;
			dest.y = (v.x*m.r[0].m128_f32[1] + v.y*m.r[1].m128_f32[1] + v.z*m.r[2].m128_f32[1] + m.r[3].m128_f32[1]) / dest.w;
			dest.z = (v.x*m.r[0].m128_f32[2] + v.y*m.r[1].m128_f32[2] + v.z*m.r[2].m128_f32[2] + m.r[3].m128_f32[2]) / dest.w;
		}

		/// <summary>GSC TransformDir func for DirectX::XMMATRIX</summary>
		inline void TransformDirByMatrix(const DirectX::XMMATRIX &m, Fvector &dest, const Fvector &v)
		{
			dest.x = v.x*m.r[0].m128_f32[0] + v.y*m.r[1].m128_f32[0] + v.z*m.r[2].m128_f32[0];
			dest.y = v.x*m.r[0].m128_f32[1] + v.y*m.r[1].m128_f32[1] + v.z*m.r[2].m128_f32[1];
			dest.z = v.x*m.r[0].m128_f32[2] + v.y*m.r[1].m128_f32[2] + v.z*m.r[2].m128_f32[2];
		}

		/// <summary>GSC TransformDir func for DirectX::XMMATRIX</summary>
		inline void TransformDirByMatrix(const DirectX::XMMATRIX &m, Fvector &v)
		{
			Fvector res;
			TransformDirByMatrix(m, res, v);
			v.set(res);
		}

		/// <summary>GSC TransformTiny23 func for DirectX::XMMATRIX</summary>
		inline void TransformTiny23ByMatrix(const DirectX::XMMATRIX &m, Fvector &dest, const Fvector2 &v)
		{
			dest.x = v.x*m.r[0].m128_f32[0] + v.y*m.r[1].m128_f32[0] + m.r[3].m128_f32[0];
			dest.y = v.x*m.r[0].m128_f32[1] + v.y*m.r[1].m128_f32[1] + m.r[3].m128_f32[1];
			dest.z = v.x*m.r[0].m128_f32[2] + v.y*m.r[1].m128_f32[2] + m.r[3].m128_f32[2];
		}

		/// <summary>GSC TransformTiny32 func for DirectX::XMMATRIX</summary>
		inline void TransformTiny32ByMatrix(const DirectX::XMMATRIX &m, Fvector2 &dest, const Fvector &v)
		{
			dest.x = v.x*m.r[0].m128_f32[0] + v.y*m.r[1].m128_f32[0] + v.z*m.r[2].m128_f32[0] + m.r[3].m128_f32[0];
			dest.y = v.x*m.r[0].m128_f32[1] + v.y*m.r[1].m128_f32[1] + v.z*m.r[2].m128_f32[1] + m.r[3].m128_f32[1];
		}

		/// <summary>GSC Inertion func for DirectX::XMMATRIX</summary>
		inline void InertionByMatrix(DirectX::XMMATRIX &m, DirectX::XMMATRIX &m1, const float &v)
		{
			float iv = 1.f - v;
			for (int i = 0; i < 4; ++i)
			{
				m.r[i].m128_f32[0] = m.r[i].m128_f32[0] * v + m1.r[i].m128_f32[0] * iv;
				m.r[i].m128_f32[1] = m.r[i].m128_f32[1] * v + m1.r[i].m128_f32[1] * iv;
				m.r[i].m128_f32[2] = m.r[i].m128_f32[2] * v + m1.r[i].m128_f32[2] * iv;
				m.r[i].m128_f32[3] = m.r[i].m128_f32[3] * v + m1.r[i].m128_f32[3] * iv;
			}
		}

		inline void InertionByVector(DirectX::XMVECTOR &origin, const DirectX::XMVECTOR &g, const float &v)
		{
			float inv = 1.0f - v;
			origin.m128_f32[0] = v * origin.m128_f32[0] + inv * g.m128_f32[0];
			origin.m128_f32[1] = v * origin.m128_f32[1] + inv * g.m128_f32[1];
			origin.m128_f32[2] = v * origin.m128_f32[2] + inv * g.m128_f32[2];
		}

		inline void InertionByVector(DirectX::XMVECTOR &origin, const Fvector &g, const float &v)
		{
			float inv = 1.0f - v;
			origin.m128_f32[0] = v * origin.m128_f32[0] + inv * g.x;
			origin.m128_f32[1] = v * origin.m128_f32[1] + inv * g.y;
			origin.m128_f32[2] = v * origin.m128_f32[2] + inv * g.z;
		}

		inline void ClampByVector(DirectX::XMVECTOR &origin, const Fvector &mn, const Fvector &mx)
		{
			::clamp<float>(origin.m128_f32[0], mn.x, mx.x);
			::clamp<float>(origin.m128_f32[1], mn.y, mx.y);
			::clamp<float>(origin.m128_f32[2], mn.z, mx.z);
		}

		/// <summary>Set i vector func for DirectX::XMMATRIX</summary>
		inline void Set_i(DirectX::XMMATRIX &m, const float &x, const float &y, const float &z, const float &w = 0)
		{
			m.r[0].m128_f32[0] = x;
			m.r[0].m128_f32[1] = y;
			m.r[0].m128_f32[2] = z;
			m.r[0].m128_f32[3] = w;
		}

		/// <summary>Set j vector func for DirectX::XMMATRIX</summary>
		inline void Set_j(DirectX::XMMATRIX &m, const float &x, const float &y, const float &z, const float &w = 0)
		{
			m.r[1].m128_f32[0] = x;
			m.r[1].m128_f32[1] = y;
			m.r[1].m128_f32[2] = z;
			m.r[1].m128_f32[3] = w;
		}

		/// <summary>Set z vector func for DirectX::XMMATRIX</summary>
		inline void Set_z(DirectX::XMMATRIX &m, const float &x, const float &y, const float &z, const float &w = 0)
		{
			m.r[2].m128_f32[0] = x;
			m.r[2].m128_f32[1] = y;
			m.r[2].m128_f32[2] = z;
			m.r[2].m128_f32[3] = w;
		}

		/// <summary>Set c vector func for DirectX::XMMATRIX (vector c is 4 vector in matrix (e. g. _14 _24 _34 _44)</summary>
		inline void Set_c(DirectX::XMMATRIX &m, const float &x, const float &y, const float &z, const float &w = 0)
		{
			m.r[3].m128_f32[0] = x;
			m.r[3].m128_f32[1] = y;
			m.r[3].m128_f32[2] = z;
			m.r[3].m128_f32[3] = w;
		}

		/// <summary>Returns Zero Vector func for DirectX::XMMATRIX</summary>
		inline DirectX::XMVECTOR GetZeroVector(const float &w = 0)
		{
			DirectX::XMVECTOR F = DirectX::XMVectorZero();
			F.m128_f32[3] = w;
			return F;
		}

		/// <summary>Set i vector is some value func for DirectX::XMMATRIX</summary>
		inline void Set_r0_w(DirectX::XMMATRIX &m, const float &w = 0)
		{
			m.r[0] = GetZeroVector(w);
		}

		/// <summary>Set j vector to 0 but w is some value func for DirectX::XMMATRIX</summary>
		inline void Set_r1_w(DirectX::XMMATRIX &m, const float &w = 0)
		{
			m.r[1] = GetZeroVector(w);
		}

		/// <summary>Set z vector to 0 but w is some value func for DirectX::XMMATRIX</summary>
		inline void Set_r2_w(DirectX::XMMATRIX &m, const float &w = 0)
		{
			m.r[2] = GetZeroVector(w);
		}

		/// <summary>Set c vector to 0 but w is some value func for DirectX::XMMATRIX</summary>
		inline void Set_r3_w(DirectX::XMMATRIX &m, const float &w = 0)
		{
			m.r[3] = GetZeroVector(w);
		}


		inline float XMV_square_magnitude(const DirectX::XMVECTOR &v)
		{
			return (v.m128_f32[0] * v.m128_f32[0] + v.m128_f32[1] * v.m128_f32[1] + v.m128_f32[2] * v.m128_f32[2]);
		}

		inline float XMV_magnitude(const DirectX::XMVECTOR &v)
		{
			return (::_sqrt(XMV_square_magnitude(v)));
		}

		inline float XMFloat2Len(const DirectX::XMFLOAT2& Flt2)
		{
			return sqrtf(Flt2.x * Flt2.x + Flt2.y * Flt2.y);
		}

		inline float XMFloat3Len(const DirectX::XMFLOAT3& Flt3)
		{
			return sqrtf(Flt3.x * Flt3.x + Flt3.y * Flt3.y + Flt3.z * Flt3.z);
		}

		struct Matrix4x4
		{
			struct IntricsVect
			{
				DirectX::XMVECTOR Vect;

			public:
				inline void	div(Fvector a, const float s)
				{
					Vect.m128_f32[0] = a.x / s;
					Vect.m128_f32[1] = a.y / s;
					Vect.m128_f32[2] = a.z / s;
				}

				DirectX::XMVECTOR operator()()
				{
					return Vect;
				};

				float& operator[](size_t id)
				{
					return Vect.m128_f32[id];
				}

				const float& operator[](size_t id) const
				{
					return Vect.m128_f32[id];
				}

				void operator=(DirectX::XMVECTOR &VectObj)
				{
					Vect = VectObj;
				}

				void operator=(const Fvector &VectObj)
				{
					Vect = { VectObj.x, VectObj.y, VectObj.z, Vect.m128_f32[3] };
				}
#pragma warning(disable: 4239)
				inline operator Fvector&() const
				{
					return Fvector({ Vect.m128_f32[0], Vect.m128_f32[1], Vect.m128_f32[2] });
				}
#pragma warning(default: 4239)

				inline bool similar(const IntricsVect &v, float E = EPS_L) const
				{ 
					return	_abs(Vect.m128_f32[0] - v[0]) < E && 
							_abs(Vect.m128_f32[1] - v[1]) < E && 
							_abs(Vect.m128_f32[2] - v[2]) < E &&
							_abs(Vect.m128_f32[3] - v[3]) < E;
				}

				__forceinline float GetH() const
				{
					if (fis_zero(Vect.m128_f32[0]) && fis_zero(Vect.m128_f32[2]))
					{
						return 0.0f;
					}
					else
					{
						if (fis_zero(Vect.m128_f32[2]))
							return (Vect.m128_f32[0] > 0.0f) ? -PI_DIV_2 : PI_DIV_2;
						else if (Vect.m128_f32[2] < 0.0f)
							return -(atanf(Vect.m128_f32[0] / Vect.m128_f32[2]) - PI);
						else
							return -atanf(Vect.m128_f32[0] / Vect.m128_f32[2]);
					}
				}

				__forceinline float GetP() const
				{
					if (fis_zero(Vect.m128_f32[0]) && fis_zero(Vect.m128_f32[2]))
					{
						if (!fis_zero(float(Vect.m128_f32[1])))
							return (Vect.m128_f32[1] > 0.0f) ? PI_DIV_2 : -PI_DIV_2;
						else
							return 0.0f;
					}
					else
					{
						float hyp = _sqrt(Vect.m128_f32[0] * Vect.m128_f32[0] + Vect.m128_f32[2] * Vect.m128_f32[2]);
						if (fis_zero(float(hyp)))
							return (Vect.m128_f32[1] > 0.0f) ? PI_DIV_2 : -PI_DIV_2;
						else
							return atanf(Vect.m128_f32[1] / hyp);
					}
				}
			};

		protected:
			/// <summary>Half fov <-> angle <-> tangent</summary>
			inline void build_projection_HAT(float HAT, float fAspect, float fNearPlane, float fFarPlane, DirectX::XMMATRIX &Matrics);

		public:
			Matrix4x4(DirectX::XMMATRIX a)
			{
				Matrix = a;

				x = Matrix.r[0];
				y = Matrix.r[1];
				z = Matrix.r[2];
				w = Matrix.r[3];
			}

			Matrix4x4(float val = 0.f)
			{

				Matrix =
				{
					val, val, val, val,
					val, val, val, val,
					val, val, val, val,
					val, val, val, val
				};

				x = Matrix.r[0];
				y = Matrix.r[1];
				z = Matrix.r[2];
				w = Matrix.r[3];
			}

			Matrix4x4
			(
				float a1, float a2, float a3, float a4,
				float b1, float b2, float b3, float b4,
				float c1, float c2, float c3, float c4,
				float d1, float d2, float d3, float d4
			)
			{
				Matrix =
				{
					a1, a2, a3, a4,
					b1, b2, b3, b4,
					c1, c2, c3, c4,
					d1, d2, d3, d4
				};

				x = Matrix.r[0];
				y = Matrix.r[1];
				z = Matrix.r[2];
				w = Matrix.r[3];
			}

			/// <summary>Generate new cam direction </summary>
			inline void BuildCamDir(const Fvector &vFrom, const Fvector &vView, const Fvector &vWorldUp);
			/// <summary>Generate new a Matrix Projection</summary>
			inline void BuildProj(float fFOV, float fAspect, float fNearPlane, float fFarPlane);
			/// <summary>Generate new ortho projection to matrix </summary>
			inline void BuildProjOrtho(float w, float h, float zn, float zf);

			inline void Identity()
			{
				Matrix = DirectX::XMMatrixIdentity();
			}

			inline void SetHPB(float h, float p, float b);
			inline void GetHPB(float &h, float &p, float &b) const
			{
				float cy = _sqrt(y[1]*y[1] + x[1]*x[1]);
				if (cy > 16.0f*type_epsilon<float>)
				{
					h -= atan2(z[1], z[2]);
					p -= atan2(-z[1], cy);
					b -= atan2(x[1], y[1]);
				}
				else 
				{
					h -= atan2(-x[2], x[0]);
					b = 0;
				}
			}

			/// <summary>Multiplication matrix by matrix</summary>
			inline void Multiply(Matrix4x4 a, Matrix4x4 b) { Matrix = DirectX::XMMatrixMultiply(a, b); }

			inline void Multiply43(Matrix4x4 a, Matrix4x4 b)
			{
				Matrix = DirectX::XMMatrixMultiply(a, b);
				x[3] = 0;
				y[3] = 0;
				z[3] = 0;
				w[3] = 1;
			}

			/// <summary>Inversion matrix by matrix</summary>
			inline void InvertMatrixByMatrix(const DirectX::XMMATRIX &a);

			inline void InvertMatrixByMatrix43(const DirectX::XMMATRIX &a);
			/// <summary>Call Fbox::xform for DirectX::XMMATRIX</summary>
			inline void BuildXForm(Fbox &B);

			inline void xform_get(const Fobb& D)
			{
				x = { D.m_rotate.i.x, D.m_rotate.i.y, D.m_rotate.i.z, 0 };
				y = { D.m_rotate.j.x, D.m_rotate.j.y, D.m_rotate.j.z, 0 };
				z = { D.m_rotate.k.x, D.m_rotate.k.y, D.m_rotate.k.z, 0 };
				w = { D.m_translate.x, D.m_translate.y, D.m_translate.z, 0 };
			}
			inline void Translate(Fvector3 diff)
			{
				Matrix = DirectX::XMMatrixTranslation(diff.x, diff.y, diff.z);
			}

			inline void Inverse(DirectX::XMVECTOR* pDeterminant, const DirectX::XMMATRIX& refMatrix)
			{
				Matrix = DirectX::XMMatrixInverse(pDeterminant, refMatrix);
			}

			inline void Transform(Fvector &dest, const Fvector &v) const noexcept
			{
				XRay::Math::TransformVectorsByMatrix(Matrix, dest, v);
			}

			inline void Transform(Fvector4 &dest, const Fvector4 &v) const noexcept
			{
				XRay::Math::TransformVectorsByMatrix(Matrix, dest, v);
			}

			inline void TransformDir(Fvector &dest, const Fvector &v) const noexcept
			{
				XRay::Math::TransformDirByMatrix(Matrix, dest, v);
			}

			inline void TransformDir(Fvector &dest) const noexcept
			{
				XRay::Math::TransformDirByMatrix(Matrix, dest);
			}

			inline void Transform(Fvector &dest) const noexcept
			{
				Fvector res;
				Transform(res, dest);
				dest.set(res);
			}

			inline void TransformTiny(Fvector &dest, const Fvector &v) const noexcept
			{
				dest.x = v.x* x[0] + v.y* y[0] + v.z* z[0] + w[0];
				dest.y = v.x* x[1] + v.y* y[1] + v.z* z[1] + w[1];
				dest.z = v.x* x[2] + v.y* y[2] + v.z* z[2] + w[2];
			}

			inline void Scale(float ax, float ay, float az) noexcept
			{
				Matrix = DirectX::XMMatrixScaling(ax, ay, az);
			}

			inline void TransformTiny(Fvector &v) const
			{
				Fvector res;
				TransformTiny(res, v);
				v.set(res);
			}

			inline void TranslateOver(const Fvector &v) noexcept
			{
				w = { v.x, v.y, v.z, w[3] };
			}

			inline void mk_xform(const _quaternion<float> &Q, const Fvector &V) noexcept
			{
				float xx = Q.x*Q.x; float yy = Q.y*Q.y; float zz = Q.z*Q.z;
				float xy = Q.x*Q.y; float xz = Q.x*Q.z; float yz = Q.y*Q.z;
				float wx = Q.w*Q.x; float wy = Q.w*Q.y; float wz = Q.w*Q.z;

				x[0] = 1 - 2 * (yy + zz);	x[1] = 2 * (xy - wz);		x[2] = 2 * (xz + wy);		x[3] = 0;
				y[0] = 2 * (xy + wz);		y[1] = 1 - 2 * (xx + zz);	y[2] = 2 * (yz - wx);		y[3] = 0;
				z[0] = 2 * (xz - wy);		z[1] = 2 * (yz + wx);		z[2] = 1 - 2 * (xx + yy);	z[3] = 0;
				w[0] = V.x;					w[1] = V.y;					w[2] = V.z;					w[3] = 1;
			}

		public:
			union
			{
				struct
				{
					IntricsVect x;
					IntricsVect y;
					IntricsVect z;
					IntricsVect w;
				};

				DirectX::XMMATRIX Matrix;
			};

		public:
			inline operator const Fmatrix& () const { return CastToGSCMatrix(Matrix); }
			inline operator DirectX::XMMATRIX() { return Matrix; }
			inline operator DirectX::XMMATRIX() const { return Matrix; }
			inline void operator= (const DirectX::XMMATRIX &a) { Matrix = a; }
			inline void operator= (const Matrix4x4 &a) { Matrix = a.Matrix; }
			inline void operator= (const Fmatrix &a)
			{
				Matrix =
				{
					a._11, a._12, a._13, a._14,
					a._21, a._22, a._23, a._24,
					a._31, a._32, a._33, a._34,
					a._41, a._42, a._43, a._44
				};
			}
			inline Matrix4x4 operator*(const Matrix4x4 &a)
			{
				return 
				{
					x[0] * a.x[0], x[1] * a.x[1], x[2] * a.x[2], x[3] * a.x[3],
					y[0] * a.y[0], y[1] * a.y[1], y[2] * a.y[2], y[3] * a.y[3],
					z[0] * a.z[0], z[1] * a.z[1], z[2] * a.z[2], z[3] * a.z[3],
					w[0] * a.w[0], w[1] * a.w[1], w[2] * a.w[2], w[3] * a.w[3]
				};
			}
		};

		inline void Matrix4x4::build_projection_HAT(float HAT, float fAspect, float fNearPlane, float fFarPlane, DirectX::XMMATRIX &Matrics)
		{
			float cot = 1.f / HAT;
			float W = fAspect * cot;
			float h = 1.f * cot;
			float Q = fFarPlane / (fFarPlane - fNearPlane);

			Matrics = DirectX::XMMATRIX(
				W, 0, 0, 0,
				0, h, 0, 0,
				0, 0, Q, 1.f,
				0, 0, -Q * fNearPlane, 0);
		}

		inline void Matrix4x4::BuildProj(float fFOV, float fAspect, float fNearPlane, float fFarPlane)
		{
			return build_projection_HAT(tanf(fFOV / 2.f), fAspect, fNearPlane, fFarPlane, Matrix);
		}

		inline void Matrix4x4::BuildProjOrtho(float W, float h, float zn, float zf)
		{
			Matrix = DirectX::XMMATRIX(
				2.f / W, 0, 0, 0,
				0, 2.f / h, 0, 0,
				0, 0, 1.f / (zf - zn), 0,
				0, 0, zn / (zn - zf), 1.f);
		}

		inline void Matrix4x4::BuildXForm(Fbox &B)
		{
			const Fmatrix bbox = *this;
			B.xform(bbox);
		}

		inline void Matrix4x4::BuildCamDir(const Fvector &vFrom, const Fvector &vView, const Fvector &vWorldUp)
		{
			// Get the dot product, and calculate the projection of the z basis
			// vector3 onto the up vector3. The projection is the y basis vector3.
			float fDotProduct = vWorldUp.dotproduct(vView);

			Fvector vUp;
			vUp.mul(vView, -fDotProduct).add(vWorldUp).normalize();

			// The x basis vector3 is found simply with the cross product of the y
			// and z basis vectors
			Fvector vRight;
			vRight.crossproduct(vUp, vView);

			// Start building the Device.mView. The first three rows contains the basis
			// vectors used to rotate the view to point at the lookat point
			Matrix = DirectX::XMMATRIX(
				vRight.x, vUp.x, vView.x, 0.0f,
				vRight.y, vUp.y, vView.y, 0.0f,
				vRight.z, vUp.z, vView.z, 0.0f,
				-vFrom.dotproduct(vRight), -vFrom.dotproduct(vUp), -vFrom.dotproduct(vView), 1.f);
		}

		inline void Matrix4x4::InvertMatrixByMatrix(const DirectX::XMMATRIX &a)
		{
			// faster than self-invert
			float fDetInv = (a.r[0].m128_f32[0] * (a.r[1].m128_f32[1] * a.r[2].m128_f32[2] - a.r[1].m128_f32[2] * a.r[2].m128_f32[1]) -
				a.r[0].m128_f32[1] * (a.r[1].m128_f32[0] * a.r[2].m128_f32[2] - a.r[1].m128_f32[2] * a.r[2].m128_f32[0]) +
				a.r[0].m128_f32[2] * (a.r[1].m128_f32[0] * a.r[2].m128_f32[1] - a.r[1].m128_f32[1] * a.r[2].m128_f32[0]));

			VERIFY(_abs(fDetInv) > flt_zero);
			fDetInv = 1.0f / fDetInv;

			Matrix.r[0].m128_f32[0] = fDetInv * (a.r[1].m128_f32[1] * a.r[2].m128_f32[2] - a.r[1].m128_f32[2] * a.r[2].m128_f32[1]);
			Matrix.r[0].m128_f32[1] = -fDetInv * (a.r[0].m128_f32[1] * a.r[2].m128_f32[2] - a.r[0].m128_f32[2] * a.r[2].m128_f32[1]);
			Matrix.r[0].m128_f32[2] = fDetInv * (a.r[0].m128_f32[1] * a.r[1].m128_f32[2] - a.r[0].m128_f32[2] * a.r[1].m128_f32[1]);
			Matrix.r[0].m128_f32[3] = 0.0f;

			Matrix.r[1].m128_f32[0] = -fDetInv * (a.r[1].m128_f32[0] * a.r[2].m128_f32[2] - a.r[1].m128_f32[2] * a.r[2].m128_f32[0]);
			Matrix.r[1].m128_f32[1] = fDetInv * (a.r[0].m128_f32[0] * a.r[2].m128_f32[2] - a.r[0].m128_f32[2] * a.r[2].m128_f32[0]);
			Matrix.r[1].m128_f32[2] = -fDetInv * (a.r[0].m128_f32[0] * a.r[1].m128_f32[2] - a.r[0].m128_f32[2] * a.r[1].m128_f32[0]);
			Matrix.r[1].m128_f32[3] = 0.0f;

			Matrix.r[2].m128_f32[0] = fDetInv * (a.r[1].m128_f32[0] * a.r[2].m128_f32[1] - a.r[1].m128_f32[1] * a.r[2].m128_f32[0]);
			Matrix.r[2].m128_f32[1] = -fDetInv * (a.r[0].m128_f32[0] * a.r[2].m128_f32[1] - a.r[0].m128_f32[1] * a.r[2].m128_f32[0]);
			Matrix.r[2].m128_f32[2] = fDetInv * (a.r[0].m128_f32[0] * a.r[1].m128_f32[1] - a.r[0].m128_f32[1] * a.r[1].m128_f32[0]);
			Matrix.r[2].m128_f32[3] = 0.0f;

			Matrix.r[3].m128_f32[0] = -(a.r[3].m128_f32[0] * Matrix.r[0].m128_f32[0] + a.r[3].m128_f32[1] * Matrix.r[1].m128_f32[0] + a.r[3].m128_f32[2] * Matrix.r[2].m128_f32[0]);
			Matrix.r[3].m128_f32[1] = -(a.r[3].m128_f32[0] * Matrix.r[0].m128_f32[1] + a.r[3].m128_f32[1] * Matrix.r[1].m128_f32[1] + a.r[3].m128_f32[2] * Matrix.r[2].m128_f32[1]);
			Matrix.r[3].m128_f32[2] = -(a.r[3].m128_f32[0] * Matrix.r[0].m128_f32[2] + a.r[3].m128_f32[1] * Matrix.r[1].m128_f32[2] + a.r[3].m128_f32[2] * Matrix.r[2].m128_f32[2]);
			Matrix.r[3].m128_f32[3] = 1.0f;
		}

		inline void Matrix4x4::InvertMatrixByMatrix43(const DirectX::XMMATRIX &a)
		{
			InvertMatrixByMatrix(a);

			x[3] = 0;
			y[3] = 0;
			z[3] = 0;
			w[3] = 1;
		}

		inline void Matrix4x4::SetHPB(float h, float p, float b)
		{
			float _ch, _cp, _cb, _sh, _sp, _sb, _cc, _cs, _sc, _ss;

			_sh = _sin(h); _ch = _cos(h);
			_sp = _sin(p); _cp = _cos(p);
			_sb = _sin(b); _cb = _cos(b);
			_cc = _ch * _cb; _cs = _ch * _sb; _sc = _sh * _cb; _ss = _sh * _sb;

			x = { _cc - _sp * _ss, -_cp * _sb, _sp*_cs + _sc,  0 };
			y = { _sp*_sc + _cs, _cp*_cb, _ss - _sp * _cc, 0 };
			z = { -_cp * _sh, _sp, _cp*_ch,  0 };
			w = { 0, 0, 0, 1 };
		}

	}
};

using XRay::Math::Matrix4x4;