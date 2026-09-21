// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Map Time HUD Component

#pragma once

void dsda_InitMapTimeHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateMapTimeHC(void* data);
void dsda_DrawMapTimeHC(void* data);
