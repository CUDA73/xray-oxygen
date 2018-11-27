#pragma once

class ECORE_API	R_xforms
{
public:
	Matrix4x4		m_w;		// Basic	- world
	Matrix4x4		m_invw;		// Derived	- world2local, cached
	Matrix4x4		m_v;		// Basic	- view
	Matrix4x4		m_invv;		// Derived	- view2world
	Matrix4x4		m_p;		// Basic	- projection
	Matrix4x4		m_invp;		// Derived	- projection2view
	Matrix4x4		m_wv;		// Derived	- world2view
	Matrix4x4		m_vp;		// Derived	- view2projection
	Matrix4x4		m_wvp;		// Derived	- world2view2projection

	R_constant*		c_w;
	R_constant*		c_invw;
	R_constant*		c_v;
	R_constant*		c_invv;
	R_constant*		c_p;
	R_constant*		c_invp;
	R_constant*		c_wv;
	R_constant*		c_vp;
	R_constant*		c_wvp;
private:
	bool			m_bInvWValid;
	bool			m_bInvVValid;
	bool			m_bInvPValid;
public:
	R_xforms		();
	void			unmap		();
	void			set_W		(const Matrix4x4& m);
	void			set_V		(const Matrix4x4& m);
	void			set_P		(const Matrix4x4& m);
	IC const Matrix4x4&	get_W	()					{ return m_w;	}
	IC const Matrix4x4&	get_V	()					{ return m_v;	}
	IC const Matrix4x4&	get_P	()					{ return m_p;	}
	IC void			set_c_w		(R_constant* C);
	IC void			set_c_invw	(R_constant* C);
	IC void			set_c_v		(R_constant* C);
	IC void			set_c_invv	(R_constant* C);
	IC void			set_c_p		(R_constant* C);
	IC void			set_c_invp	(R_constant* C);
	IC void			set_c_wv	(R_constant* C);
	IC void			set_c_vp	(R_constant* C);
	IC void			set_c_wvp	(R_constant* C);
private:
	void			apply_invw	();
	void			apply_invv	();
	void			apply_invp	();
};
