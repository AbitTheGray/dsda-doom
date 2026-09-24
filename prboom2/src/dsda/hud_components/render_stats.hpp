// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Render Stats HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitRenderStatsHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateRenderStatsHC(void* data);
void dsda_DrawRenderStatsHC(void* data);

#ifdef __cplusplus
}
#endif
