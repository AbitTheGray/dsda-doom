// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Palette Management

#pragma once

#include "SDL.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct PlaypalIndex : int32_t
{
	Default,
	Pal1,
	Pal2,
	Pal3,
	Pal4,
	Pal5,
	Pal6,
	Pal7,
	Pal8,
	Pal9,
	HereticE2End,
	Custom,
	Count
};

extern PlaypalIndex playpal_index;;

typedef struct playpal_data_s
{
	const PlaypalIndex index;
	const char* lump_name;
	unsigned char* lump;
	int length;
	// See r_patch.c
	int transparent;
	int duplicate;
	int darkest;
	int lightest;

	// Array of SDL_Color structs used for setting the 256-colour palette
	SDL_Color* colours;
} dsda_playpal_t;

double dsda_PaletteEntryLightness(const byte* playpal, int i);
dsda_playpal_t* dsda_PlayPalData(PlaypalIndex playpal_i);
void dsda_CyclePlayPal();
void dsda_SetPlayPal(PlaypalIndex index);
void dsda_FreePlayPal(PlaypalIndex playpal_i);
void dsda_FreeAllPlayPals();
void dsda_InitPlayPal(PlaypalIndex playpal_i);
void dsda_InitAllPlayPals();

#ifdef __cplusplus
}
#endif
