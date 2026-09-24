// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ambient

#include <string>
#include <cstring>
#include <unordered_map>

#include "m_random.hpp"
#include "lprintf.hpp"
#include "p_tick.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "w_wad.hpp"
#include "z_zone.hpp"
#include "dsda/sfx.hpp"

#include "scanner.hpp"

#include "ambient.hpp"

typedef struct
{
	char* lump_name;
	SfxId sfx_id;
} named_sfx_t;

std::unordered_map<std::string, named_sfx_t> name_to_sfx;
std::unordered_map<int, ambient_sfx_t> id_to_ambient_sfx;

dboolean dsda_IsLoopingAmbientSFX(SfxId sfx_id)
{
	for(auto& amb_sfx : id_to_ambient_sfx)
	{
		if(amb_sfx.second.sfx_id == sfx_id && amb_sfx.second.min_tics < 0)
			return true;
	}

	return false;
}

static ambient_sfx_t* dsda_AmbientSFX(int id)
{
	return id_to_ambient_sfx[id].sfx_id != SfxId::None ? &id_to_ambient_sfx[id] : nullptr;
}

static int dsda_AmbientWaitTime(ambient_sfx_t* amb_sfx)
{
	return amb_sfx->min_tics + M_Random() * (amb_sfx->max_tics - amb_sfx->min_tics) / 255;
}

void dsda_UpdateAmbientSource(ambient_source_t* source)
{
	if(source->wait_tics > 0)
	{
		--source->wait_tics;
		return;
	}

	// looping
	if(source->data.min_tics < 0)
	{
		source->wait_tics = TICRATE;

		if(!source->data.attenuation)
		{
			S_AdjustVolume(source->data.volume);
			S_LoopVoidSound(source->data.sfx_id, source->wait_tics + 1);
			S_ResetAdjustments();
		}
		else
		{
			S_AdjustAttenuation(source->data.attenuation);
			S_AdjustVolume(source->data.volume);
			S_LoopMobjSound(source->mobj, source->data.sfx_id, source->wait_tics + 1);
			S_ResetAdjustments();
		}
	}
	else
	{
		source->wait_tics = dsda_AmbientWaitTime(&source->data);

		if(!source->data.attenuation)
		{
			S_AdjustVolume(source->data.volume);
			S_StartVoidSound(source->data.sfx_id);
			S_ResetAdjustments();
		}
		else
		{
			S_AdjustAttenuation(source->data.attenuation);
			S_AdjustVolume(source->data.volume);
			S_StartMobjSound(source->mobj, source->data.sfx_id);
			S_ResetAdjustments();
		}
	}
}

void dsda_SpawnAmbientSource(mobj_t* mobj)
{
	ambient_source_t* source;
	ambient_sfx_t* data;

	data = dsda_AmbientSFX(mobj->iden_nums);
	if(!data)
		return;

	source = (ambient_source_t*)Z_MallocLevel(sizeof(*source));
	source->mobj = nullptr;
	P_SetTarget(&source->mobj, mobj);
	source->data = *data;
	source->data.sound_name = nullptr;
	source->wait_tics = dsda_AmbientWaitTime(data);
	source->thinker.function = (think_t)dsda_UpdateAmbientSource;
	P_AddThinker(&source->thinker);
}

