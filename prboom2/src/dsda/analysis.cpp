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
#include "analysis_category.hpp"
#include "analysis_report.hpp"

int dsda_analysis;

AnalysisRunStats dsda_run_stats;
AnalysisMapTracking dsda_map_tracking;
AnalysisNotes dsda_analysis_notes;

// Set by `dsda_DetectCategory` from the command line every time it runs, so nothing resets them.
static bool dsda_nomo = false;
static bool dsda_respawn = false;
static bool dsda_fast = false;

void dsda_ResetAnalysis()
{
	dsda_run_stats = {};
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
	// `dsda_fast`, and clears `almostReality`, `stroller` and `weaponCollector`
	// in `dsda_run_stats`, all of which are read below.
	const std::string category = to_string(dsda_DetectCategory());

	const Analysis analysis {
		.skill = gameskill + 1,
		.noMonsters = dsda_nomo,
		.respawn = dsda_respawn,
		.fast = dsda_fast,
		.pacifist = dsda_run_stats.pacifist,
		.stroller = dsda_run_stats.stroller,
		.reality = dsda_run_stats.reality,
		.almostReality = dsda_run_stats.almostReality,
		.reborn = dsda_run_stats.reborn,
		.hundredKills = dsda_run_stats.hundredKills,
		.hundredSecrets = dsda_run_stats.hundredSecrets,
		.missedMonsters = dsda_run_stats.missedMonsters,
		.missedSecrets = dsda_run_stats.missedSecrets,
		.weaponCollector = dsda_run_stats.weaponCollector,
		.tysonWeapons = dsda_run_stats.tysonWeapons,
		.turbo = dsda_run_stats.turbo,
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

// `gameskill` values of the two skills with speedrun categories.
// `gameskill` is an index into the skills, which MAPINFO can extend, so these are not an enum.
// They are needed for detecting a category anyway.
constexpr int32_t k_Skill_UltraViolence = 3;
constexpr int32_t k_Skill_Nightmare = 4;

Category dsda_DetectCategory()
{
	dboolean satisfies_max;
	dboolean satisfies_respawn;
	dboolean satisfies_tyson;
	dboolean satisfies_100s;

	if(dsda_run_stats.reality) dsda_run_stats.almostReality = false;
	if(!dsda_run_stats.pacifist) dsda_run_stats.stroller = false;
	if(!dsda_run_stats.anyWeapons) dsda_run_stats.weaponCollector = false;

	dsda_nomo = nomonsters > 0;
	dsda_respawn = respawnparm > 0;
	dsda_fast = fastparm > 0;

	satisfies_max = (
		dsda_run_stats.missedMonsters == 0
		&& dsda_run_stats.hundredSecrets
		&& (dsda_run_stats.anySecrets || dsda_run_stats.anyCountedMonsters)
	);
	satisfies_respawn = (
		dsda_run_stats.hundredSecrets
		&& dsda_run_stats.hundredKills
		&& dsda_run_stats.anyMonsters
		&& (dsda_run_stats.anySecrets || dsda_run_stats.anyCountedMonsters)
	);
	satisfies_tyson = (
		dsda_run_stats.missedMonsters == 0
		&& dsda_run_stats.tysonWeapons
		&& dsda_run_stats.anyCountedMonsters
	);
	satisfies_100s = dsda_run_stats.anySecrets && dsda_run_stats.hundredSecrets;

	if(dsda_ExCmdDemo()) return Category::Other;
	if(dsda_run_stats.turbo) return Category::Other;
	if(coop_spawns) return Category::Other;
	if(solo_net) return Category::Other;
	if(dsda_run_stats.reborn) return Category::Other;

	if(gameskill == k_Skill_UltraViolence)
	{
		if(dsda_nomo && !dsda_respawn && !dsda_fast)
		{
			if(satisfies_100s) return Category::NoMo100S;

			return Category::NoMo;
		}

		if(dsda_respawn && !dsda_nomo && !dsda_fast)
		{
			if(satisfies_respawn) return Category::UvRespawn;

			return Category::Other;
		}

		if(dsda_fast && !dsda_nomo && !dsda_respawn)
		{
			if(satisfies_max) return Category::UvFast;

			return Category::Other;
		}

		if(dsda_nomo || dsda_respawn || dsda_fast) return Category::Other;

		if(satisfies_max) return Category::UvMax;
		if(satisfies_tyson) return Category::UvTyson;
		if(dsda_run_stats.anyMonsters && dsda_run_stats.stroller) return Category::Stroller;
		if(dsda_run_stats.anyMonsters && dsda_run_stats.pacifist) return Category::Pacifist;

		return Category::UvSpeed;
	}
	else if(gameskill == k_Skill_Nightmare)
	{
		if(nomonsters) return Category::Other;
		if(satisfies_100s) return Category::Nm100S;

		return Category::NmSpeed;
	}

	return Category::Other;
}
