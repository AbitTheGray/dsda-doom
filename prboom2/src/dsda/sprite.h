// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Sprite

#pragma once

int dsda_GetDehSpriteIndex(const char* key);
int dsda_GetOriginalSpriteIndex(const char* key);
void dsda_InitializeSprites(const char** source, int count);
void dsda_FreeDehSprites(void);
