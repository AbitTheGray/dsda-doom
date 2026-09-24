// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Refresh module, data I/O, caching, retrieval of graphics
 *  by name.
 */

#pragma once

#include "r_defs.hpp"
#include "r_state.hpp"
#include "r_patch.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

// A single patch from a texture definition, basically
// a rectangular area within the texture rectangle.
typedef struct
{
	int originx, originy; // Block origin, which has already accounted
	int patch;            // for the internal origin of the patch.
} texpatch_t;

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//

typedef struct
{
	char name[8];    // Keep name for switch changing, etc.
	int next, index; // killough 1/31/98: used in hashing algorithm
	// CPhipps - moved arrays with per-texture entries to elements here
	unsigned widthmask;
	// CPhipps - end of additions
	short width, height;
	short patchcount;      // All the patches[patchcount] are drawn
	texpatch_t patches[1]; // back-to-front into the cached texture.
} texture_t;

extern int numtextures;
extern texture_t** textures;

const byte* R_GetTextureColumn(const rpatch_t* texpatch, int col);

// I/O, setting up the stuff.
void R_InitData();
void R_PrecacheLevel();

// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int R_FlatNumForName(const char* name); // killough -- const added

// R_*TextureNumForName returns the texture number for the texture name, or NO_TEXTURE if
//  there is no texture (i.e. "-") specified.
/* cph 2006/07/23 - defined value for no-texture marker (texture "-" in the WAD file) */
#define NO_TEXTURE 0
int PUREFUNC R_TextureNumForName(const char* name); // killough -- const added; cph - now PUREFUNC
int PUREFUNC R_SafeTextureNumForName(const char* name, int snum);
int PUREFUNC R_CheckTextureNumForName(const char* name);

int R_ColormapNumForName(const char* name); // killough 4/4/98

extern const byte *main_tranmap, *tranmap;

/* Proff - Added for OpenGL - cph - const char* param */
void R_SetPatchNum(patchnum_t* patchnum, const char* name);
// e6y: Added for "GRNROCK" mostly
void R_SetFloorNum(patchnum_t* patchnum, const char* name);
int R_SetSpriteByIndex(patchnum_t* patchnum, SpriteId item);
int R_SetSpriteByName(patchnum_t* patchnum, const char* name);
int R_SetPatchByName(patchnum_t* patchnum, const char* name);
int R_NumPatchForSpriteIndex(SpriteId item);

#ifdef __cplusplus
}
#endif
