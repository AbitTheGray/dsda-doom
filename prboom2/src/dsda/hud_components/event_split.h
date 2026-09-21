// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Event Split HUD Component

#ifndef __DSDA_HUD_COMPONENT_EVENT_SPLIT__
#define __DSDA_HUD_COMPONENT_EVENT_SPLIT__

void dsda_InitEventSplitHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateEventSplitHC(void* data);
void dsda_DrawEventSplitHC(void* data);

#endif
