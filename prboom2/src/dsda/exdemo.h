// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Demo

#ifndef __DSDA_EXDEMO__
#define __DSDA_EXDEMO__

#include "doomtype.h"

int dsda_IsExDemoSigned(void);
void dsda_MergeExDemoFeatures(void);
void dsda_LoadExDemo(const char* filename);
int dsda_CopyExDemo(const byte** buffer, int* length);
void dsda_WriteExDemoFooter(void);

#endif
