#pragma once

#include "../Include/xrRender/KinematicsAnimated.h"
#include "poses_blending.h"

class poses_blending;
class CBlend;

class XRPHYSICS_API animation_movement_controller : public  IBlendDestroyCallback
{
	Matrix4x4&			m_pObjXForm;
	Matrix4x4				m_startObjXForm;
	poses_blending		m_poses_blending;
public:
	void				DBG_verify_position_not_chaged() const;
private:
	IKinematics * m_pKinematicsC;
	IKinematicsAnimated *m_pKinematicsA;
	CBlend*				m_control_blend;
	bool				inital_position_blending;
	bool				stopped;
	float				blend_linear_speed;
	float				blend_angular_speed;
	static void	__stdcall	RootBoneCallback(CBoneInstance* B);
	void				deinitialize();
	void				BlendDestroy(CBlend& blend);
public:
	animation_movement_controller(Matrix4x4	*_pObjXForm, const Matrix4x4 &inital_pose, IKinematics *_pKinematicsC, CBlend *b);
	animation_movement_controller(const animation_movement_controller& other) = delete;
	animation_movement_controller& operator=(const animation_movement_controller& other) = delete;
	virtual		~animation_movement_controller();
	void	ObjStartXform(Matrix4x4 &m)const { m = (m_startObjXForm); }
	CBlend*	ControlBlend() const { return m_control_blend; }
	void	NewBlend(CBlend* B, const Matrix4x4 &new_matrix, bool local_animation);
	bool	IsActive() const;
	void	OnFrame();
private:
	void	GetInitalPositionBlenSpeed();
	void	animation_root_position(Matrix4x4 &pos);
	void	InitalPositionBlending(const Matrix4x4 &to);
	void	SetPosesBlending();
public:
	bool	IsBlending() const;
	inline	Matrix4x4 const& start_transform() const
	{
		return		(m_startObjXForm);
	}

	void	stop();
};