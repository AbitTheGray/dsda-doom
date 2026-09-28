// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Split Tracker

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "m_file.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"
#include "z_zone.hpp"

#include "dsda/args.hpp"
#include "dsda/demo.hpp"
#include "dsda/settings.hpp"
#include "dsda/data_organizer.hpp"

#include "split_tracker.hpp"

#define SPLIT_VERSION 1

static dsda_split_t* dsda_splits;
static size_t dsda_splits_count;
static int attempts;
static int current_split;
static char* dsda_split_tracker_dir;
static char* dsda_split_tracker_path;

extern int gameskill, gamemap, gameepisode, leveltime, totalleveltimes;
extern int respawnparm, fastparm, nomonsters;

static char* dsda_SplitTrackerDir()
{
	if(!dsda_split_tracker_dir)
		dsda_split_tracker_dir = dsda_DataDir();

	return dsda_split_tracker_dir;
}

const char* dsda_SplitFileBase()
{
	dsda_arg_t* arg;

	arg = dsda_Arg(ArgId::TrackPlayback);
	if(arg->found)
		return arg->value.v_string;

	return dsda_DemoNameBase();
}

static char* dsda_SplitTrackerPath()
{
	if(!dsda_split_tracker_path)
	{
		int length;
		const char* name_base;
		char* dir;
		char params[4];

		name_base = dsda_SplitFileBase();
		if(!name_base)
			return nullptr;

		name_base = PathFindFileName(name_base);

		params[0] = respawnparm ? 'r' : 'x';
		params[1] = fastparm ? 'f' : 'x';
		params[2] = nomonsters ? 'o' : 'x';
		params[3] = '\0';

		dir = dsda_SplitTrackerDir();

		length = strlen(dir) + strlen(name_base) + 28;
		dsda_split_tracker_path = static_cast<char*>(Z_Malloc(length));

		snprintf(
			dsda_split_tracker_path, length, "%s/%s_%i_%i_%i_%s_splits.txt",
			dir, name_base, gameskill + 1, gameepisode, gamemap, params
		);
	}

	return dsda_split_tracker_path;
}

static void dsda_InitSplitTime(dsda_split_time_t* split_time)
{
	split_time->current = 0;
	split_time->best = -1;
	split_time->best_delta = 0;
	split_time->session_best = -1;
	split_time->session_best_delta = 0;
	split_time->ref = -1;
	split_time->ref_delta = 0;
}

static void dsda_LoadSplits()
{
	char* path;
	char* buffer;
	int version;
	static int loaded = false;

	if(loaded)
		return;

	loaded = true;
	path = dsda_SplitTrackerPath();

	if(!path)
		return;

	if(M_ReadFileToString(path, &buffer) != -1)
	{
		int episode, map, tics, total_tics, exits, count, i, ref_tics, ref_total_tics;
		char* line;

		line = strtok(buffer, "\n");

		if(line)
		{
			count = sscanf(line, "%d %d", &attempts, &version);

			if(count < 1)
				attempts = 0;

			if(count < 2)
				version = 0;

			line = strtok(nullptr, "\n");
		}

		while(line)
		{
			ref_tics = ref_total_tics = 0;
			count = sscanf(
				line, "%i %i %i %i %i %i %i",
				&episode, &map, &tics, &total_tics, &exits,
				&ref_tics, &ref_total_tics
			);
			if(count < 5)
				break;

			i = dsda_splits_count;
			dsda_splits = static_cast<dsda_split_t*>(Z_Realloc(dsda_splits, (++dsda_splits_count) * sizeof(dsda_split_t)));
			dsda_InitSplitTime(&dsda_splits[i].leveltime);
			dsda_InitSplitTime(&dsda_splits[i].totalleveltimes);
			dsda_splits[i].first_time = 0;
			dsda_splits[i].episode = episode;
			dsda_splits[i].map = map;
			dsda_splits[i].leveltime.best = tics;
			dsda_splits[i].totalleveltimes.best = total_tics;
			dsda_splits[i].leveltime.ref = ref_tics;
			dsda_splits[i].totalleveltimes.ref = ref_total_tics;
			dsda_splits[i].exits = exits;
			dsda_splits[i].run_counter = 0;

			// in version 0, a time of 0 was considered unset
			if(version == 0)
			{
				if(!dsda_splits[i].leveltime.best)
					dsda_splits[i].leveltime.best = -1;

				if(!dsda_splits[i].totalleveltimes.best)
					dsda_splits[i].totalleveltimes.best = -1;

				if(!dsda_splits[i].leveltime.ref)
					dsda_splits[i].leveltime.ref = -1;

				if(!dsda_splits[i].totalleveltimes.ref)
					dsda_splits[i].totalleveltimes.ref = -1;
			}

			line = strtok(nullptr, "\n");
		}

		Z_Free(buffer);
	}
}

