// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA MapInfo

#include <string.h>

#include "doomstat.hpp"
#include "dsda/episode.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo/hexen.hpp"
#include "dsda/mapinfo/u.hpp"
#include "dsda/mapinfo/legacy.hpp"

#include "mapinfo.hpp"

int dsda_NameToMap(const char* name, int* episode, int* map)
{
	int found;

	if(dsda_HexenNameToMap(&found, name, episode, map))
		return found;

	if(dsda_UNameToMap(&found, name, episode, map))
		return found;

	dsda_LegacyNameToMap(&found, name, episode, map);

	return found;
}

void dsda_FirstMap(int* episode, int* map)
{
	if(dsda_HexenFirstMap(episode, map))
		return;

	if(dsda_UFirstMap(episode, map))
		return;

	dsda_LegacyFirstMap(episode, map);
}

void dsda_NewGameMap(int* episode, int* map)
{
	if(dsda_HexenNewGameMap(episode, map))
		return;

	if(dsda_UNewGameMap(episode, map))
		return;

	dsda_LegacyNewGameMap(episode, map);
}

void dsda_ResolveWarp(int* args, int arg_count, int* episode, int* map)
{
	if(dsda_HexenResolveWarp(args, arg_count, episode, map))
		return;

	if(dsda_UResolveWarp(args, arg_count, episode, map))
		return;

	dsda_LegacyResolveWarp(args, arg_count, episode, map);
}

void dsda_NextMap(int* episode, int* map)
{
	if(dsda_HexenNextMap(episode, map))
		return;

	if(dsda_UNextMap(episode, map))
		return;

	dsda_LegacyNextMap(episode, map);
}

void dsda_PrevMap(int* episode, int* map)
{
	if(dsda_HexenPrevMap(episode, map))
		return;

	if(dsda_UPrevMap(episode, map))
		return;

	dsda_LegacyPrevMap(episode, map);
}

void dsda_ShowNextLocBehaviour(ShowNextLocFlag* behaviour)
{
	if(dsda_HexenShowNextLocBehaviour(behaviour))
		return;

	if(dsda_UShowNextLocBehaviour(behaviour))
		return;

	dsda_LegacyShowNextLocBehaviour(behaviour);
}

int dsda_SkipDrawShowNextLoc()
{
	int skip;

	if(dsda_HexenSkipDrawShowNextLoc(&skip))
		return skip;

	if(dsda_USkipDrawShowNextLoc(&skip))
		return skip;

	dsda_LegacySkipDrawShowNextLoc(&skip);

	return skip;
}

static fixed_t dsda_Gravity()
{
	fixed_t gravity;

	if(dsda_HexenGravity(&gravity))
		return gravity;

	if(dsda_UGravity(&gravity))
		return gravity;

	dsda_LegacyGravity(&gravity);

	return gravity;
}

static fixed_t dsda_AirControl()
{
	fixed_t air_control;

	if(dsda_HexenAirControl(&air_control))
		return air_control;

	if(dsda_UAirControl(&air_control))
		return air_control;

	dsda_LegacyAirControl(&air_control);

	return air_control;
}

static int dsda_MapColorMap()
{
	int colormap;

	if(dsda_HexenMapColorMap(&colormap))
		return colormap;

	if(dsda_UMapColorMap(&colormap))
		return colormap;

	dsda_LegacyMapColorMap(&colormap);

	return colormap;
}

static void dsda_UpdateMapInfo()
{
	dsda_HexenUpdateMapInfo();
	dsda_UUpdateMapInfo();
	dsda_LegacyUpdateMapInfo();

	map_colormap = dsda_MapColorMap();
	map_gravity = dsda_Gravity();
	map_aircontrol = dsda_AirControl();
	map_airfriction = map_aircontrol > 256
		? 65560 - FixedMul(map_aircontrol, 6168)
		: FRACUNIT;
}

void dsda_UpdateGameMap(int episode, int map)
{
	gameepisode = episode;
	gamemap = map;
	dsda_UpdateMapInfo();
}

extern "C" void dsda_ResetAirControl()
{
	map_aircontrol = dsda_AirControl();
}

void dsda_ResetLeaveData()
{
	memset(&leave_data, 0, sizeof(leave_data));
}

void dsda_UpdateLeaveData(int map, int position, int flags, angle_t angle)
{
	leave_data.map = map;
	leave_data.position = position;
	leave_data.flags = flags;
	leave_data.angle = angle;
}

