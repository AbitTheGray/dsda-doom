// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Coordinate Display HUD Component

#ifndef __DSDA_HUD_COMPONENT_COORDINATE_DISPLAY__
#define __DSDA_HUD_COMPONENT_COORDINATE_DISPLAY__

void dsda_InitCoordinateDisplayHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateCoordinateDisplayHC(void* data);
void dsda_DrawCoordinateDisplayHC(void* data);

#endif
