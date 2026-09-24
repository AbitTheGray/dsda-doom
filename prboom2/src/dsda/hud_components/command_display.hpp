// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Command Display HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitCommandDisplayHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateCommandDisplayHC(void* data);
void dsda_DrawCommandDisplayHC(void* data);

#ifdef __cplusplus
}
#endif
