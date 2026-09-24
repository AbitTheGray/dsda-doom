// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mouse

#pragma once

#include "d_ticcmd.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_ApplyQuickstartMouseCache(int* mousex);
void dsda_QueueQuickstart();
void dsda_GetMousePosition(int* x, int* y);

#ifdef __cplusplus
}
#endif
