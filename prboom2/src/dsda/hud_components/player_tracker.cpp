// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Player Tracker HUD Component

#include "base.hpp"

#include "player_tracker.hpp"

void dsda_PlayerTrackerHC(char* str, size_t max_size)
{
	extern int player_damage_last_tic;

	snprintf(
		str,
		max_size,
		"%sp: %d",
		player_damage_last_tic > 0
		? dsda_TextColor(dsda_tc_exhud_player_damage)
		: dsda_TextColor(dsda_tc_exhud_player_neutral),
		player_damage_last_tic
	);
}
