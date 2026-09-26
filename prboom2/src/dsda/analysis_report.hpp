// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	The `analysis.txt` report - every key `dsda_WriteAnalysis` writes, in the
//	same order. The spec suite parses the file back into this struct.

#pragma once

#include <cstdint>
#include <string>

#include "dsda/exdemo_signature.hpp"

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
	Signature signature = Signature::Unsigned;
};
