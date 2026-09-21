// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ready Ammo Text HUD Component

#ifndef __DSDA_HUD_COMPONENT_READY_AMMO_TEXT__
#define __DSDA_HUD_COMPONENT_READY_AMMO_TEXT__

void dsda_InitReadyAmmoTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateReadyAmmoTextHC(void* data);
void dsda_DrawReadyAmmoTextHC(void* data);

#endif
