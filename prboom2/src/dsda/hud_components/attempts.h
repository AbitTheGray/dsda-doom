// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Attempts HUD Component

#pragma once

void dsda_InitAttemptsHC(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
void dsda_UpdateAttemptsHC(void* data);
void dsda_DrawAttemptsHC(void* data);
