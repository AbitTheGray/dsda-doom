// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Message HUD Component

#ifndef __DSDA_HUD_COMPONENT_MESSAGE__
#define __DSDA_HUD_COMPONENT_MESSAGE__

void dsda_InitMessageHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMessageHC(void* data);
void dsda_DrawMessageHC(void* data);

#endif
