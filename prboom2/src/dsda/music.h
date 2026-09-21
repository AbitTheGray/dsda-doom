// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Music

#pragma once

#include "sounds.h"

int dsda_GetDehMusicIndex(const char* key, size_t length);
int dsda_GetOriginalMusicIndex(const char* key);
void dsda_InitializeMusic(musicinfo_t* source, int count);
void dsda_FreeDehMusic(void);

void dsda_ArchiveMusic(void);
void dsda_UnArchiveMusic(void);
dboolean dsda_StartQueuedMusic(void);
