// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Totals HUD Component

#pragma once

void dsda_InitMapTotalsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMapTotalsHC(void* data);
void dsda_DrawMapTotalsHC(void* data);
