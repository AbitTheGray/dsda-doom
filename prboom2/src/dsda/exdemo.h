// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Demo

#pragma once

#include "doomtype.h"

int dsda_IsExDemoSigned(void);
void dsda_MergeExDemoFeatures(void);
void dsda_LoadExDemo(const char* filename);
int dsda_CopyExDemo(const byte** buffer, int* length);
void dsda_WriteExDemoFooter(void);
