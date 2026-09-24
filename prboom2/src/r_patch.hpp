// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

enum struct EdgeSlope : uint32_t
{
	TopUp   = (1 << 0),
	TopDown = (1 << 1),
	BotUp   = (1 << 2),
	BotDown = (1 << 3),
	TopMask = 0x3,
	BotMask = 0xc,
};
ENUM_FLAGS_FUNC(EdgeSlope)

enum struct PatchFlag : uint32_t
{
	IsNotTileable = 0x00000001,
	Repeat        = 0x00000002,
	HasHoles      = 0x00000004,
};
ENUM_FLAGS_FUNC(PatchFlag)

#ifdef __cplusplus
extern "C"
{
#endif

// Used to specify the sloping of the top and bottom of a column post

//e6y

typedef struct
{
	int topdelta;
	int length;
	EdgeSlope slope;
} rpost_t;

typedef struct
{
	int numPosts;
	rpost_t* posts;
	unsigned char* pixels;
} rcolumn_t;

typedef struct
{
	int width;
	int height;
	unsigned widthmask;

	int leftoffset;
	int topoffset;

	// this is the single malloc'ed/free'd array
	// for this patch
	unsigned char* data;

	// these are pointers into the data array
	unsigned char* pixels;
	rcolumn_t* columns;
	rpost_t* posts;

	PatchFlag flags; //e6y
} rpatch_t;

const rpatch_t* R_PatchByNum(int id);
#define R_PatchByName(name) R_PatchByNum(W_GetNumForName(name))

const rpatch_t* R_TextureCompositePatchByNum(int id);

dboolean R_IsPatchLump(int lumpnum);

// Size query funcs
int R_NumPatchWidth(int lump);
int R_NumPatchHeight(int lump);
#define R_NamePatchWidth(name) R_NumPatchWidth(W_GetNumForName(name))
#define R_NamePatchHeight(name) R_NumPatchHeight(W_GetNumForName(name))

const rcolumn_t* R_GetPatchColumnWrapped(const rpatch_t* patch, int columnIndex);
const rcolumn_t* R_GetPatchColumnClamped(const rpatch_t* patch, int columnIndex);

// returns R_GetPatchColumnWrapped for square, non-holed textures
// and R_GetPatchColumnClamped otherwise
const rcolumn_t* R_GetPatchColumn(const rpatch_t* patch, int columnIndex);

void R_InitPatches();
void R_UpdatePlayPal();
void R_FlushAllPatches();

extern int playpal_darkest;
extern int playpal_lightest;

#ifdef __cplusplus
}
#endif
