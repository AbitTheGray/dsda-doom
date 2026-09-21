// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Stat Totals HUD Component

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitStatTotalsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateStatTotalsHC(void* data);
void dsda_DrawStatTotalsHC(void* data);

#ifdef __cplusplus
}
#endif
