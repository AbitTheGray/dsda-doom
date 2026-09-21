// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA FPS HUD Component

#pragma once

void dsda_InitFPSHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateFPSHC(void* data);
void dsda_DrawFPSHC(void* data);
