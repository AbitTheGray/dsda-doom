// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Armor Text HUD Component

#ifndef __DSDA_HUD_COMPONENT_ARMOR_TEXT__
#define __DSDA_HUD_COMPONENT_ARMOR_TEXT__

void dsda_InitArmorTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateArmorTextHC(void* data);
void dsda_DrawArmorTextHC(void* data);

#endif
