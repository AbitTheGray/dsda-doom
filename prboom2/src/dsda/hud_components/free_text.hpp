// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Free Text HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitFreeTextHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateFreeTextHC(void* data);
void dsda_DrawFreeTextHC(void* data);

#ifdef __cplusplus
}
#endif
