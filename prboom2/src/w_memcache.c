// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Handles in-memory caching of WAD lumps
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "doomtype.h"

#include "w_wad.h"
#include "z_zone.h"
#include "lprintf.h"

static void **lump_data;

/* W_InitCache
 *
 * cph 2001/07/07 - split from W_Init
 */
void W_InitCache(void)
{
  // set up caching
  lump_data = calloc(sizeof *lump_data, numlumps);
  if (!lump_data)
    I_Error ("W_Init: Couldn't allocate lump data");
}

void W_DoneCache(void)
{
}

/* W_LumpByNum
 * killough 4/25/98: simplified
 * CPhipps - modified for new lump locking scheme
 *           returns a const*
 */

const void *W_LumpByNum(int lump)
{
#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error ("W_LumpByNum: %i >= numlumps",lump);
#endif

  // read the lump in
  if (!lump_data[lump]) {
    lump_data[lump] = Z_Malloc(W_LumpLength(lump));
    W_ReadLump(lump, lump_data[lump]);
  }

  return lump_data[lump];
}

const void *W_LockLumpNum(int lump)
{
  return W_LumpByNum(lump);
}

void *W_GetModifiableLumpData(int lump)
{
  return lump_data[lump];
}
