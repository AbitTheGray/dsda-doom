// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Armor HUD Component

#ifndef __DSDA_HUD_COMPONENT_BIG_ARMOR__
#define __DSDA_HUD_COMPONENT_BIG_ARMOR__

void dsda_InitBigArmorHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigArmorHC(void* data);
void dsda_DrawBigArmorHC(void* data);

#endif
