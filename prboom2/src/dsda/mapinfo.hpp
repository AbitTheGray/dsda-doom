// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA MapInfo

#pragma once

#include <string_view>

#include "p_mobj.hpp"
#include "dsda/utility.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define WI_SHOW_NEXT_LOC      0x01
#define WI_SHOW_NEXT_DONE     0x02
#define WI_SHOW_NEXT_EPISODAL 0x04

#define DC_VICTORY 0x01

#define WD_VICTORY      0x01
#define WD_START_FINALE 0x02

void dsda_FirstMap(int* episode, int* map);
void dsda_NewGameMap(int* episode, int* map);
void dsda_ResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_NameToMap(const char* name, int* episode, int* map);
void dsda_NextMap(int* episode, int* map);
void dsda_PrevMap(int* episode, int* map);
void dsda_ShowNextLocBehaviour(int* behaviour);
int dsda_SkipDrawShowNextLoc();
void dsda_UpdateGameMap(int episode, int map);
void dsda_ResetLeaveData();
void dsda_UpdateLeaveData(int map, int position, int flags, angle_t angle);
dboolean dsda_FinaleShortcut();
void dsda_UpdateLastMapInfo();
void dsda_UpdateNextMapInfo();
int dsda_ResolveCLEV(int* episode, int* map);
int dsda_ResolveINIT();
int dsda_MusicIndexToLumpNum(int music_index);
void dsda_MapMusic(int* music_index, int* music_lump, int episode, int map);
void dsda_IntermissionMusic(int* music_index, int* music_lump);
void dsda_InterMusic(int* music_index, int* music_lump);
void dsda_StartFinale();
int dsda_FTicker();
int dsda_FDrawer();
int dsda_BossAction(mobj_t* mo);
const char* dsda_MapAuthor();
void dsda_HUTitle(dsda_string_t* str);
int dsda_SkyTexture();
void dsda_PrepareInitNew();
void dsda_PrepareIntermission(int* behaviour);
void dsda_PrepareFinale(int* behaviour);
void dsda_LoadMapInfo();
const char* dsda_ExitPic();
const char* dsda_EnterPic();
const char* dsda_BorderTexture();
void dsda_PrepareEntering();
void dsda_PrepareFinished();
int dsda_MapLightning();
void dsda_ApplyFadeTable();
int dsda_MapCluster(int map);
short dsda_Sky1Texture();
short dsda_Sky2Texture();
void dsda_InitSky();

#ifdef __cplusplus
}
#endif

/// The name of the map's lump, e.g. "MAP01" or "E1M1".
/// The view points into a buffer that the next call overwrites, so it is valid only until then.
std::string_view dsda_MapLumpName(int episode, int map);
