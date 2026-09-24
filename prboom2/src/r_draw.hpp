// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      System specific interface stuff.
 */

#pragma once

#include "r_patch.hpp"

#include "r_defs.hpp"

enum struct DrawColumnFlag : uint32_t
{
	IsPatch = 0x00000001
};
ENUM_FLAGS_FUNC(DrawColumnFlag)

#ifdef __cplusplus
extern "C"
{
#endif

enum struct ColumnPipeline : int32_t
{
	Standard,
	Translucent,
	Translated,
	Fuzz,
	Count,
};

// Used to specify what kind of filering you want
enum struct DrawFilterType : int32_t
{
	None,
	Point,
	Count
};

typedef struct draw_column_vars_s* pdraw_column_vars_s;
typedef void (*R_DrawColumn_f)(pdraw_column_vars_s dcvars);

// Packaged into a struct - POPE
typedef struct draw_column_vars_s
{
	int x;
	int yl;
	int yh;
	int dy;
	fixed_t iscale;
	fixed_t texturemid;
	int texheight;          // killough
	const byte* source;     // first pixel in a column
	const byte* prevsource; // first pixel in previous column
	const byte* nextsource; // first pixel in next column
	const lighttable_t* colormap;
	const byte* translation;
	EdgeSlope edgeslope; // OR'ed EdgeSlope values
	// 1 if R_DrawColumn* is currently drawing a masked column, otherwise 0
	int drawingmasked;
	DrawColumnFlag flags; //e6y: for detect patches ind colfunc()

	// [AR] mark weapon sprite
	dboolean isplayersprite;
	int pspritepostheight;

	// heretic
	int baseclip;
} draw_column_vars_t;

void R_SetDefaultDrawColumnVars(draw_column_vars_t* dcvars);

typedef struct
{
	int y;
	int x1;
	int x2;
	fixed_t z; // the current span z coord
	fixed_t xfrac;
	fixed_t yfrac;
	fixed_t xstep;
	fixed_t ystep;
	const byte* source; // start of a 64*64 tile image
	const lighttable_t* colormap;

	fixed_t xoffs;
	fixed_t yoffs;
	fixed_t xscale;
	fixed_t yscale;
	fixed_t sine;
	fixed_t cosine;
	fixed_t planeheight;
	const lighttable_t** planezlight;
} draw_span_vars_t;

typedef struct
{
	byte* topleft;
	int pitch;
} draw_vars_t;

extern draw_vars_t drawvars;

extern byte playernumtotrans[MAX_MAXPLAYERS]; // CPhipps - what translation table for what player
extern byte* translationtables;

R_DrawColumn_f R_GetDrawColumnFunc(ColumnPipeline type, DrawFilterType filterz);

// Span blitting for rows, floor/ceiling. No Spectre effect needed.
void R_DrawSpan(draw_span_vars_t* dsvars);

void R_InitBuffer(int width, int height);

void R_InitBuffersRes();

// Initialize color translation tables, for player rendering etc.
void R_InitTranslationTables();

// Rendering function.
void R_FillBackScreen();

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder();

// haleyjd 09/13/04: new function to call from main rendering loop
// which gets rid of the unnecessary reset of various variables during
// column drawing.
void R_ResetColumnBuffer();

void R_SetFuzzPos(int fuzzpos);
int R_GetFuzzPos();

// height is the height of the last column, in pixels
void R_ResetFuzzCol(int height);

// Calls R_ResetFuzzCol if x is aligned to the fuzz cell grid
void R_CheckFuzzCol(int x, int height);

extern int fuzz_cutoff;

#ifdef __cplusplus
}
#endif
