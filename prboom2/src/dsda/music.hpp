// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Music

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "sounds.hpp"

int dsda_GetDehMusicIndex(const char* key, size_t length);
int dsda_GetOriginalMusicIndex(const char* key);
void dsda_InitializeMusic(musicinfo_t* source, int count);
void dsda_FreeDehMusic();

void dsda_ArchiveMusic();
void dsda_UnArchiveMusic();
dboolean dsda_StartQueuedMusic();

#ifdef __cplusplus
}
#endif
