// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Health Text HUD Component

#ifndef __DSDA_HUD_COMPONENT_BIG_HEALTH_TEXT__
#define __DSDA_HUD_COMPONENT_BIG_HEALTH_TEXT__

void dsda_InitBigHealthTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigHealthTextHC(void* data);
void dsda_DrawBigHealthTextHC(void* data);

#endif
