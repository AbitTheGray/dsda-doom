// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Handles in-memory caching of WAD lumps
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.hpp"
#include "doomtype.hpp"

#include "w_wad.hpp"
#include "z_zone.hpp"
#include "lprintf.hpp"

static void** lump_data;

/* W_InitCache
 *
 * cph 2001/07/07 - split from W_Init
 */
void W_InitCache()
{
	// set up caching
	lump_data = static_cast<void**>(calloc(sizeof *lump_data, numlumps));
	if(!lump_data)
		Log::Fatal("W_Init: Couldn't allocate lump data");
}

void W_DoneCache()
{
}

/* W_LumpByNum
 * killough 4/25/98: simplified
 * CPhipps - modified for new lump locking scheme
 *           returns a const*
 */

const void* W_LumpByNum(int lump)
{
#ifdef RANGECHECK
	if((unsigned)lump >= (unsigned)numlumps)
		Log::Fatal("W_LumpByNum: {} >= numlumps", lump);
#endif

	// read the lump in
	if(!lump_data[lump])
	{
		lump_data[lump] = Z_Malloc(W_LumpLength(lump));
		W_ReadLump(lump, lump_data[lump]);
	}

	return lump_data[lump];
}

const void* W_LockLumpNum(int lump)
{
	return W_LumpByNum(lump);
}

void* W_GetModifiableLumpData(int lump)
{
	return lump_data[lump];
}
