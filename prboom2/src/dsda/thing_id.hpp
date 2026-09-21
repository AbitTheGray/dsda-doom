// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Thing ID

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "p_mobj.hpp"

typedef struct thing_id_list_entry_s
{
	mobj_t* mo;
	struct thing_id_list_entry_s* next;
} thing_id_list_entry_t;

typedef struct
{
	dboolean done;
	struct thing_id_list_entry_s* start;
} thing_id_search_t;

typedef struct thing_id_list_s
{
	short thing_id;
	struct thing_id_list_entry_s* first;
	struct thing_id_list_entry_s* last;
	struct thing_id_list_s* next;
} thing_id_list_t;

void dsda_AddMobjThingID(mobj_t* mo, short thing_id);
void dsda_RemoveMobjThingID(mobj_t* mo);
void dsda_BuildMobjThingIDList();
void dsda_ResetThingIDSearch(thing_id_search_t* search);
mobj_t* dsda_FindMobjFromThingID(short thing_id, thing_id_search_t* search);
mobj_t* dsda_FindMobjFromThingIDOrMobj(short thing_id, mobj_t* mo, thing_id_search_t* search);

#ifdef __cplusplus
}
#endif
