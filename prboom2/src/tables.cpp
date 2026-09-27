// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Lookup tables.
 *      Do not try to look them up :-).
 *      In the order of appearance:
 *
 *      int finetangent[4096]   - Tangens LUT.
 *       Should work with BAM fairly well (12 of 16bit,
 *      effectively, by shifting).
 *
 *      int finesine[10240]             - Sine lookup.
 *       Guess what, serves as cosine, too.
 *       Remarkable thing is, how to use BAMs with this?
 *
 *      int tantoangle[2049]    - ArcTan LUT,
 *        maps tan(angle) to angle fast. Gotta search.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include "w_wad.hpp"
#include "tables.hpp"

// killough 5/3/98: reformatted

int SlopeDiv(unsigned int num, unsigned int den)
{
	unsigned ans;

	if(den < 512)
		return SLOPERANGE;
	ans = (num << 3) / (den >> 8);
	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

// [crispy] catch SlopeDiv overflows, only used in rendering
int SlopeDivEx(unsigned int num, unsigned int den)
{
	uint64_t ans;
	if(den < 512)
		return SLOPERANGE;
	ans = ((uint64_t)num << 3) / (den >> 8);
	return ans <= SLOPERANGE ? (int)ans : SLOPERANGE;
}

fixed_t finetangent[4096];

//const fixed_t *const finecosine = &finesine[FINEANGLES/4];

fixed_t finesine[10240];

angle_t tantoangle[2049];

#include "m_swap.hpp"
#include "lprintf.hpp"

// R_LoadTrigTables
// Load trig tables from a wad file lump
// CPhipps 24/12/98 - fix endianness (!)
//
void R_LoadTrigTables()
{
	int lump;
	{
		lump = W_CheckNumForName2("SINETABL", LumpNamespace::Prboom);
		if(lump == LUMP_NOT_FOUND)
			Log::Fatal("Failed to locate trig tables");
		if(W_LumpLength(lump) != sizeof(finesine))
			Log::Fatal("R_LoadTrigTables: Invalid SINETABL");
		W_ReadLump(lump, (unsigned char*)finesine);
	}
	{
		lump = W_CheckNumForName2("TANGTABL", LumpNamespace::Prboom);
		if(lump == LUMP_NOT_FOUND)
			Log::Fatal("Failed to locate trig tables");
		if(W_LumpLength(lump) != sizeof(finetangent))
			Log::Fatal("R_LoadTrigTables: Invalid TANGTABL");
		W_ReadLump(lump, (unsigned char*)finetangent);
	}
	{
		lump = W_CheckNumForName2("TANTOANG", LumpNamespace::Prboom);
		if(lump == LUMP_NOT_FOUND)
			Log::Fatal("Failed to locate trig tables");
		if(W_LumpLength(lump) != sizeof(tantoangle))
			Log::Fatal("R_LoadTrigTables: Invalid TANTOANG");
		W_ReadLump(lump, (unsigned char*)tantoangle);
	}
	// Endianness correction - might still be non-portable, but is fast where possible
	{
		size_t n;
		Log::Debug("Endianness...");

		// This test doesn't assume the endianness of the tables, but deduces them from
		// en entry. I hope this is portable.
		if((10 < finesine[1]) && (finesine[1] < 100))
		{
			Log::Debug("ok.");
			return; // Endianness is correct
		}

		// Must correct endianness of every long loaded (!)
#define CORRECT_TABLE_ENDIAN(tbl) \
    for (n = 0; n<sizeof(tbl)/sizeof(tbl[0]); n++) tbl[n] = doom_swap_l(tbl[n])

		CORRECT_TABLE_ENDIAN(finesine);
		CORRECT_TABLE_ENDIAN(finetangent);
		CORRECT_TABLE_ENDIAN(tantoangle);
		Log::Debug("corrected.");
	}
}
