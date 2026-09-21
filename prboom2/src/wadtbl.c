// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Ex Demo Wad Table
 */

#include <string.h>

#include "lprintf.h"
#include "m_swap.h"
#include "z_zone.h"

#include "wadtbl.h"

void InitPWADTable(wadtbl_t* wadtbl)
{
	//init header signature and lookup table offset and size
	memcpy(wadtbl->header.identification, PWAD_SIGNATURE, 4);
	wadtbl->header.infotableofs = sizeof(wadtbl->header);
	wadtbl->header.numlumps = 0;

	//clear PWAD lookup table
	wadtbl->lumps = NULL;

	//clear PWAD data
	wadtbl->data = NULL;
	wadtbl->datasize = 0;
}

void FreePWADTable(wadtbl_t* wadtbl)
{
	//clear PWAD lookup table
	Z_Free(wadtbl->lumps);

	//clear PWAD data
	Z_Free(wadtbl->data);
}

void AddPWADTableLump(wadtbl_t* wadtbl, const char* name, const byte* data, size_t size)
{
	int lumpnum;

	if(!wadtbl || (name && strlen(name) > 8))
	{
		I_Error("W_AddLump: wrong parameters.");
		return;
	}

	lumpnum = wadtbl->header.numlumps;

	if(name)
	{
		wadtbl->lumps = Z_Realloc(wadtbl->lumps, (lumpnum + 1) * sizeof(wadtbl->lumps[0]));

		memcpy(wadtbl->lumps[lumpnum].name, name, 8);
		wadtbl->lumps[lumpnum].size = size;
		wadtbl->lumps[lumpnum].filepos = wadtbl->header.infotableofs;

		wadtbl->header.numlumps++;
	}

	if(data && size > 0)
	{
		wadtbl->data = Z_Realloc(wadtbl->data, wadtbl->datasize + size);

		memcpy(wadtbl->data + wadtbl->datasize, data, size);
		wadtbl->datasize += size;

		wadtbl->header.infotableofs += size;
	}
}

wadinfo_t* ReadPWADTable(byte* buffer, size_t size)
{
	int i;
	unsigned int length;
	wadinfo_t* header;
	const filelump_t* fileinfo;

	if(buffer && size > sizeof(*header))
	{
		header = (wadinfo_t*)buffer;
		if(strncmp(header->identification, "IWAD", 4) == 0 ||
			strncmp(header->identification, "PWAD", 4) == 0)
		{
			header->numlumps = LittleLong(header->numlumps);
			header->infotableofs = LittleLong(header->infotableofs);
			length = header->numlumps * sizeof(filelump_t);

			if(header->infotableofs + length <= size)
			{
				fileinfo = (const filelump_t*)(buffer + header->infotableofs);
				for(i = 0; i < header->numlumps; i++, fileinfo++)
				{
					if(fileinfo->filepos < 0 ||
						fileinfo->filepos > header->infotableofs ||
						fileinfo->filepos + fileinfo->size > header->infotableofs)
					{
						break;
					}
				}
				if(i == header->numlumps)
					return header;
			}
		}
	}

	return NULL;
}
