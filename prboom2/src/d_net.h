// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Fake networking stuff.
 */

#pragma once

#include "d_player.h"

// Create any new ticcmds
void FakeNetUpdate(void);

//? how many ticks to run?
void TryRunTics(void);

// CPhipps - move to header file
void D_InitFakeNetGame(void); // This does the setup
