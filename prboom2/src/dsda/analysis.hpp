// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Analysis

#pragma once

#include <cstdint>

#include "doomtype.hpp"

#include "dsda/analysis_category.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

extern int dsda_analysis;

void dsda_ResetAnalysis();
void dsda_WriteAnalysis();
Category dsda_DetectCategory();

#ifdef __cplusplus
}
#endif

/// What the run has done so far, for the category and `analysis.txt`.
/// `dsda_ResetAnalysis` starts it over.
struct AnalysisRunStats
{
	bool pacifist = true;
	bool reality = true;
	bool almostReality = true;
	bool reborn = false;
	int32_t missedMonsters = 0;
	int32_t missedSecrets = 0;
	int32_t missedWeapons = 0;
	bool tysonWeapons = true;
	bool hundredKills = true;
	bool hundredSecrets = true;
	bool anyCountedMonsters = false;
	bool anyMonsters = false;
	bool anySecrets = false;
	bool anyWeapons = false;
	bool stroller = true;
	bool turbo = false;
	bool weaponCollector = true;
};

/// Kills on the current map, for the "100K achieved!" note.
/// Starts over before every level setup.
struct AnalysisMapTracking
{
	int32_t kills = 0;
	bool hundredKills = false;
	bool hundredKillsNoteShown = false;
};

/// Which of the run's notes have been shown.
/// Starts over together with the run stats.
struct AnalysisNotes
{
	bool pacifistShown = false;
	bool realityShown = false;
	bool almostRealityShown = false;
};

extern AnalysisRunStats dsda_run_stats;
extern AnalysisMapTracking dsda_map_tracking;
extern AnalysisNotes dsda_analysis_notes;
