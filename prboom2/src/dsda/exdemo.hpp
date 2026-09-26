// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Demo

#pragma once

#include "doomtype.hpp"

#include "dsda/exdemo_signature.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

Signature dsda_IsExDemoSigned();
void dsda_MergeExDemoFeatures();
void dsda_LoadExDemo(const char* filename);
int dsda_CopyExDemo(const byte** buffer, int* length);
void dsda_WriteExDemoFooter();

#ifdef __cplusplus
}
#endif
