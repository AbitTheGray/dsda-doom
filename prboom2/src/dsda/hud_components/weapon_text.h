// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Weapon Text HUD Component

#pragma once

void dsda_InitWeaponTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateWeaponTextHC(void* data);
void dsda_DrawWeaponTextHC(void* data);
