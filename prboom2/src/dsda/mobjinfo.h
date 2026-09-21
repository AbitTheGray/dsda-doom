// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mobj Info

#pragma once

#include "info.h"

#include "dsda/deh_hash.h"

typedef struct
{
	mobjinfo_t* info;
	byte* edited_bits;
} dsda_deh_mobjinfo_t;

int dsda_FindDehMobjIndex(int index);
int dsda_TranslateDehMobjIndex(int index);
int dsda_GetDehMobjIndex(int index);
dsda_deh_mobjinfo_t dsda_GetDehMobjInfo(int index);
void dsda_InitializeMobjInfo(int zero, int max, int count);
void dsda_FreeDehMobjInfo(void);
void dsda_AppendZDoomMobjInfo(void);
