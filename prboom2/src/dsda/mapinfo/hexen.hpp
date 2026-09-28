// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//  DSDA MapInfo Hexen

#pragma once

#include "p_mobj.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/utility.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

int dsda_HexenNameToMap(int* found, const char* name, int* episode, int* map);
int dsda_HexenFirstMap(int* episode, int* map);
int dsda_HexenNewGameMap(int* episode, int* map);
int dsda_HexenResolveWarp(int* args, int arg_count, int* episode, int* map);
int dsda_HexenNextMap(int* episode, int* map);
int dsda_HexenPrevMap(int* episode, int* map);
int dsda_HexenShowNextLocBehaviour(ShowNextLocFlag* behaviour);
int dsda_HexenSkipDrawShowNextLoc(int* skip);
void dsda_HexenUpdateMapInfo();
void dsda_HexenUpdateLastMapInfo();
void dsda_HexenUpdateNextMapInfo();
int dsda_HexenResolveCLEV(int* clev, int* episode, int* map);
int dsda_HexenResolveINIT(int* init);
int dsda_HexenMusicIndexToLumpNum(int* lump, int music_index);
int dsda_HexenMapMusic(int* music_index, int* music_lump, int episode, int map);
int dsda_HexenIntermissionMusic(int* music_index, int* music_lump);
int dsda_HexenInterMusic(int* music_index, int* music_lump);
int dsda_HexenStartFinale();
int dsda_HexenFTicker();
void dsda_HexenFDrawer();
int dsda_HexenBossAction(mobj_t* mo);
int dsda_HexenMapLumpName(const char** name, int episode, int map);
int dsda_HexenMapAuthor(const char** author);
int dsda_HexenHUTitle(dsda_string_t* str);
int dsda_HexenSkyTexture(int* sky);
int dsda_HexenPrepareInitNew();
int dsda_HexenPrepareIntermission(int* result);
int dsda_HexenPrepareFinale(int* result);
void dsda_HexenLoadMapInfo();
int dsda_HexenExitPic(const char** exit_pic);
int dsda_HexenEnterPic(const char** enter_pic);
int dsda_HexenBorderTexture(const char** border_texture);
int dsda_HexenPrepareEntering();
int dsda_HexenPrepareFinished();
int dsda_HexenMapLightning(int* lightning);
int dsda_HexenApplyFadeTable();
int dsda_HexenMapCluster(int* cluster, int map);
int dsda_HexenSky1Texture(short* texture);
int dsda_HexenSky2Texture(short* texture);
int dsda_HexenGravity(fixed_t* gravity);
int dsda_HexenAirControl(fixed_t* air_control);
int dsda_HexenInitSky();
int dsda_HexenMapColorMap(int* colormap);

#ifdef __cplusplus
}
#endif
