// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ammo Text HUD Component

#ifndef __DSDA_HUD_COMPONENT_AMMO_TEXT__
#define __DSDA_HUD_COMPONENT_AMMO_TEXT__

void dsda_InitAmmoTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateAmmoTextHC(void* data);
void dsda_DrawAmmoTextHC(void* data);

#endif
