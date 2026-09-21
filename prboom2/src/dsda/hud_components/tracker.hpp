// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Tracker HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitTrackerHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateTrackerHC(void* data);
void dsda_DrawTrackerHC(void* data);

#ifdef __cplusplus
}
#endif
