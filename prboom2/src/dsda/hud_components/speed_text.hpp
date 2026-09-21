// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Speed Text HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitSpeedTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateSpeedTextHC(void* data);
void dsda_DrawSpeedTextHC(void* data);

#ifdef __cplusplus
}
#endif
