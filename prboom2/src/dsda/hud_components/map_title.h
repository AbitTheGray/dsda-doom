// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Title HUD Component

#ifndef __DSDA_HUD_COMPONENT_MAP_TITLE__
#define __DSDA_HUD_COMPONENT_MAP_TITLE__

void dsda_InitMapTitleHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMapTitleHC(void* data);
void dsda_DrawMapTitleHC(void* data);

#endif
