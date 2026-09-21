// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef R_PATCH_H
#define R_PATCH_H

// Used to specify the sloping of the top and bottom of a column post
typedef enum {
  RDRAW_EDGESLOPE_TOP_UP   = (1<<0),
  RDRAW_EDGESLOPE_TOP_DOWN = (1<<1),
  RDRAW_EDGESLOPE_BOT_UP   = (1<<2),
  RDRAW_EDGESLOPE_BOT_DOWN = (1<<3),
  RDRAW_EDGESLOPE_TOP_MASK = 0x3,
  RDRAW_EDGESLOPE_BOT_MASK = 0xc,
} edgeslope_t;

//e6y
typedef enum {
  PATCH_ISNOTTILEABLE   = 0x00000001,
  PATCH_REPEAT          = 0x00000002,
  PATCH_HASHOLES        = 0x00000004,
} rpatch_flag_t;

typedef struct {
  int topdelta;
  int length;
  edgeslope_t slope;
} rpost_t;

typedef struct {
  int numPosts;
  rpost_t *posts;
  unsigned char *pixels;
} rcolumn_t;

typedef struct {
  int width;
  int height;
  unsigned  widthmask;

  int leftoffset;
  int topoffset;

  // this is the single malloc'ed/free'd array
  // for this patch
  unsigned char *data;

  // these are pointers into the data array
  unsigned char *pixels;
  rcolumn_t *columns;
  rpost_t *posts;

  unsigned int flags;//e6y
} rpatch_t;


const rpatch_t *R_PatchByNum(int id);
#define R_PatchByName(name) R_PatchByNum(W_GetNumForName(name))

const rpatch_t *R_TextureCompositePatchByNum(int id);

dboolean R_IsPatchLump(int lumpnum);

// Size query funcs
int R_NumPatchWidth(int lump) ;
int R_NumPatchHeight(int lump);
#define R_NamePatchWidth(name) R_NumPatchWidth(W_GetNumForName(name))
#define R_NamePatchHeight(name) R_NumPatchHeight(W_GetNumForName(name))


const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex);
const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex);


// returns R_GetPatchColumnWrapped for square, non-holed textures
// and R_GetPatchColumnClamped otherwise
const rcolumn_t *R_GetPatchColumn(const rpatch_t *patch, int columnIndex);


void R_InitPatches();
void R_UpdatePlayPal();
void R_FlushAllPatches();

extern int playpal_darkest;
extern int playpal_lightest;

#endif
