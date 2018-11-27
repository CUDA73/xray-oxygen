#include "stdafx.h"
#pragma hdrstop

#include "r_backend_xform.h"

void	R_xforms::set_W			(const Matrix4x4& m)
{
	m_w = m;
	m_wv.Multiply43(m_w, m_v);
	m_wvp.Multiply(m_wv, m_p);
	if (c_w)		RCache.set_c(c_w,	m_w);
	if (c_wv)		RCache.set_c(c_wv,	m_wv);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	m_bInvWValid	= false;
	if (c_invw)		apply_invw();
	RCache.set_xform(D3DTS_WORLD,m);
}
void	R_xforms::set_V			(const Matrix4x4& m)
{
	m_v = m;
	m_wv.Multiply43(m_w, m_v);
	m_vp.Multiply(m_v, m_p);
	m_wvp.Multiply(m_wv, m_p);

	if (c_v)		RCache.set_c(c_v,	m_v);
	if (c_vp)		RCache.set_c(c_vp,	m_vp);
	if (c_wv)		RCache.set_c(c_wv,	m_wv);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);

	m_bInvVValid	= false;

	if (c_invv)		apply_invv();

	RCache.set_xform(D3DTS_VIEW,m);
}
void	R_xforms::set_P			(const Matrix4x4& m)
{
	m_p = (m);
	m_vp.Multiply(m_v, m_p);
	m_wvp.Multiply(m_wv, m_p);
	if (c_p)		RCache.set_c(c_p,	m_p);
	if (c_vp)		RCache.set_c(c_vp,	m_vp);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	m_bInvPValid	= false;
	if (c_invp)		apply_invp();
	// always setup projection - D3D relies on it to work correctly :(
	RCache.set_xform(D3DTS_PROJECTION,m);		
}

void	R_xforms::apply_invw()
{
	VERIFY(c_invw);

	if (!m_bInvWValid)
	{
		m_invw.InvertMatrixByMatrix43(m_w);
		m_bInvWValid = true;
	}

	RCache.set_c( c_invw, m_invw);
}

void	R_xforms::apply_invv()
{
	VERIFY(c_invv);

	if (!m_bInvVValid)
	{
		m_invv.InvertMatrixByMatrix43(m_v);
		m_bInvVValid = true;
	}
	RCache.set_c(c_invv, m_invv);
}

void	R_xforms::apply_invp()
{
	VERIFY(c_invp);

	if (!m_bInvPValid)
	{
		//#TODO: check if we need 4x4 invert for projection matrix. There is no one in xrCore :(
		m_invp.InvertMatrixByMatrix(m_p);
		m_bInvPValid = true;
	}
	RCache.set_c(c_invp, m_invp);
}

void	R_xforms::unmap			()
{
	c_w			= nullptr;
	c_invw		= nullptr;
	c_v			= nullptr;
	c_invv		= nullptr;
	c_p			= nullptr;
	c_invp		= nullptr;
	c_wv		= nullptr;
	c_vp		= nullptr;
	c_wvp		= nullptr;
}
R_xforms::R_xforms				()
{
	unmap			();
	m_w.Identity	();
	m_invw.Identity	();
	m_v.Identity	();
	m_invv.Identity	();
	m_p.Identity	();
	m_invp.Identity	();
	m_wv.Identity	();
	m_vp.Identity	();
	m_wvp.Identity	();
	m_bInvWValid = true;
	m_bInvVValid = true;
	m_bInvPValid = true;
}
