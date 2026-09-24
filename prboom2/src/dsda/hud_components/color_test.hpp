// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Color Test HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitColorTestHC(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateColorTestHC(void* data);
void dsda_DrawColorTestHC(void* data);

#ifdef __cplusplus
}
#endif
