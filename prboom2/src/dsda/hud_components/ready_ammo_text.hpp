// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ready Ammo Text HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitReadyAmmoTextHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateReadyAmmoTextHC(void* data);
void dsda_DrawReadyAmmoTextHC(void* data);

#ifdef __cplusplus
}
#endif
