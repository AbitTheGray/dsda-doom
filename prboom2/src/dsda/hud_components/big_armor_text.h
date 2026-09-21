// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Armor Text HUD Component

#ifndef __DSDA_HUD_COMPONENT_BIG_ARMOR_TEXT__
#define __DSDA_HUD_COMPONENT_BIG_ARMOR_TEXT__

void dsda_InitBigArmorTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigArmorTextHC(void* data);
void dsda_DrawBigArmorTextHC(void* data);

#endif
