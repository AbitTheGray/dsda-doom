// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Secret Message HUD Component

#ifndef __DSDA_HUD_COMPONENT_SECRET_MESSAGE__
#define __DSDA_HUD_COMPONENT_SECRET_MESSAGE__

void dsda_InitSecretMessageHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateSecretMessageHC(void* data);
void dsda_DrawSecretMessageHC(void* data);

#endif
