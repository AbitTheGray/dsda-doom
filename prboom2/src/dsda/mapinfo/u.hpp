// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA MapInfo U

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "p_mobj.hpp"

#include "dsda/mapinfo.hpp"
#include "dsda/utility.hpp"

int dsda_UNameToMap(int* found, const char* name, int* episode, int* map);
int dsda_UFirstMap(int* episode, int* map);
int dsda_UNewGameMap(int* episode, int* map);
int dsda_UResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_UNextMap(int* episode, int* map);
int dsda_UPrevMap(int* episode, int* map);
int dsda_UShowNextLocBehaviour(int* behaviour);
int dsda_USkipDrawShowNextLoc(int* skip);
void dsda_UUpdateMapInfo();
void dsda_UUpdateLastMapInfo();
void dsda_UUpdateNextMapInfo();
int dsda_UResolveCLEV(int* clev, int* episode, int* map);
int dsda_UResolveINIT(int* init);
int dsda_UMusicIndexToLumpNum(int* lump, int music_index);
int dsda_UMapMusic(int* music_index, int* music_lump, int episode, int map);
int dsda_UIntermissionMusic(int* music_index, int* music_lump);
int dsda_UInterMusic(int* music_index, int* music_lump);
int dsda_UStartFinale();
int dsda_UFTicker();
void dsda_UFDrawer();
int dsda_UBossAction(mobj_t* mo);
int dsda_UMapLumpName(const char** name, int episode, int map);
int dsda_UMapAuthor(const char** author);
int dsda_UHUTitle(dsda_string_t* str);
int dsda_USkyTexture(int* sky);
int dsda_UPrepareInitNew();
int dsda_UPrepareIntermission(int* result);
int dsda_UPrepareFinale(int* result);
void dsda_ULoadMapInfo();
int dsda_UExitPic(const char** exit_pic);
int dsda_UEnterPic(const char** enter_pic);
int dsda_UBorderTexture(const char** border_texture);
int dsda_UPrepareEntering();
int dsda_UPrepareFinished();
int dsda_UMapLightning(int* lightning);
int dsda_UApplyFadeTable();
int dsda_UMapCluster(int* cluster, int map);
int dsda_USky1Texture(short* texture);
int dsda_USky2Texture(short* texture);
int dsda_UGravity(fixed_t* gravity);
int dsda_UAirControl(fixed_t* air_control);
int dsda_UInitSky();
int dsda_UMapColorMap(int* colormap);

#ifdef __cplusplus
}
#endif
