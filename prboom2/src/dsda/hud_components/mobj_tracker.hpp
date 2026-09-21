// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mobj Tracker HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "p_mobj.hpp"

void dsda_MobjTrackerHC(char* str, size_t max_size, int id, mobj_t* mobj);

#ifdef __cplusplus
}
#endif
