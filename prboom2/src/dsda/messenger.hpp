// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Message

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_AddPlayerAlert(const char* str, player_t* player);
void dsda_AddAlert(const char* str);
void dsda_AddPlayerMessage(const char* str, player_t* player);
void dsda_AddMessage(const char* str);
void dsda_AddUnblockableMessage(const char* str);
void dsda_UpdateMessenger();
void dsda_InitMessenger();
void dsda_ReplayMessage();

#ifdef __cplusplus
}
#endif
