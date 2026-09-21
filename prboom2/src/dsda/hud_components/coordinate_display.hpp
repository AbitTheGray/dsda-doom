// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Coordinate Display HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitCoordinateDisplayHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateCoordinateDisplayHC(void* data);
void dsda_DrawCoordinateDisplayHC(void* data);

#ifdef __cplusplus
}
#endif
