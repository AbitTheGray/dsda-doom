// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Stat Totals HUD Component

#ifndef __DSDA_HUD_COMPONENT_STAT_TOTALS__
#define __DSDA_HUD_COMPONENT_STAT_TOTALS__

void dsda_InitStatTotalsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateStatTotalsHC(void* data);
void dsda_DrawStatTotalsHC(void* data);

#endif
