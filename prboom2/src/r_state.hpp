// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Refresh/render internal state variables (global).
 */

#pragma once

#include "d_player.hpp"
#include "r_data.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

// Need data structure definitions.

//
// Refresh internal data structures,
//  for rendering.
//

// needed for texture pegging
extern fixed_t* textureheight;

extern int firstflat, numflats;

// for global animation
extern int* flattranslation;
extern int* texturetranslation;

// Sprite....
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;

//
// Lookup tables for map data.
//
extern spritedef_t* sprites;

extern int numvertexes;
extern vertex_t* vertexes;

extern int numsegs;
extern seg_t* segs;

extern int numsectors;
extern sector_t* sectors;

extern int numsubsectors;
extern subsector_t* subsectors;

extern int numnodes;
extern node_t* nodes;

extern int numlines;
extern line_t* lines;

extern int numsides;
extern side_t* sides;

extern int* sslines_indexes;
extern ssline_t* sslines;

extern byte* map_subsectors;

//
// POV data.
//
extern fixed_t viewx;
extern fixed_t viewy;
extern fixed_t viewz;
extern angle_t viewangle;
extern player_t* viewplayer;
extern angle_t clipangle;
extern int viewangletox[FINEANGLES / 2];

// e6y: resolution limitation is removed
extern angle_t* xtoviewangle; // killough 2/8/98

extern int FieldOfView;

extern fixed_t rw_distance;
extern angle_t rw_normalangle;

// angle to line origin
extern int rw_angle1;

extern visplane_t* floorplane;
extern visplane_t* ceilingplane;

// [FG] linear horizontal sky scrolling
extern angle_t* linearskyangle;

#ifdef __cplusplus
}
#endif
