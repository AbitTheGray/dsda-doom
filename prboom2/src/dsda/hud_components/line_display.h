// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Line Display HUD Component

#ifndef __DSDA_HUD_COMPONENT_LINE_DISPLAY__
#define __DSDA_HUD_COMPONENT_LINE_DISPLAY__

void dsda_InitLineDisplayHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateLineDisplayHC(void* data);
void dsda_DrawLineDisplayHC(void* data);

#endif
