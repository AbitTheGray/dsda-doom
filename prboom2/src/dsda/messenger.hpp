// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Message

#pragma once

#include <format>
#include <utility>

#include "d_player.hpp"

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

/// On-screen messages formatted with `std::format`, e.g. `Message::Add("Game Speed {}", value)`.
namespace Message
{
	/// Show a message, as `dsda_AddMessage` does; the text is not cut to any length.
	template<typename... Args>
	void Add(const std::format_string<Args...> format, Args&&... args)
	{
		dsda_AddMessage(std::format(format, std::forward<Args>(args)...).c_str());
	}
}
