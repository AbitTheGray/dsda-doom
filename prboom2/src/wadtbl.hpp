// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Ex Demo Wad Table
 */

#pragma once

#include "doomtype.hpp"
#include "w_wad.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	wadinfo_t header;
	filelump_t* lumps;
	char* data;
	int datasize;
} wadtbl_t;

#define PWAD_SIGNATURE "PWAD"

void InitPWADTable(wadtbl_t* wadtbl);
void FreePWADTable(wadtbl_t* wadtbl);
void AddPWADTableLump(wadtbl_t* wadtbl, const char* name, const byte* data, size_t size);
wadinfo_t* ReadPWADTable(byte* buffer, size_t size);

#ifdef __cplusplus
}
#endif
