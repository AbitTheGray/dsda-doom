// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Setup a game, startup stuff.
 */

#pragma once

#include "p_mobj.h"

void P_SetupLevel(int episode, int map, int skill);
void P_Init(void); /* Called by startup code. */

extern const byte* rejectmatrix; /* for fast sight rejection -  cph - const* */

/* killough 3/1/98: change blockmap from "short" to "long" offsets: */
extern int* blockmaplump; /* offsets in blockmap are from here */
extern int* blockmap;
extern int bmapwidth;
extern int bmapheight; /* in mapblocks */
extern fixed_t bmaporgx;
extern fixed_t bmaporgy;    /* origin of block map */
extern mobj_t** blocklinks; /* for thing chains */

extern dboolean skipblstart; // MaxW: Skip initial blocklist short

// MAES: extensions to support 512x512 blockmaps.
extern int blockmapxneg;
extern int blockmapyneg;

typedef struct
{
	int width;
	int height;
	fixed_t orgx;
	fixed_t orgy;
} blockmap_t;

extern blockmap_t original_blockmap;

void P_RestoreOriginalBlockMap(void);

typedef struct
{
	void (*load_vertexes)(int lump);
	void (*load_sectors)(int lump);
	void (*load_things)(int lump);
	void (*load_linedefs)(int lump);
	void (*allocate_sidedefs)(int lump);
	void (*load_sidedefs)(int lump);
	void (*update_level_components)(int lumpnum);
	void (*po_load_things)(int lump);
} map_loader_t;

extern map_loader_t map_loader;
