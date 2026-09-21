// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA SFX

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "sounds.hpp"

int dsda_GetDehSFXIndex(const char* key, size_t length);
int dsda_GetOriginalSFXIndex(const char* key);
sfxinfo_t* dsda_GetDehSFX(int index);
sfxinfo_t* dsda_NewSFX(int* index);
void dsda_InitializeSFX(sfxinfo_t* source, int count);
int dsda_TranslateDehSFXIndex(int index);
void dsda_FreeDehSFX();
dboolean dsda_BlockSFX(sfxinfo_t* sfx);

#ifdef __cplusplus
}
#endif
