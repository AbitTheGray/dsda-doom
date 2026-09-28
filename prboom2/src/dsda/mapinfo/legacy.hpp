// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA MapInfo Legacy

#pragma once

#include "p_mobj.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/utility.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

int dsda_LegacyNameToMap(int* found, const char* name, int* episode, int* map);
int dsda_LegacyFirstMap(int* episode, int* map);
int dsda_LegacyNewGameMap(int* episode, int* map);
int dsda_LegacyResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_LegacyNextMap(int* episode, int* map);
int dsda_LegacyPrevMap(int* episode, int* map);
int dsda_LegacyShowNextLocBehaviour(ShowNextLocFlag* behaviour);
int dsda_LegacySkipDrawShowNextLoc(int* skip);
void dsda_LegacyUpdateMapInfo();
void dsda_LegacyUpdateLastMapInfo();
void dsda_LegacyUpdateNextMapInfo();
int dsda_LegacyResolveCLEV(int* clev, int* episode, int* map);
int dsda_LegacyResolveINIT(int* init);
int dsda_LegacyMusicIndexToLumpNum(int* lump, int music_index);
int dsda_LegacyMapMusic(int* music_index, int* music_lump, int episode, int map);
int dsda_LegacyIntermissionMusic(int* music_index, int* music_lump);
int dsda_LegacyInterMusic(int* music_index, int* music_lump);
int dsda_LegacyStartFinale();
int dsda_LegacyFTicker();
void dsda_LegacyFDrawer();
int dsda_LegacyBossAction(mobj_t* mo);
int dsda_LegacyMapLumpName(const char** name, int episode, int map);
int dsda_LegacyMapAuthor(const char** author);
int dsda_LegacyHUTitle(dsda_string_t* str);
int dsda_LegacySkyTexture(int* sky);
int dsda_LegacyPrepareInitNew();
int dsda_LegacyPrepareIntermission(int* result);
int dsda_LegacyPrepareFinale(int* result);
void dsda_LegacyLoadMapInfo();
int dsda_LegacyExitPic(const char** exit_pic);
int dsda_LegacyEnterPic(const char** enter_pic);
int dsda_LegacyBorderTexture(const char** border_texture);
int dsda_LegacyPrepareEntering();
int dsda_LegacyPrepareFinished();
int dsda_LegacyMapLightning(int* lightning);
int dsda_LegacyApplyFadeTable();
int dsda_LegacyMapCluster(int* cluster, int map);
int dsda_LegacySky1Texture(short* texture);
int dsda_LegacySky2Texture(short* texture);
int dsda_LegacyGravity(fixed_t* gravity);
int dsda_LegacyAirControl(fixed_t* air_control);
int dsda_LegacyInitSky();
int dsda_LegacyMapColorMap(int* colormap);

#ifdef __cplusplus
}
#endif
