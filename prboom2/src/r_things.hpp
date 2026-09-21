// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Rendering of moving objects, sprites.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "r_draw.hpp"

#define MINZ        (FRACUNIT*4)

extern int r_near_clip_plane;

/* Constant arrays used for psprite clipping and initializing clipping. */

// e6y: resolution limitation is removed
extern int* negonearray; /* killough 2/8/98: */       // dropoff overflow
extern int* screenheightarray; /* change to MAX_*  */ // dropoff overflow

/* Vars for R_DrawMaskedColumn */

extern int* mfloorclip;   // dropoff overflow
extern int* mceilingclip; // dropoff overflow
extern fixed_t spryscale;
extern int64_t sprtopscreen;
extern fixed_t pspriteiscale;
/* proff 11/06/98: Added for high-res */
extern fixed_t pspritexscale;
extern fixed_t pspriteyscale;
extern fixed_t pspriteiyscale;
//e6y: added for GL
extern float pspritexscale_f;
extern float pspriteyscale_f;

void R_DrawMaskedColumn(const rpatch_t* patch,
	R_DrawColumn_f colfunc,
	draw_column_vars_t* dcvars,
	const rcolumn_t* column,
	const rcolumn_t* prevcolumn,
	const rcolumn_t* nextcolumn);
void R_SortVisSprites();
void R_AddSprites(subsector_t* subsec, int lightlevel);
void R_AddAllAliveMonstersSprites();
void R_DrawPlayerSprites();
void R_InitSpritesRes();
void R_InitSprites(const char* const * namelist);
void R_ClearSprites();
void R_DrawMasked();

void R_SetClipPlanes();

#ifdef __cplusplus
}
#endif
