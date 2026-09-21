// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Message

#ifndef __DSDA_MESSAGE__
#define __DSDA_MESSAGE__

void dsda_AddPlayerAlert(const char* str, player_t* player);
void dsda_AddAlert(const char* str);
void dsda_AddPlayerMessage(const char* str, player_t* player);
void dsda_AddMessage(const char* str);
void dsda_AddUnblockableMessage(const char* str);
void dsda_UpdateMessenger(void);
void dsda_InitMessenger(void);
void dsda_ReplayMessage(void);

#endif
