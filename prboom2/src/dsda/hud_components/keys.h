// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Keys HUD Component

#ifndef __DSDA_HUD_COMPONENT_KEYS__
#define __DSDA_HUD_COMPONENT_KEYS__

void dsda_InitKeysHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateKeysHC(void* data);
void dsda_DrawKeysHC(void* data);

#endif
