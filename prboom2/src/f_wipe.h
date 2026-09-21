// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Mission start screen wipe/melt, special effects.
 */

#pragma once

// e6y: resolution limitation is removed
void R_InitMeltRes(void);

/*
 * SCREEN WIPE PACKAGE
 */

int wipe_ScreenWipe (int ticks);
int wipe_StartScreen(void);
int wipe_EndScreen  (void);
