// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Tracker

#include "doomstat.hpp"
#include "p_tick.hpp"
#include "r_state.hpp"

#include "dsda/args.hpp"
#include "dsda/features.hpp"
#include "dsda/settings.hpp"
#include "dsda/tracker.hpp"

#include "tracker.hpp"

dsda_tracker_t dsda_tracker[TRACKER_LIMIT];

static int tracker_map;
static int tracker_episode;

static int dsda_FindTracker(TrackerType type, int id)
{
	int i;

	for(i = 0; i < TRACKER_LIMIT; ++i)
		if(dsda_tracker[i].type == type && dsda_tracker[i].id == id)
			return i;

	return -1;
}

mobj_t* dsda_FindMobj(int id)
{
	thinker_t* th;
	mobj_t* mobj;

	for(th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if(th->function != reinterpret_cast<think_t>(P_MobjThinker))
			continue;

		mobj = (mobj_t*)th;

		if(mobj->index == id)
			return mobj;
	}

	return nullptr;
}

static void dsda_WipeTracker(int i)
{
	dsda_tracker[i].type = TrackerType::Nothing;
	dsda_tracker[i].id = 0;
	dsda_tracker[i].mobj = nullptr;
}

void dsda_WipeTrackers()
{
	int i;

	for(i = 0; i < TRACKER_LIMIT; ++i)
		if(dsda_tracker[i].type != TrackerType::Player)
			dsda_WipeTracker(i);
}

static void dsda_ConsolidateTrackers()
{
	int i;

	for(i = 0; i < TRACKER_LIMIT; ++i)
		if(dsda_tracker[i].type == TrackerType::Nothing)
		{
			int j;

			for(j = i + 1; j < TRACKER_LIMIT; ++j)
				if(dsda_tracker[j].type != TrackerType::Nothing)
				{
					dsda_tracker[i] = dsda_tracker[j];
					dsda_WipeTracker(j);
					break;
				}

			if(j == TRACKER_LIMIT)
				return;
		}
}

static void dsda_RefreshTrackers()
{
	int i;

	for(i = 0; i < TRACKER_LIMIT; ++i)
	{
		switch(dsda_tracker[i].type)
		{
			default:
				break;
			case TrackerType::Mobj:
				dsda_tracker[i].mobj = dsda_FindMobj(dsda_tracker[i].id);
				if(!dsda_tracker[i].mobj)
					dsda_WipeTracker(i);
				break;
		}

		if(dsda_tracker[i].type != TrackerType::Nothing)
			dsda_TrackFeature(FeatureFlag::Tracker);
	}
}

static void dsda_ParseCommandlineTrackers(ArgId arg_id, dboolean (*track)(int))
{
	dsda_arg_t* arg;

	arg = dsda_Arg(static_cast<ArgId>(arg_id));
	if(arg->found)
	{
		int i;

		for(i = 0; i < arg->count; ++i)
			track(arg->value.v_int_array[i]);
	}
}

void dsda_ResetTrackers()
{
	static dboolean first_time = true;

	if(first_time)
	{
		first_time = false;

		dsda_ParseCommandlineTrackers(ArgId::TrackLine, dsda_TrackLine);
		dsda_ParseCommandlineTrackers(ArgId::TrackLineDistance, dsda_TrackLineDistance);
		dsda_ParseCommandlineTrackers(ArgId::TrackSector, dsda_TrackSector);
		dsda_ParseCommandlineTrackers(ArgId::TrackMobj, dsda_TrackMobj);

		if(dsda_Flag(ArgId::TrackPlayer))
			dsda_TrackPlayer(0);

		return;
	}

	if(gamemap != tracker_map || gameepisode != tracker_episode)
		dsda_WipeTrackers();
	else
		dsda_RefreshTrackers();

	dsda_ConsolidateTrackers();
}

static dboolean dsda_AddTracker(TrackerType type, int id, mobj_t* mobj)
{
	int i;

	tracker_map = gamemap;
	tracker_episode = gameepisode;

	if(dsda_FindTracker(type, id) >= 0)
		return false;

	if((i = dsda_FindTracker(TrackerType::Nothing, 0)) >= 0)
	{
		dsda_TrackFeature(FeatureFlag::Tracker);

		dsda_tracker[i].type = (TrackerType)type;
		dsda_tracker[i].id = id;
		dsda_tracker[i].mobj = mobj;

		return true;
	}

	return false;
}

static dboolean dsda_RemoveTracker(TrackerType type, int id)
{
	int i;

	if(dsda_StrictMode())
		return false;

	if((i = dsda_FindTracker(type, id)) >= 0)
	{
		dsda_WipeTracker(i);
		dsda_ConsolidateTrackers();

		return true;
	}

	return false;
}

dboolean dsda_TrackLine(int id)
{
	if(dsda_StrictMode())
		return false;

	if(id >= numlines || id < 0)
		return false;

	return dsda_AddTracker(TrackerType::Line, id, nullptr);
}

dboolean dsda_UntrackLine(int id)
{
	return dsda_RemoveTracker(TrackerType::Line, id);
}

dboolean dsda_TrackLineDistance(int id)
{
	if(dsda_StrictMode())
		return false;

	if(id >= numlines || id < 0)
		return false;

	return dsda_AddTracker(TrackerType::LineDistance, id, nullptr);
}

dboolean dsda_UntrackLineDistance(int id)
{
	return dsda_RemoveTracker(TrackerType::LineDistance, id);
}

dboolean dsda_TrackSector(int id)
{
	if(dsda_StrictMode())
		return false;

	if(id >= numsectors || id < 0)
		return false;

	return dsda_AddTracker(TrackerType::Sector, id, nullptr);
}

dboolean dsda_UntrackSector(int id)
{
	return dsda_RemoveTracker(TrackerType::Sector, id);
}

dboolean dsda_TrackMobj(int id)
{
	mobj_t* mobj = nullptr;

	if(dsda_StrictMode())
		return false;

	mobj = dsda_FindMobj(id);

	if(!mobj)
		return false;

	{
		mobj_t* target = nullptr;

		// While a mobj is targeted, its address is preserved
		P_SetTarget(&target, mobj);
	}

	return dsda_AddTracker(TrackerType::Mobj, id, mobj);
}

dboolean dsda_UntrackMobj(int id)
{
	return dsda_RemoveTracker(TrackerType::Mobj, id);
}

dboolean dsda_TrackPlayer(int id)
{
	if(dsda_StrictMode())
		return false;

	return dsda_AddTracker(TrackerType::Player, id, nullptr);
}

dboolean dsda_UntrackPlayer(int id)
{
	return dsda_RemoveTracker(TrackerType::Player, id);
}
