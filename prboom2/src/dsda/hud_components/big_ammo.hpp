// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Ammo HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitBigAmmoHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigAmmoHC(void* data);
void dsda_DrawBigAmmoHC(void* data);

#ifdef __cplusplus
}
#endif
