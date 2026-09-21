// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Keys HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitKeysHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateKeysHC(void* data);
void dsda_DrawKeysHC(void* data);

#ifdef __cplusplus
}
#endif
