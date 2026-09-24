// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Armor Text HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitArmorTextHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateArmorTextHC(void* data);
void dsda_DrawArmorTextHC(void* data);

#ifdef __cplusplus
}
#endif