static void dsda_ParseAmbient(Scanner& scanner)
{
	ambient_sfx_t amb_sfx = {nullptr};
	int id;

	scanner.MustGetInteger();
	id = scanner.number;

	scanner.MustGetString();
	amb_sfx.sound_name = Z_Strdup(scanner.string);

	scanner.MustGetString();
	if(scanner.StringMatch("point"))
	{
		amb_sfx.attenuation = scanner.CheckFloat() ? scanner.decimal : 1.0;
		scanner.MustGetString();
	}
	else if(scanner.StringMatch("world") || scanner.StringMatch("surround"))
	{
		amb_sfx.attenuation = 0;
		scanner.MustGetString();
	}
	else
	{
		amb_sfx.attenuation = 0;
	}

	if(scanner.StringMatch("continuous"))
	{
		amb_sfx.min_tics = -1;
		amb_sfx.max_tics = -1;
	}
	else if(scanner.StringMatch("random"))
	{
		scanner.MustGetFloat();
		amb_sfx.min_tics = TICRATE * scanner.decimal;
		scanner.MustGetFloat();
		amb_sfx.max_tics = TICRATE * scanner.decimal;
	}
	else if(scanner.StringMatch("periodic"))
	{
		scanner.MustGetFloat();
		amb_sfx.min_tics = TICRATE * scanner.decimal;
		amb_sfx.max_tics = amb_sfx.min_tics;
	}

	scanner.MustGetFloat();
	amb_sfx.volume = BETWEEN(0.0, 1.0, scanner.decimal);
	if(amb_sfx.attenuation < 0)
		amb_sfx.attenuation = 1;

	if(!amb_sfx.min_tics && !amb_sfx.max_tics)
	{
		lprintf(OutputLevels::Warn, "Ambient sound %d has invalid parameters\n", id);
		Z_Free(amb_sfx.sound_name);
		return;
	}

	if(id_to_ambient_sfx[id].sound_name)
		Z_Free(id_to_ambient_sfx[id].sound_name);

	id_to_ambient_sfx[id] = amb_sfx;
}

static void dsda_ParseSndInfoLine(Scanner& scanner)
{
	if(!scanner.CheckString())
	{
		scanner.GetNextToken();
		scanner.SkipLine();
		return;
	}

	if(!stricmp(scanner.string, "$ambient"))
		dsda_ParseAmbient(scanner);
	else if(scanner.string[0] == '$')
	{
		scanner.SkipLine();
		return;
	}
	else
	{
		std::string name(scanner.string);

		scanner.CheckToken(static_cast<TokenType>('=')); // Optional

		if(!scanner.CheckString())
		{
			lprintf(OutputLevels::Warn, "Invalid SNDINFO: name \"%s\" expects string sound lump\n", name.c_str());

			scanner.GetNextToken();
			scanner.SkipLine();
			return;
		}

		if(!W_LumpNameExists(scanner.string))
		{
			lprintf(OutputLevels::Warn, "Sound lump \"%s\" does not exist\n", scanner.string);
			return;
		}

		if(name_to_sfx[name].lump_name)
			Z_Free(name_to_sfx[name].lump_name);

		name_to_sfx[name].lump_name = Z_Strdup(scanner.string);
	}
}

static void dsda_ResolveAmbientSounds()
{
	for(auto& named_sfx : name_to_sfx)
	{
		sfxinfo_t* new_sfx;
		int id;

		new_sfx = dsda_NewSFX(&id);
		new_sfx->name = named_sfx.second.lump_name;
		named_sfx.second.sfx_id = static_cast<SfxId>(id);

		lprintf(OutputLevels::Debug, "Named sound: %s -> %s %d\n", named_sfx.first.c_str(), new_sfx->name, id);
	}

	for(auto& amb_sfx : id_to_ambient_sfx)
	{
		amb_sfx.second.sfx_id = name_to_sfx[amb_sfx.second.sound_name].sfx_id;

		if(amb_sfx.second.sfx_id == SfxId::None)
			lprintf(OutputLevels::Warn, "Sound \"%s\" does not exist\n", amb_sfx.second.sound_name);

		lprintf(OutputLevels::Debug, "Ambient sound: %d att: %f, vol: %f, t: %d %d, sfx: %d\n",
			amb_sfx.first,
			amb_sfx.second.attenuation,
			amb_sfx.second.volume,
			amb_sfx.second.min_tics,
			amb_sfx.second.max_tics,
			amb_sfx.second.sfx_id);
	}
}

void dsda_LoadAmbientSndInfo()
{
	int lump;

	lump = W_CheckNumForName("SNDINFO");

	if(lump == LUMP_NOT_FOUND)
		return;

	Scanner scanner((const char*)W_LumpByNum(lump), W_LumpLength(lump));

	scanner.SetErrorCallback(I_Error);

	while(scanner.TokensLeft())
		dsda_ParseSndInfoLine(scanner);

	dsda_ResolveAmbientSounds();
}
