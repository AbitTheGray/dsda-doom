// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstdint>

// What an ANIMDEFS animation cycles through.
// Part of the savegame (AnimDefs is saved as raw bytes), so it keeps the width of the old int.
enum struct AnimType : int32_t
{
	Flat = 0,
	Texture = 1,
};

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_ANIM_DEFS 20

typedef struct
{
	AnimType type;
	int index;
	int tics;
	int currentFrameDef;
	int startFrameDef;
	int endFrameDef;
} animDef_t;

extern animDef_t AnimDefs[MAX_ANIM_DEFS];

extern int NextLightningFlash;
extern int LightningFlash;

void P_AnimateSurfaces();
void P_ForceLightning();
void P_InitLightning();
void P_InitFTAnims();

#ifdef __cplusplus
}
#endif
