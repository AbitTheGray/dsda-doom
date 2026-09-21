// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Smooth demo playback
 */

#pragma once

#include "doomtype.h"
#include "tables.h"
#include "d_player.h"

#define SMOOTH_PLAYING_MAXFACTOR 16

extern int demo_smoothturns;
extern int demo_smoothturnsfactor;

void R_SmoothPlaying_Reset(player_t *player);
void R_SmoothPlaying_Add(int delta);
angle_t R_SmoothPlaying_Get(player_t *player);
void R_ResetAfterTeleport(player_t *player);
