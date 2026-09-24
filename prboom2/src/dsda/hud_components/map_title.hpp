// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Title HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitMapTitleHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMapTitleHC(void* data);
void dsda_DrawMapTitleHC(void* data);

#ifdef __cplusplus
}
#endif
