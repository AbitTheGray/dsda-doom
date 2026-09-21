// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Palette Management

#pragma once

#include "SDL.h"

typedef enum {
  playpal_default,
  playpal_1,
  playpal_2,
  playpal_3,
  playpal_4,
  playpal_5,
  playpal_6,
  playpal_7,
  playpal_8,
  playpal_9,
  playpal_heretic_e2end,
  playpal_custom,
  NUMPALETTES
} dsda_playpal_index_t;

extern int playpal_index;;

typedef struct playpal_data_s {
  const int index;
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

double dsda_PaletteEntryLightness(const byte *playpal, int i);
dsda_playpal_t* dsda_PlayPalData(int playpal_i);
void dsda_CyclePlayPal(void);
void dsda_SetPlayPal(int index);
void dsda_FreePlayPal(int playpal_i);
void dsda_FreeAllPlayPals(void);
void dsda_InitPlayPal(int playpal_i);
void dsda_InitAllPlayPals(void);
