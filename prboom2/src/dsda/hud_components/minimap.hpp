// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Minimap HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitMinimapHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMinimapHC(void* data);
void dsda_DrawMinimapHC(void* data);

#ifdef __cplusplus
}
#endif
