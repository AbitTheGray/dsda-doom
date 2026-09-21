// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Aim

#pragma once

#include "d_player.h"

typedef struct
{
	angle_t angle;
	fixed_t slope;
	fixed_t z_offset;
} aim_t;

angle_t dsda_PlayerPitch(player_t* player);
fixed_t dsda_PlayerSlope(player_t* player);
int dsda_PitchToLookDir(angle_t pitch);
angle_t dsda_LookDirToPitch(int lookdir);
int dsda_PlayerLookDir(player_t* player);
void dsda_PlayerAim(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask);
void dsda_PlayerAimBad(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask);
