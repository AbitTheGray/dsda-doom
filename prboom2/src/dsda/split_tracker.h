// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Split Tracker

#pragma once

typedef struct
{
	int current;
	int best;
	int best_delta;
	int session_best;
	int session_best_delta;
	int ref;
	int ref_delta;
} dsda_split_time_t;

typedef struct
{
	dsda_split_time_t leveltime;
	dsda_split_time_t totalleveltimes;
	int episode;
	int map;
	int first_time;
	int run_counter;
	int exits;
} dsda_split_t;

void dsda_RecordSplit(void);
dsda_split_t* dsda_CurrentSplit(void);
void dsda_WriteSplits(void);
void dsda_ResetSplits(void);
int dsda_DemoAttempts(void);
