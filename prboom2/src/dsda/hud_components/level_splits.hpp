// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Level Splits HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitLevelSplitsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateLevelSplitsHC(void* data);
void dsda_DrawLevelSplitsHC(void* data);

#ifdef __cplusplus
}
#endif
