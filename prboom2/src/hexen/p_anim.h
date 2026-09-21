// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define MAX_ANIM_DEFS 20

typedef struct
{
	int type;
	int index;
	int tics;
	int currentFrameDef;
	int startFrameDef;
	int endFrameDef;
} animDef_t;

extern animDef_t AnimDefs[MAX_ANIM_DEFS];

extern int NextLightningFlash;
extern int LightningFlash;

void P_AnimateSurfaces(void);
void P_ForceLightning(void);
void P_InitLightning(void);
void P_InitFTAnims(void);
