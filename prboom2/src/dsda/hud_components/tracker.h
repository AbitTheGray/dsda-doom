// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Tracker HUD Component

#ifndef __DSDA_HUD_COMPONENT_TRACKER__
#define __DSDA_HUD_COMPONENT_TRACKER__

void dsda_InitTrackerHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateTrackerHC(void* data);
void dsda_DrawTrackerHC(void* data);

#endif
