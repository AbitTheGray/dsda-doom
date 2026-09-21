// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Parser for the `analysis.txt` report.
//	Mirrors `dsda_WriteAnalysis` in `prboom2/src/dsda/analysis.c` - every key it
//	writes has a field here, in the same order.

#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

struct Analysis
{
	int32_t skill = 0;
	bool noMonsters = false;
	bool respawn = false;
	bool fast = false;
	bool pacifist = false;
	bool stroller = false;
	bool reality = false;
	bool almostReality = false;
	bool reborn = false;
	bool hundredKills = false;
	bool hundredSecrets = false;
	int32_t missedMonsters = 0;
	int32_t missedSecrets = 0;
	bool weaponCollector = false;
	bool tysonWeapons = false;
	bool turbo = false;
	bool soloNet = false;
	bool coopSpawns = false;
	std::string category;
	bool signature = false;

	[[nodiscard]] static std::expected<Analysis, std::string> Parse(std::string_view contents);
	[[nodiscard]] static std::expected<Analysis, std::string> Read(const std::filesystem::path& file);
};
