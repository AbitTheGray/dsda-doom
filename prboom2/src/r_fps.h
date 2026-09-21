// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Uncapped framerate stuff
 */

#pragma once

#include "doomstat.h"

extern int movement_smooth;
extern dboolean isExtraDDisplay;

typedef struct {
  fixed_t frac;
} tic_vars_t;

extern tic_vars_t tic_vars;

void M_ChangeUncappedFrameRate(void);

dboolean R_ViewInterpolation(void);
void R_InterpolateView(player_t *player, fixed_t frac);

extern dboolean WasRenderedInTryRunTics;

void R_ResetViewInterpolation ();
void R_UpdateInterpolations();
void R_StopAllInterpolations(void);
void R_RestoreInterpolations();
void R_ActivateSectorInterpolations();
void R_ActivateThinkerInterpolations(thinker_t *th);
void R_StopInterpolationIfNeeded(thinker_t *th);
