// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Fake networking stuff.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "d_player.hpp"

// Create any new ticcmds
void FakeNetUpdate();

//? how many ticks to run?
void TryRunTics();

// CPhipps - move to header file
void D_InitFakeNetGame(); // This does the setup

#ifdef __cplusplus
}
#endif
