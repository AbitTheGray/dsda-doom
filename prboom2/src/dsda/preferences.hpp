// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Preferences

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_LoadWadPreferences();
void dsda_HandleMapPreferences();
void dsda_PreferOpenGL();
void dsda_PreferSoftware();

#ifdef __cplusplus
}
#endif
