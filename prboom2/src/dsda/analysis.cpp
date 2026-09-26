// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Analysis

#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

#include "doomstat.hpp"

#include "dsda/excmd.hpp"
#include "dsda/exdemo.hpp"
#include "dsda/settings.hpp"

#include "analysis.hpp"
#include "analysis_report.hpp"

int dsda_analysis;

dboolean dsda_pacifist = true;
dboolean dsda_reality = true;
dboolean dsda_almost_reality = true;
dboolean dsda_reborn = false;
int dsda_missed_monsters = 0;
int dsda_missed_secrets = 0;
int dsda_missed_weapons = 0;
dboolean dsda_tyson_weapons = true;
dboolean dsda_100k = true;
dboolean dsda_100s = true;
dboolean dsda_any_counted_monsters = false;
dboolean dsda_any_monsters = false;
dboolean dsda_any_secrets = false;
dboolean dsda_any_weapons = false;
dboolean dsda_stroller = true;
dboolean dsda_nomo = false;
dboolean dsda_respawn = false;
dboolean dsda_fast = false;
dboolean dsda_turbo = false;
dboolean dsda_weapon_collector = true;

int dsda_kills_on_map = 0;
dboolean dsda_100k_on_map = false;
dboolean dsda_100k_note_shown = false;
dboolean dsda_pacifist_note_shown = false;
dboolean dsda_reality_note_shown = false;
dboolean dsda_almost_reality_note_shown = false;

void dsda_ResetAnalysis()
{
	dsda_pacifist = true;
	dsda_reality = true;
	dsda_almost_reality = true;
	dsda_reborn = false;
	dsda_missed_monsters = 0;
	dsda_missed_secrets = 0;
	dsda_missed_weapons = 0;
	dsda_tyson_weapons = true;
	dsda_100k = true;
	dsda_100s = true;
	dsda_any_counted_monsters = false;
	dsda_any_monsters = false;
	dsda_any_secrets = false;
	dsda_any_weapons = false;
	dsda_stroller = true;
	dsda_turbo = false;
	dsda_weapon_collector = true;
}

void dsda_WriteAnalysis()
{
	if(!dsda_analysis) return;

	// Text mode, as upstream's "w": `\n` stays `\n`, except on Windows where it becomes `\r\n`.
	const std::filesystem::path path = u8"analysis.txt";
	std::ofstream file(path);

	if(!file)
	{
		std::println(std::cerr, "Unable to open analysis.txt for writing!");
		return;
	}

	// Detect the category first - it sets `dsda_nomo`, `dsda_respawn` and
	// `dsda_fast`, and clears `dsda_almost_reality`, `dsda_stroller` and
	// `dsda_weapon_collector`, all of which are read below.
	const std::string category = dsda_DetectCategory();

	const Analysis analysis {
		.skill = gameskill + 1,
		.noMonsters = dsda_nomo != 0,
		.respawn = dsda_respawn != 0,
		.fast = dsda_fast != 0,
		.pacifist = dsda_pacifist != 0,
		.stroller = dsda_stroller != 0,
		.reality = dsda_reality != 0,
		.almostReality = dsda_almost_reality != 0,
		.reborn = dsda_reborn != 0,
		.hundredKills = dsda_100k != 0,
		.hundredSecrets = dsda_100s != 0,
		.missedMonsters = dsda_missed_monsters,
		.missedSecrets = dsda_missed_secrets,
		.weaponCollector = dsda_weapon_collector != 0,
		.tysonWeapons = dsda_tyson_weapons != 0,
		.turbo = dsda_turbo != 0,
		.soloNet = solo_net != 0,
		.coopSpawns = coop_spawns != 0,
		.category = category,
		.signature = dsda_IsExDemoSigned(),
	};

	// `{:d}` prints a `bool` as 0 or 1, as upstream's `%d` did, not as `true`/`false`.
	std::println(file, "skill {}", analysis.skill);
	std::println(file, "nomonsters {:d}", analysis.noMonsters);
	std::println(file, "respawn {:d}", analysis.respawn);
	std::println(file, "fast {:d}", analysis.fast);
	std::println(file, "pacifist {:d}", analysis.pacifist);
	std::println(file, "stroller {:d}", analysis.stroller);
	std::println(file, "reality {:d}", analysis.reality);
	std::println(file, "almost_reality {:d}", analysis.almostReality);
	std::println(file, "reborn {:d}", analysis.reborn);
	std::println(file, "100k {:d}", analysis.hundredKills);
	std::println(file, "100s {:d}", analysis.hundredSecrets);
	std::println(file, "missed_monsters {}", analysis.missedMonsters);
	std::println(file, "missed_secrets {}", analysis.missedSecrets);
	std::println(file, "weapon_collector {:d}", analysis.weaponCollector);
	std::println(file, "tyson_weapons {:d}", analysis.tysonWeapons);
	std::println(file, "turbo {:d}", analysis.turbo);
	std::println(file, "solo_net {:d}", analysis.soloNet);
	std::println(file, "coop_spawns {:d}", analysis.coopSpawns);
	std::println(file, "category {}", analysis.category);
	// `int8_t` is a `signed char`, which `std::format` prints as a number, like `%d`.
	std::println(file, "signature {}", std::to_underlying(analysis.signature));
}

#define SKILL4 3
#define SKILL5 4

const char* dsda_DetectCategory()
{
	dboolean satisfies_max;
	dboolean satisfies_respawn;
	dboolean satisfies_tyson;
	dboolean satisfies_100s;

	if(dsda_reality) dsda_almost_reality = false;
	if(!dsda_pacifist) dsda_stroller = false;
	if(!dsda_any_weapons) dsda_weapon_collector = false;

	dsda_nomo = nomonsters > 0;
	dsda_respawn = respawnparm > 0;
	dsda_fast = fastparm > 0;

	satisfies_max = (
		dsda_missed_monsters == 0
		&& dsda_100s
		&& (dsda_any_secrets || dsda_any_counted_monsters)
	);
	satisfies_respawn = (
		dsda_100s
		&& dsda_100k
		&& dsda_any_monsters
		&& (dsda_any_secrets || dsda_any_counted_monsters)
	);
	satisfies_tyson = (
		dsda_missed_monsters == 0
		&& dsda_tyson_weapons
		&& dsda_any_counted_monsters
	);
	satisfies_100s = dsda_any_secrets && dsda_100s;

	if(dsda_ExCmdDemo()) return "Other";
	if(dsda_turbo) return "Other";
	if(coop_spawns) return "Other";
	if(solo_net) return "Other";
	if(dsda_reborn) return "Other";

	if(gameskill == SKILL4)
	{
		if(dsda_nomo && !dsda_respawn && !dsda_fast)
		{
			if(satisfies_100s) return "NoMo 100S";

			return "NoMo";
		}

		if(dsda_respawn && !dsda_nomo && !dsda_fast)
		{
			if(satisfies_respawn) return "UV Respawn";

			return "Other";
		}

		if(dsda_fast && !dsda_nomo && !dsda_respawn)
		{
			if(satisfies_max) return "UV Fast";

			return "Other";
		}

		if(dsda_nomo || dsda_respawn || dsda_fast) return "Other";

		if(satisfies_max) return "UV Max";
		if(satisfies_tyson) return "UV Tyson";
		if(dsda_any_monsters && dsda_stroller) return "Stroller";
		if(dsda_any_monsters && dsda_pacifist) return "Pacifist";

		return "UV Speed";
	}
	else if(gameskill == SKILL5)
	{
		if(nomonsters) return "Other";
		if(satisfies_100s) return "NM 100S";

		return "NM Speed";
	}

	return "Other";
}
