// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Minimap HUD Component

#ifndef __DSDA_HUD_COMPONENT_MINIMAP__
#define __DSDA_HUD_COMPONENT_MINIMAP__

void dsda_InitMinimapHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMinimapHC(void* data);
void dsda_DrawMinimapHC(void* data);

#endif
