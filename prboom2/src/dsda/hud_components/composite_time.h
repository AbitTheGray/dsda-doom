// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Composite Time HUD Component

#ifndef __DSDA_HUD_COMPONENT_COMPOSITE_TIME__
#define __DSDA_HUD_COMPONENT_COMPOSITE_TIME__

void dsda_InitCompositeTimeHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateCompositeTimeHC(void* data);
void dsda_DrawCompositeTimeHC(void* data);

#endif
