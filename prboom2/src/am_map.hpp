// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  AutoMap module.
 */

#pragma once

#include "d_event.hpp"
#include "m_fixed.hpp"
#include "m_misc.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef struct
{
	int back;
	int grid;
	int wall;
	int fchg;
	int cchg;
	int clsd;
	int rkey;
	int bkey;
	int ykey;
	int rdor;
	int bdor;
	int ydor;
	int tele;
	int secr;
	int revsecr;
	int tagfinder;
	int exit;
	int exitsecr;
	int unsn;
	int flat;
	int sprt;
	int item;
	int frnd;
	int enemy;
	int hair;
	int sngl;
	int me;
	int plyr[8];
	int trail_1;
	int trail_2;
	int pickup;
} mapcolor_t;

typedef struct map_point_s
{
	float x, y;
	unsigned char r, g, b, a;
}
	PACKEDATTR map_point_t;

typedef struct map_line_s
{
	map_point_t point[2];
}
	PACKEDATTR map_line_t;

extern array_t map_lines;

#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// Called by main loop.
dboolean AM_Responder(event_t* ev);

// Called by main loop.
void AM_Ticker();

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer(dboolean minimap);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Stop(dboolean minimap);

// killough 2/22/98: for saving automap information in savegame:

enum struct AutomapStart : int32_t
{
	Minimap,
	FullAutomap
};

void AM_Start(AutomapStart open_full_automap);

//jff 4/16/98 make externally available

void AM_clearMarks();

void AM_setMarkParams(int num);

void AM_SetResolution();

typedef struct
{
	fixed_t x, y;
	float fx, fy;
} mpoint_t;

typedef struct
{
	fixed_t x, y;
	fixed_t w, h;

	char label[16];
	int widths[16];
} markpoint_t;

extern markpoint_t* markpoints;
extern int markpointnum, markpointnum_max;

// end changes -- killough 2/22/98

extern mapcolor_t mapcolor;

void M_ChangeMapTextured();
void M_ChangeMapMultisamling();
void AM_ResetIDDTcheat();
void AM_SetMapCenter(fixed_t x, fixed_t y);

typedef struct am_frame_s
{
	fixed_t centerx, centery;
	fixed_t sin, cos;

	float centerx_f, centery_f;
	float sin_f, cos_f;

	fixed_t bbox[4];

	int precise;
} am_frame_t;

extern am_frame_t am_frame;

enum struct MapThingsAppearance : int32_t
{
	Classic,
	Scaled,
#if defined(HAVE_LIBSDL2_IMAGE)
	Icon,
#endif
	Box,

	Count
};

enum struct MapTrailMode : int32_t
{
	Off,
	IgnoreCollisions,
	IncludeCollisions,
	Count
};

extern MapTrailMode map_trail_mode;

void AM_updatePlayerTrail(fixed_t x, fixed_t y);
void AM_RefreshMinimap();

#ifdef __cplusplus
}
#endif
