// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Local Time HUD Component

#ifndef __DSDA_HUD_COMPONENT_LOCAL_TIME__
#define __DSDA_HUD_COMPONENT_LOCAL_TIME__

void dsda_InitLocalTimeHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateLocalTimeHC(void* data);
void dsda_DrawLocalTimeHC(void* data);

#endif
