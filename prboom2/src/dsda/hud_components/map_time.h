// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Time HUD Component

#ifndef __DSDA_HUD_COMPONENT_MAP_TIME__
#define __DSDA_HUD_COMPONENT_MAP_TIME__

void dsda_InitMapTimeHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMapTimeHC(void* data);
void dsda_DrawMapTimeHC(void* data);

#endif
