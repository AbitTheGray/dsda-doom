// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Mouse

#ifndef __DSDA_MOUSE__
#define __DSDA_MOUSE__

#include "d_ticcmd.h"

void dsda_ApplyQuickstartMouseCache(int* mousex);
void dsda_QueueQuickstart(void);
void dsda_GetMousePosition(int *x, int *y);

#endif
