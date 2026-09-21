// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      General system functions. Signal related stuff, exit function
 *      prototypes, and programmable Doom clock.
 */

#pragma once

void I_Init(void);
void I_Init2(void);
dboolean I_Interrupted(void);
NORETURNC11 void I_SafeExit(int rc) NORETURN;
