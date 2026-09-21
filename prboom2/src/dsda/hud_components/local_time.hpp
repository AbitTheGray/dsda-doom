// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Local Time HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitLocalTimeHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateLocalTimeHC(void* data);
void dsda_DrawLocalTimeHC(void* data);

#ifdef __cplusplus
}
#endif