dboolean dsda_FinaleShortcut()
{
	return map_format.zdoom && leave_data.map == LEAVE_VICTORY;
}

void dsda_UpdateLastMapInfo()
{
	dsda_HexenUpdateLastMapInfo();
	dsda_UUpdateLastMapInfo();
	dsda_LegacyUpdateLastMapInfo();
}

void dsda_UpdateNextMapInfo()
{
	dsda_HexenUpdateNextMapInfo();
	dsda_UUpdateNextMapInfo();
	dsda_LegacyUpdateNextMapInfo();
}

int dsda_ResolveCLEV(int* episode, int* map)
{
	int clev;

	if(dsda_HexenResolveCLEV(&clev, episode, map))
		return clev;

	if(dsda_UResolveCLEV(&clev, episode, map))
		return clev;

	dsda_LegacyResolveCLEV(&clev, episode, map);

	return clev;
}

int dsda_ResolveINIT()
{
	int init;

	if(dsda_HexenResolveINIT(&init))
		return init;

	if(dsda_UResolveINIT(&init))
		return init;

	dsda_LegacyResolveINIT(&init);

	return init;
}

int dsda_MusicIndexToLumpNum(int music_index)
{
	int lump;

	if(dsda_HexenMusicIndexToLumpNum(&lump, music_index))
		return lump;

	if(dsda_UMusicIndexToLumpNum(&lump, music_index))
		return lump;

	dsda_LegacyMusicIndexToLumpNum(&lump, music_index);

	return lump;
}

void dsda_MapMusic(int* music_index, int* music_lump, int episode, int map)
{
	if(dsda_HexenMapMusic(music_index, music_lump, episode, map))
		return;

	if(dsda_UMapMusic(music_index, music_lump, episode, map))
		return;

	dsda_LegacyMapMusic(music_index, music_lump, episode, map);
}

void dsda_IntermissionMusic(int* music_index, int* music_lump)
{
	if(dsda_HexenIntermissionMusic(music_index, music_lump))
		return;

	if(dsda_UIntermissionMusic(music_index, music_lump))
		return;

	dsda_LegacyIntermissionMusic(music_index, music_lump);
}

void dsda_InterMusic(int* music_index, int* music_lump)
{
	if(dsda_HexenInterMusic(music_index, music_lump))
		return;

	if(dsda_UInterMusic(music_index, music_lump))
		return;

	dsda_LegacyInterMusic(music_index, music_lump);
}

enum struct FinaleOwner : int32_t
{
	Legacy,
	U,
	Hexen,
};

static FinaleOwner finale_owner = FinaleOwner::Legacy;

void dsda_StartFinale()
{
	if(dsda_HexenStartFinale())
	{
		finale_owner = FinaleOwner::Hexen;
		return;
	}

	if(dsda_UStartFinale())
	{
		finale_owner = FinaleOwner::U;
		return;
	}

	dsda_LegacyStartFinale();
	finale_owner = FinaleOwner::Legacy;
}

int dsda_FTicker()
{
	if(finale_owner == FinaleOwner::Hexen)
	{
		if(!dsda_HexenFTicker())
			finale_owner = FinaleOwner::Legacy;

		return true;
	}

	if(finale_owner == FinaleOwner::U)
	{
		if(!dsda_UFTicker())
			finale_owner = FinaleOwner::Legacy;

		return true;
	}

	dsda_LegacyFTicker();
	return false;
}

int dsda_FDrawer()
{
	if(finale_owner == FinaleOwner::Hexen)
	{
		dsda_HexenFDrawer();

		return true;
	}

	if(finale_owner == FinaleOwner::U)
	{
		dsda_UFDrawer();

		return true;
	}

	dsda_LegacyFDrawer();
	return false;
}

int dsda_BossAction(mobj_t* mo)
{
	if(dsda_HexenBossAction(mo))
		return true;

	if(dsda_UBossAction(mo))
		return true;

	dsda_LegacyBossAction(mo);
	return false;
}

std::string_view dsda_MapLumpName(int episode, int map)
{
	const char* name;

	if(dsda_HexenMapLumpName(&name, episode, map))
		return name;

	if(dsda_UMapLumpName(&name, episode, map))
		return name;

	dsda_LegacyMapLumpName(&name, episode, map);

	return name;
}

void dsda_HUTitle(dsda_string_t* str)
{
	if(dsda_HexenHUTitle(str))
		return;

	if(dsda_UHUTitle(str))
		return;

	dsda_LegacyHUTitle(str);
}

