// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Tracker

#pragma once

#include "p_mobj.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define TRACKER_LIMIT 16

enum struct TrackerType : int32_t
{
	Nothing,
	Line,
	LineDistance,
	Sector,
	Mobj,
	Player,
};

typedef struct
{
	TrackerType type;
	int id;
	mobj_t* mobj;
} dsda_tracker_t;

dboolean dsda_TrackLine(int id);
dboolean dsda_UntrackLine(int id);
dboolean dsda_TrackLineDistance(int id);
dboolean dsda_UntrackLineDistance(int id);
dboolean dsda_TrackSector(int id);
dboolean dsda_UntrackSector(int id);
dboolean dsda_TrackMobj(int id);
dboolean dsda_UntrackMobj(int id);
dboolean dsda_TrackPlayer(int id);
dboolean dsda_UntrackPlayer(int id);
void dsda_WipeTrackers();
void dsda_ResetTrackers();
mobj_t* dsda_FindMobj(int id);

#ifdef __cplusplus
}
#endif
