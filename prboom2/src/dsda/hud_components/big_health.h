// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Big Health HUD Component

#pragma once

void dsda_InitBigHealthHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateBigHealthHC(void* data);
void dsda_DrawBigHealthHC(void* data);
