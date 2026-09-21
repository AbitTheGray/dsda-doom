// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mouse

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "d_ticcmd.hpp"

void dsda_ApplyQuickstartMouseCache(int* mousex);
void dsda_QueueQuickstart();
void dsda_GetMousePosition(int* x, int* y);

#ifdef __cplusplus
}
#endif
