// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Smooth demo playback
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"
#include "tables.hpp"
#include "d_player.hpp"

#define SMOOTH_PLAYING_MAXFACTOR 16

extern int demo_smoothturns;
extern int demo_smoothturnsfactor;

void R_SmoothPlaying_Reset(player_t* player);
void R_SmoothPlaying_Add(int delta);
angle_t R_SmoothPlaying_Get(player_t* player);
void R_ResetAfterTeleport(player_t* player);

#ifdef __cplusplus
}
#endif
