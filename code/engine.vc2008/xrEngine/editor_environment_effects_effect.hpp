////////////////////////////////////////////////////////////////////////////
//	Module 		: editor_environment_effects_effect.hpp
//	Created 	: 28.12.2007
//  Modified 	: 28.12.2007
//	Author		: Dmitriy Iassenev
//	Description : editor environment effects effect class
////////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef INGAME_EDITOR
#include "../include/editor/property_holder.hpp"
#include "environment.h"

namespace editor {
namespace environment {

class manager;

namespace effects {

class manager;

class effect :
	public CEnvAmbient::SEffect,
	public editor::property_holder_holder
{
public:
							effect		(manager const& manager, shared_str const& id);
                            effect(const effect&) = delete;
                            effect& operator= (const effect&) = delete;
	virtual					~effect		();
			void			load		(CInifile& config);
			void			save		(CInifile& config);
			void			fill		(editor::property_holder_collection* collection);
	inline	LPCSTR			id			() const { return m_id.c_str(); }


private:
	LPCSTR 		id_getter	() const;
	void   		id_setter	(LPCSTR value);

	float 		wind_blast_longitude_getter	() const;
	void  		wind_blast_longitude_setter	(float value);


	LPCSTR		sound_getter		();
	void 		sound_setter		(LPCSTR value);

private:
	typedef editor::property_holder	property_holder_type;

public:
	virtual	property_holder_type*object	();

private:
	shared_str				m_id;
	property_holder_type*	m_property_holder;
	shared_str				m_sound;

public:
	manager const&			m_manager;
}; // class effect

} // namespace effects
} // namespace environment
} // namespace editor

#endif // #ifdef INGAME_EDITOR