#pragma once
inline float clamp_rotation(Fquaternion &q, float v)
{
	float angl; Fvector ax;
	q.get_axis_angle(ax, angl);
	float abs_angl = _abs(angl);

	if (abs_angl > v)
	{
		if (angl < 0.f) v = -v;
		q.rotation(ax, v);
		q.normalize();
	}

	return abs_angl;
}

inline float  clamp_rotation(Fmatrix &m, float v)
{
	Fquaternion q;
	q.set(m);
	float r = clamp_rotation(q, v);
	Fvector c = m.c;
	m.rotation(q);
	m.c = c;
	return r;
}

inline void get_axis_angle(const Matrix4x4 &m, Fvector &ax, float &angl)
{
	Fquaternion q;
	q.set(CastToGSCMatrix(m));
	q.get_axis_angle(ax, angl);
}

inline bool clamp_change(Matrix4x4& m, const Matrix4x4 &start, float ml, float ma, float tl, float ta)
{
	Matrix4x4 InvMatrix;
	InvMatrix.InvertMatrixByMatrix(start);
	Matrix4x4 diff; diff.Multiply43(m, InvMatrix);
	float linear_ch = CastToGSCMatrix(diff).c.magnitude();
	bool ret = linear_ch < tl;

	if (linear_ch > ml)
		CastToGSCMatrix(diff).c.mul(ml / linear_ch);

	if (clamp_rotation(CastToGSCMatrix(diff), ma) > ta)
		ret = false;

	if (!ret)
		m.Multiply43(diff, start);

	return ret;
}

inline void get_diff_value(const Matrix4x4 & m0, const Matrix4x4 &m1, float &l, float &a)
{
	Matrix4x4 InvMatrix;
	InvMatrix.InvertMatrixByMatrix(m1);
	Matrix4x4 diff; diff.Multiply43(m0, InvMatrix);

	l = CastToGSCMatrix(diff).c.magnitude();
	Fvector ax;
	get_axis_angle(diff, ax, a);
	a = _abs(a);
}

inline void	cmp_matrix(bool &eq_linear, bool &eq_angular, const Matrix4x4 &m0, const Matrix4x4 &m1, float tl, float ta)
{
	float l, a;
	get_diff_value(m0, m1, l, a);
	eq_linear = l < tl; eq_angular = a < ta;
}

inline bool cmp_matrix(const Matrix4x4 &m0, const Matrix4x4 &m1, float tl, float ta)
{
	bool l = false, a = false;
	cmp_matrix(l, a, m0, m1, tl, ta);
	return l && a;
}

inline void angular_diff(Fvector &aw, const Matrix4x4 &diff, float dt)
{
	aw.set((diff.z[1] - diff.y[2]) / 2.f / dt, (diff.x[2] - diff.z[0]) / 2.f / dt, (diff.y[0] - diff.x[1]) / 2.f / dt);
}

inline void linear_diff(Fvector &lv, const Fvector &diff, float dt)
{
	lv.mul(diff, (1.f / dt));
}

inline void linear_diff(Fvector &lv, const Fvector &mc1, const Fvector &mc0, float dt)
{
	linear_diff(lv, Fvector().sub(mc1, mc0), dt);
}

inline void matrix_diff(Fvector &lv, Fvector &aw, const Matrix4x4 &diff, float dt)
{
	angular_diff(aw, diff, dt);
	linear_diff(lv, diff.z, dt);
}

inline void matrix_diff(Fvector &lv, Fvector &aw, const Matrix4x4 &m0, const Matrix4x4 &m1, float dt)
{
	Matrix4x4 InvMatrix;
	InvMatrix.InvertMatrixByMatrix(m0);
	Matrix4x4 diff; diff.Multiply43(m1, InvMatrix);

	matrix_diff(lv, aw, diff, dt);
}