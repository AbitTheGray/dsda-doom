// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Coordinates HUD Component

#ifndef __DSDA_HUD_COMPONENT_MAP_COORDINATES__
#define __DSDA_HUD_COMPONENT_MAP_COORDINATES__

void dsda_InitMapCoordinatesHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMapCoordinatesHC(void* data);
void dsda_DrawMapCoordinatesHC(void* data);

#endif
