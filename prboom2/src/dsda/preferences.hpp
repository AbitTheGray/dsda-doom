// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Preferences

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

void dsda_LoadWadPreferences();
void dsda_HandleMapPreferences();
void dsda_PreferOpenGL();
void dsda_PreferSoftware();

#ifdef __cplusplus
}
#endif
