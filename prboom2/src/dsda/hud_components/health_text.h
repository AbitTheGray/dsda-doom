// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Health Text HUD Component

#ifndef __DSDA_HUD_COMPONENT_HEALTH_TEXT__
#define __DSDA_HUD_COMPONENT_HEALTH_TEXT__

void dsda_InitHealthTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateHealthTextHC(void* data);
void dsda_DrawHealthTextHC(void* data);

#endif
