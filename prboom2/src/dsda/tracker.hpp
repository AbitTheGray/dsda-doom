// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Tracker

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "p_mobj.hpp"

#define TRACKER_LIMIT 16

typedef enum
{
	dsda_tracker_nothing,
	dsda_tracker_line,
	dsda_tracker_line_distance,
	dsda_tracker_sector,
	dsda_tracker_mobj,
	dsda_tracker_player,
} dsda_tracker_type_t;

typedef struct
{
	dsda_tracker_type_t type;
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