const char* dsda_MapAuthor()
{
	const char* author;

	if(dsda_HexenMapAuthor(&author))
		return author;

	if(dsda_UMapAuthor(&author))
		return author;

	dsda_LegacyMapAuthor(&author);

	return author;
}

int dsda_SkyTexture()
{
	int sky;

	if(dsda_HexenSkyTexture(&sky))
		return sky;

	if(dsda_USkyTexture(&sky))
		return sky;

	dsda_LegacySkyTexture(&sky);

	return sky;
}

void dsda_PrepareInitNew()
{
	if(dsda_HexenPrepareInitNew())
		return;

	if(dsda_UPrepareInitNew())
		return;

	dsda_LegacyPrepareInitNew();
}

void dsda_PrepareIntermission(DoCompletedFlag* behaviour)
{
	if(dsda_HexenPrepareIntermission(behaviour))
		return;

	if(dsda_UPrepareIntermission(behaviour))
		return;

	dsda_LegacyPrepareIntermission(behaviour);
}

void dsda_PrepareFinale(WorldDoneFlag* behaviour)
{
	if(dsda_HexenPrepareFinale(behaviour))
		return;

	if(dsda_UPrepareFinale(behaviour))
		return;

	dsda_LegacyPrepareFinale(behaviour);
}

void dsda_LoadMapInfo()
{
	dsda_AddOriginalEpisodes();

	dsda_HexenLoadMapInfo();
	dsda_ULoadMapInfo();
	dsda_LegacyLoadMapInfo();
}

const char* dsda_ExitPic()
{
	const char* exit_pic;

	if(dsda_HexenExitPic(&exit_pic))
		return exit_pic;

	if(dsda_UExitPic(&exit_pic))
		return exit_pic;

	dsda_LegacyExitPic(&exit_pic);
	return exit_pic;
}

const char* dsda_EnterPic()
{
	const char* enter_pic;

	if(dsda_HexenEnterPic(&enter_pic))
		return enter_pic;

	if(dsda_UEnterPic(&enter_pic))
		return enter_pic;

	dsda_LegacyEnterPic(&enter_pic);
	return enter_pic;
}

const char* dsda_BorderTexture()
{
	const char* border_texture;

	if(dsda_HexenBorderTexture(&border_texture))
		return border_texture;

	if(dsda_UBorderTexture(&border_texture))
		return border_texture;

	dsda_LegacyBorderTexture(&border_texture);
	return border_texture;
}

void dsda_PrepareEntering()
{
	if(dsda_HexenPrepareEntering())
		return;

	if(dsda_UPrepareEntering())
		return;

	dsda_LegacyPrepareEntering();
}

void dsda_PrepareFinished()
{
	if(dsda_HexenPrepareFinished())
		return;

	if(dsda_UPrepareFinished())
		return;

	dsda_LegacyPrepareFinished();
}

int dsda_MapLightning()
{
	int lightning;

	if(dsda_HexenMapLightning(&lightning))
		return lightning;

	if(dsda_UMapLightning(&lightning))
		return lightning;

	dsda_LegacyMapLightning(&lightning);

	return lightning;
}

void dsda_ApplyFadeTable()
{
	if(dsda_HexenApplyFadeTable())
		return;

	if(dsda_UApplyFadeTable())
		return;

	dsda_LegacyApplyFadeTable();
}

int dsda_MapCluster(int map)
{
	int cluster;

	if(dsda_HexenMapCluster(&cluster, map))
		return cluster;

	if(dsda_UMapCluster(&cluster, map))
		return cluster;

	dsda_LegacyMapCluster(&cluster, map);

	return cluster;
}

short dsda_Sky1Texture()
{
	short texture;

	if(dsda_HexenSky1Texture(&texture))
		return texture;

	if(dsda_USky1Texture(&texture))
		return texture;

	dsda_LegacySky1Texture(&texture);

	return texture;
}

short dsda_Sky2Texture()
{
	short texture;

	if(dsda_HexenSky2Texture(&texture))
		return texture;

	if(dsda_USky2Texture(&texture))
		return texture;

	dsda_LegacySky2Texture(&texture);

	return texture;
}

void dsda_InitSky()
{
	if(dsda_HexenInitSky())
		return;

	if(dsda_UInitSky())
		return;

	dsda_LegacyInitSky();
}
