// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Event Split HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitEventSplitHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateEventSplitHC(void* data);
void dsda_DrawEventSplitHC(void* data);

#ifdef __cplusplus
}
#endif