void dsda_WriteSplits()
{
	char* path;
	char* buffer = static_cast<char*>(Z_Malloc(22 + 72 * dsda_splits_count));
	char* p = buffer;
	int i;

	if(!attempts)
		return;
	path = dsda_SplitTrackerPath();

	p += snprintf(p, 22, "%d %d\n", attempts, SPLIT_VERSION);

	for(i = 0; i < dsda_splits_count; ++i)
	{
		p += snprintf(
			p, 72,
			"%i %i %i %i %i %i %i\n",
			dsda_splits[i].episode,
			dsda_splits[i].map,
			dsda_splits[i].leveltime.best,
			dsda_splits[i].totalleveltimes.best,
			dsda_splits[i].exits,
			dsda_splits[i].leveltime.ref,
			dsda_splits[i].totalleveltimes.ref
		);
	}

	if(!M_WriteFile(path, buffer, p - buffer))
		Log::Alert("dsda_WriteSplits: Failed to write splits file \"{}\". ({})", path, errno);

	Z_Free(buffer);
}

static void dsda_UpdateReferenceSplits()
{
	int i;

	for(i = 0; i < dsda_splits_count; ++i)
	{
		dsda_splits[i].leveltime.ref = dsda_splits[i].leveltime.current;
		dsda_splits[i].totalleveltimes.ref = dsda_splits[i].totalleveltimes.current;
	}
}

static int dsda_PersonalBest()
{
	dsda_split_time_t* split_single;
	dsda_split_time_t* split_total;

	split_single = &dsda_splits[dsda_splits_count - 1].leveltime;
	split_total = &dsda_splits[dsda_splits_count - 1].totalleveltimes;

	return split_total->ref_delta < 0 || split_total->ref == -1 ||
		(dsda_splits_count == 1 && split_single->ref_delta < 0);
}

static void dsda_TrackSplitTime(dsda_split_time_t* split_time, int current)
{
	split_time->current = current;
	split_time->best_delta = current - split_time->best;
	split_time->session_best_delta = current - split_time->session_best;
	split_time->ref_delta = current - split_time->ref;

	if(current < split_time->best || split_time->best == -1)
		split_time->best = current;

	if(current < split_time->session_best || split_time->session_best == -1)
		split_time->session_best = current;
}

void dsda_RecordSplit()
{
	int i;

	if(!dsda_TrackSplits()) return;

	dsda_LoadSplits();

	for(i = 0; i < dsda_splits_count; ++i)
		if(dsda_splits[i].episode == gameepisode && dsda_splits[i].map == gamemap)
		{
			dsda_splits[i].first_time = 0;
			break;
		}

	if(i == dsda_splits_count)
	{
		dsda_splits = static_cast<dsda_split_t*>(Z_Realloc(dsda_splits, (++dsda_splits_count) * sizeof(dsda_split_t)));
		dsda_splits[i].first_time = 1;
		dsda_InitSplitTime(&dsda_splits[i].leveltime);
		dsda_InitSplitTime(&dsda_splits[i].totalleveltimes);
		dsda_splits[i].episode = gameepisode;
		dsda_splits[i].map = gamemap;
		dsda_splits[i].exits = 0;
	}

	current_split = i;
	dsda_splits[i].run_counter = attempts;
	dsda_splits[i].exits++;
	dsda_TrackSplitTime(&dsda_splits[i].leveltime, leveltime);
	dsda_TrackSplitTime(&dsda_splits[i].totalleveltimes, totalleveltimes);

	if(i == dsda_splits_count - 1 && dsda_PersonalBest())
		dsda_UpdateReferenceSplits();
}

dsda_split_t* dsda_CurrentSplit()
{
	if(!dsda_ShowSplitData()) return nullptr;

	return &dsda_splits[current_split];
}

void dsda_ResetSplits()
{
	if(!dsda_TrackSplits()) return;

	dsda_LoadSplits();
	++attempts;
}

int dsda_DemoAttempts()
{
	return attempts;
}
