// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      General system functions. Signal related stuff, exit function
 *      prototypes, and programmable Doom clock.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void I_Init();
void I_Init2();
dboolean I_Interrupted();
NORETURNC11 void I_SafeExit(int rc) NORETURN;

#ifdef __cplusplus
}
#endif
