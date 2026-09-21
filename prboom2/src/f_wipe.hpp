// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Mission start screen wipe/melt, special effects.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

// e6y: resolution limitation is removed
void R_InitMeltRes();

/*
 * SCREEN WIPE PACKAGE
 */

int wipe_ScreenWipe(int ticks);
int wipe_StartScreen();
int wipe_EndScreen();

#ifdef __cplusplus
}
#endif
