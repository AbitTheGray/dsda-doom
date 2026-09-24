// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ambient

#pragma once

#include "p_mobj.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		char* sound_name;
		SfxId sfx_id;
		float attenuation;
		float volume;
		int min_tics;
		int max_tics;
	} ambient_sfx_t;

	typedef struct
	{
		thinker_t thinker;
		mobj_t* mobj;
		ambient_sfx_t data;
		int wait_tics;
	} ambient_source_t;

	dboolean dsda_IsLoopingAmbientSFX(SfxId sfx_id);
	void dsda_UpdateAmbientSource(ambient_source_t* source);
	void dsda_SpawnAmbientSource(mobj_t* mobj);
	void dsda_LoadAmbientSndInfo();

#ifdef __cplusplus
}
#endif
