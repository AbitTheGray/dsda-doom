// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Handles WAD file header, directory, lump I/O.
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _MSC_VER
#include <stddef.h>
#include <io.h>
#endif

#include <array>
#include <string_view>

#include "doomstat.hpp"
#include "d_net.hpp"
#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_file.hpp"
#include "r_main.hpp"

#include "w_wad.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"

#include "dsda/utility.hpp"

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t* lumpinfo;
int numlumps; // killough

int MainLumpCache = false;

void ExtractFileBase(const char* path, char* dest)
{
	const char* src = path + strlen(path) - 1;
	int length;

	// back up until a \ or the start
	while(src != path && src[-1] != ':' // killough 3/22/98: allow c:filename
		&& *(src - 1) != '\\'
		&& *(src - 1) != '/')
	{
		src--;
	}

	// copy up to eight characters
	memset(dest, 0, 8);
	length = 0;

	while((*src) && (*src != '.') && (++length < 9))
	{
		*dest++ = toupper(*src++);
	}
	/* cph - length check removed, just truncate at 8 chars.
	* If there are 8 or more chars, we'll copy 8, and no zero termination
	*/
}

//
// 1/18/98 killough: adds a default extension to a path
// Note: Backslashes are treated specially, for MS-DOS.
//

char* AddDefaultExtension(char* path, const char* ext)
{
	char* p = path;
	while(*p++);
	while(p-- > path && *p != '/' && *p != '\\')
		if(*p == '.')
			return path;
	if(*ext != '.')
		strcat(path, ".");
	return strcat(path, ext);
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// Reload hack removed by Lee Killough
// CPhipps - source is an enum
//
// proff - changed using pointer to wadfile_info_t
static void W_AddFile(wadfile_info_t* wadfile)
// killough 1/31/98: static, const
{
	wadinfo_t header;
	lumpinfo_t* lump_p;
	unsigned i;
	int length;
	int startlump;
	filelump_t *fileinfo, *fileinfo2free = nullptr; //killough
	filelump_t singleinfo;
	int flags = 0;

	if(wadfile->src == WadSource::Skip)
	{
		return;
	}

	// Close any existing handle
	if(wadfile->handle > 0)
	{
		close(wadfile->handle);
		wadfile->handle = 0;
	}

	// open the file and add to directory

	wadfile->handle = M_OpenRB(wadfile->name);
	if(wadfile->handle == -1)
	{
		if(!dsda_HasFileExt(wadfile->name, ".lmp"))
			I_Error("W_AddFile: couldn't open %s", wadfile->name);
		return;
	}

	//jff 8/3/98 use logical output routine
	if(MainLumpCache)
		lprintf(OutputLevels::Info, " adding %s\n", wadfile->name);
	startlump = numlumps;

	// mark lumps from internal resource
	if(wadfile->src == WadSource::PortWad)
	{
		int len = strlen(WAD_DATA);
		int len_file = strlen(wadfile->name);
		if(len_file >= len)
		{
			if(!strcasecmp(wadfile->name + len_file - len, WAD_DATA))
			{
				flags = LUMP_PRBOOM;
			}
		}
	}

	if(!dsda_HasFileExt(wadfile->name, ".wad"))
	{
		// single lump file
		fileinfo = &singleinfo;
		singleinfo.filepos = 0;
		singleinfo.size = LittleLong(I_Filelength(wadfile->handle));
		ExtractFileBase(wadfile->name, singleinfo.name);
		numlumps++;
	}
	else
	{
		// WAD file
		I_Read(wadfile->handle, &header, sizeof(header));
		if(strncmp(header.identification, "IWAD", 4) &&
			strncmp(header.identification, "PWAD", 4))
			I_Error("W_AddFile: Wad file %s doesn't have IWAD or PWAD id", wadfile->name);
		header.numlumps = LittleLong(header.numlumps);
		header.infotableofs = LittleLong(header.infotableofs);
		length = header.numlumps * sizeof(filelump_t);
		fileinfo2free = fileinfo = static_cast<filelump_t*>(Z_Malloc(length)); // killough
		lseek(wadfile->handle, header.infotableofs, SEEK_SET),
			I_Read(wadfile->handle, fileinfo, length);
		numlumps += header.numlumps;
	}

	// Fill in lumpinfo
	lumpinfo = static_cast<lumpinfo_t*>(Z_Realloc(lumpinfo, numlumps * sizeof(lumpinfo_t)));

	lump_p = &lumpinfo[startlump];

	for(i = startlump; (int)i < numlumps; i++, lump_p++, fileinfo++)
	{
		lump_p->flags = flags;
		// Not hashed until W_HashLumps; lookups before that find nothing
		lump_p->index = LUMP_NOT_FOUND;
		lump_p->next = LUMP_NOT_FOUND;
		lump_p->wadfile = wadfile; //  killough 4/25/98
		lump_p->position = LittleLong(fileinfo->filepos);
		lump_p->size = LittleLong(fileinfo->size);
		if(wadfile->src == WadSource::Lmp)
		{
			// Modifications to place command-line-added demo lumps
			// into a separate "ns_demos" namespace so that they cannot
			// conflict with other lump names
			lump_p->li_namespace = LumpNamespace::Demos;
		}
		else
		{
			lump_p->li_namespace = LumpNamespace::Global; // killough 4/17/98
		}
		strncpy(lump_p->name, fileinfo->name, 8);
		lump_p->source = wadfile->src; // Ty 08/29/98
	}

	Z_Free(fileinfo2free); // killough
}

// jff 1/23/98 Create routines to reorder the master directory
// putting all flats into one marked block, and all sprites into another.
// This will allow loading of sprites and flats from a PWAD with no
// other changes to code, particularly fast hashes of the lumps.
//
// killough 1/24/98 modified routines to be a little faster and smaller

static int IsMarker(const char* marker, const char* name)
{
	return !strncasecmp(name, marker, 8) ||
		// doubled first character test for single-character prefixes only
		// FF_* is valid alias for F_*, but HI_* should not allow HHI_*
		(marker[1] == '_' && *name == *marker && !strncasecmp(name + 1, marker, 7));
}

// killough 4/17/98: add namespace tags

static int W_CoalesceMarkedResource(const char* start_marker,
	const char* end_marker, LumpNamespace li_namespace)
{
	int result = 0;
	lumpinfo_t* marked = static_cast<lumpinfo_t*>(Z_Malloc(sizeof(*marked) * numlumps));
	size_t i, num_marked = 0, num_unmarked = 0;
	int is_marked = 0, mark_end = 0;
	lumpinfo_t* lump = lumpinfo;

	for(i = numlumps; i--; lump++)
		if(IsMarker(start_marker, lump->name)) // start marker found
		{
			// If this is the first start marker, add start marker to marked lumps
			if(!num_marked)
			{
				strncpy(marked->name, start_marker, 8);
				marked->size = 0;                 // killough 3/20/98: force size to be 0
				marked->li_namespace = LumpNamespace::Global; // killough 4/17/98
				marked->wadfile = nullptr;
				num_marked = 1;
			}
			is_marked = 1; // start marking lumps
		}
		else if(IsMarker(end_marker, lump->name)) // end marker found
		{
			mark_end = 1;  // add end marker below
			is_marked = 0; // stop marking lumps
		}
		else if(is_marked || lump->li_namespace == li_namespace)
		{
			// if we are marking lumps,
			// move lump to marked list
			// sf: check for namespace already set

			// sf 26/10/99:
			// ignore sprite lumps smaller than 8 bytes (the smallest possible)
			// in size -- this was used by some dmadds wads
			// as an 'empty' graphics resource
			if(li_namespace != LumpNamespace::Sprites || lump->size > 8)
			{
				marked[num_marked] = *lump;
				marked[num_marked++].li_namespace = li_namespace; // killough 4/17/98
				result++;
			}
		}
		else
			lumpinfo[num_unmarked++] = *lump; // else move down THIS list

	// Append marked list to end of unmarked list
	memcpy(lumpinfo + num_unmarked, marked, num_marked * sizeof(*marked));

	Z_Free(marked); // free marked list

	numlumps = num_unmarked + num_marked; // new total number of lumps

	if(mark_end) // add end marker
	{
		lumpinfo[numlumps].size = 0; // killough 3/20/98: force size to be 0
		lumpinfo[numlumps].wadfile = nullptr;
		lumpinfo[numlumps].li_namespace = LumpNamespace::Global; // killough 4/17/98
		strncpy(lumpinfo[numlumps++].name, end_marker, 8);
	}

	return result;
}

// Hash function used for lump names.
// Must be mod'ed with table size.
// Can be used for any 8-character names.
// by Lee Killough

unsigned W_LumpNameHash(const char* s)
{
	unsigned hash;
	(void)((hash = toupper(s[0]), s[1]) &&
		(hash = hash * 3 + toupper(s[1]), s[2]) &&
		(hash = hash * 2 + toupper(s[2]), s[3]) &&
		(hash = hash * 2 + toupper(s[3]), s[4]) &&
		(hash = hash * 2 + toupper(s[4]), s[5]) &&
		(hash = hash * 2 + toupper(s[5]), s[6]) &&
		(hash = hash * 2 + toupper(s[6]),
			hash = hash * 2 + toupper(s[7]))
	);
	return hash;
}

//
// W_CheckNumForName
// Returns LUMP_NOT_FOUND if name not found.
//
// Rewritten by Lee Killough to use hash table for performance. Significantly
// cuts down on time -- increases Doom performance over 300%. This is the
// single most important optimization of the original Doom sources, because
// lump name lookup is used so often, and the original Doom used a sequential
// search. For large wads with > 1000 lumps this meant an average of over
// 500 were probed during every search. Now the average is under 2 probes per
// search. There is no significant benefit to packing the names into longwords
// with this new hashing algorithm, because the work to do the packing is
// just as much work as simply doing the string comparisons with the new
// algorithm, which minimizes the expected number of comparisons to under 2.
//
// killough 4/17/98: add namespace parameter to prevent collisions
// between different resources such as flats, sprites, colormaps
//

// W_FindNumFromName, an iterative version of W_CheckNumForName
// returns list of lump numbers for a given name (latest first)
//
int W_FindNumFromName2(const char* name, LumpNamespace li_namespace, int i)
{
	// Hash function maps the name to one of possibly numlump chains.
	// It has been tuned so that the average chain length never exceeds 2.

	// proff 2001/09/07 - check numlumps==0, this happens when called before WAD loaded
	if(numlumps == 0)
		i = LUMP_NOT_FOUND;
	else
	{
		if(i < 0)
			i = lumpinfo[W_LumpNameHash(name) % (unsigned)numlumps].index;
		else
			i = lumpinfo[i].next;

		// We search along the chain until end, looking for case-insensitive
		// matches which also match a namespace tag. Separate hash tables are
		// not used for each namespace, because the performance benefit is not
		// worth the overhead, considering namespace collisions are rare in
		// Doom wads.

		while(i != LUMP_NOT_FOUND && (strncasecmp(lumpinfo[i].name, name, 8) ||
			lumpinfo[i].li_namespace != li_namespace))
			i = lumpinfo[i].next;
	}

	// Return the matching lump, or LUMP_NOT_FOUND if none found.

	return i;
}

//
// killough 1/31/98: Initialize lump hash table
//

void W_HashLumps()
{
	int i;

	for(i = 0; i < numlumps; i++)
		lumpinfo[i].index = LUMP_NOT_FOUND; // mark slots empty

	// Insert nodes to the beginning of each chain, in first-to-last
	// lump order, so that the last lump of a given name appears first
	// in any chain, observing pwad ordering rules. killough

	for(i = 0; i < numlumps; i++)
	{
		// hash function:
		int j = W_LumpNameHash(lumpinfo[i].name) % (unsigned)numlumps;
		lumpinfo[i].next = lumpinfo[j].index; // Prepend to list
		lumpinfo[j].index = i;
	}
}

// End of lump hashing -- killough 1/31/98



// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName(const char* name) // killough -- const added
{
	int i = W_CheckNumForName(name);
	if(i == LUMP_NOT_FOUND)
		I_Error("W_GetNumForName: %.8s not found", name);
	return i;
}

const lumpinfo_t* W_GetLumpInfoByNum(int lump)
{
	if(lump < 0 || lump >= numlumps)
		I_Error("W_GetLumpInfoByNum: lump num %d out of range", lump);

	return &lumpinfo[lump];
}

// W_CheckNumForNameInternal
// checks only internal resource
//
int W_CheckNumForNameInternal(const char* name)
{
	int p;
	for(p = LUMP_NOT_FOUND; (p = W_ListNumFromName(name, p)) != LUMP_NOT_FOUND;)
	{
		if(lumpinfo[p].flags == LUMP_PRBOOM)
		{
			return p;
		}
	}
	return LUMP_NOT_FOUND;
}

// W_ListNumFromName
// calls W_FindNumFromName and returns the lumps in ascending order
//
int W_ListNumFromName(const char* name, int lump)
{
	int i, next;

	for(i = LUMP_NOT_FOUND; (next = W_FindNumFromName(name, i)) != LUMP_NOT_FOUND; i = next)
		if(next == lump)
			break;

	return i;
}

// W_Init
// Loads each of the files in the wadfiles array.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
// CPhipps - modified to use the new wadfiles array
//
wadfile_info_t* wadfiles = nullptr;

size_t numwadfiles = 0; // CPhipps - size of the wadfiles array (dynamic, no limit)

void W_Init()
{
	// CPhipps - start with nothing

	numlumps = 0;
	lumpinfo = nullptr;

	{
		// CPhipps - new wadfiles array used
		// open all the files, load headers, and count lumps
		int i;
		for(i = 0; (size_t)i < numwadfiles; i++)
			W_AddFile(&wadfiles[i]);
	}

	if(!numlumps)
	{
		if(!MainLumpCache) return;
		I_Error("W_Init: No files found");
	}

	//jff 1/23/98
	// get all the sprites and flats into one marked block each
	// killough 1/24/98: change interface to use M_START/M_END explicitly
	// killough 4/17/98: Add namespace tags to each entry
	// killough 4/4/98: add colormap markers
	W_CoalesceMarkedResource("S_START", "S_END", LumpNamespace::Sprites);
	W_CoalesceMarkedResource("F_START", "F_END", LumpNamespace::Flats);
	W_CoalesceMarkedResource("C_START", "C_END", LumpNamespace::Colormaps);
	W_CoalesceMarkedResource("B_START", "B_END", LumpNamespace::Prboom);
	W_CoalesceMarkedResource("HI_START", "HI_END", LumpNamespace::Hires);

	// killough 1/31/98: initialize lump hash table
	W_HashLumps();

	/* cph 2001/07/07 - separated cache setup */
	lprintf(OutputLevels::Debug, "W_InitCache\n");
	W_InitCache();

	V_FreePlaypal();
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(int lump)
{
	if(lump >= numlumps)
		I_Error("W_LumpLength: %i >= numlumps", lump);
	return lumpinfo[lump].size;
}

int W_SafeLumpLength(int lump)
{
	return W_LumpNumExists(lump) ? lumpinfo[lump].size : 0;
}

const char* W_LumpName(int lump)
{
	return W_LumpNumExists(lump) ? lumpinfo[lump].name : nullptr;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//

void W_ReadLump(int lump, void* dest)
{
	lumpinfo_t* l = lumpinfo + lump;

#ifdef RANGECHECK
	if(lump >= numlumps)
		I_Error("W_ReadLump: %i >= numlumps", lump);
#endif

	{
		if(l->wadfile)
		{
			lseek(l->wadfile->handle, l->position, SEEK_SET);
			I_Read(l->wadfile->handle, dest, l->size);
		}
	}
}

char* W_ReadLumpToString(int lump)
{
	char* buffer = nullptr;
	lumpinfo_t* l = lumpinfo + lump;

	if(lump >= 0 && lump < numlumps && l->wadfile)
	{
		buffer = static_cast<char*>(Z_Malloc(l->size + 1));
		lseek(l->wadfile->handle, l->position, SEEK_SET);
		I_Read(l->wadfile->handle, buffer, l->size);
		buffer[l->size] = '\0';
	}

	return buffer;
}

int W_LumpNumInPortWad(int lump)
{
	const lumpinfo_t* info;
	size_t name_length, default_name_length;

	info = W_GetLumpInfoByNum(lump);
	name_length = strlen(info->wadfile->name);
	default_name_length = strlen(WAD_DATA);

	return name_length >= default_name_length &&
		!strcmp(info->wadfile->name + name_length - default_name_length, WAD_DATA);
}

const void* W_SafeLumpByNum(int lump)
{
	return W_LumpNumExists(lump) ? W_LumpByNum(lump) : nullptr;
}

int W_GetAnimatedOrSwitchesLump(const char* lumpname)
{
	// Normal lookup - PWAD > port wad > IWAD
	int lump = W_CheckNumForName(lumpname);

	// If not PWAD lump, check if IWAD lump exists
	if(W_LumpNumExists(lump) && W_LumpNumInPortWad(lump))
	{
		int iwad_lump = LUMP_NOT_FOUND;

		for(iwad_lump = 0; iwad_lump < numlumps; iwad_lump++)
		{
			if(!strncmp(lumpinfo[iwad_lump].name, lumpname, 8) &&
				lumpinfo[iwad_lump].li_namespace == LumpNamespace::Global &&
				lumpinfo[iwad_lump].source == WadSource::Iwad)
				return iwad_lump;
		}
	}

	// PWAD or port wad lump
	return lump;
}

int W_LumpNumExists(int lump)
{
	return lump != LUMP_NOT_FOUND && lump < numlumps;
}

int W_PWADLumpNumExists(int lump)
{
	return W_LumpNumExists(lump) && (lumpinfo[lump].source == WadSource::Pwad || lumpinfo[lump].source == WadSource::PwadAutoLoad);
}

int W_AUTOLumpNumExists(int lump)
{
	return W_LumpNumExists(lump) && (lumpinfo[lump].source == WadSource::AutoLoad);
}

int W_PWADLumpNumExists2(int lump)
{
	return (W_AUTOLumpNumExists(lump) || W_PWADLumpNumExists(lump));
}

int W_LumpNameExists(const char* name)
{
	return W_CheckNumForName(name) != LUMP_NOT_FOUND;
}

// The part of `name` the `const char*` lookups read: at most 8 characters, up to the first zero, terminated.
static std::array<char, 9> W_TerminatedLumpName(const std::string_view name)
{
	std::array<char, 9> terminated {};
	name.substr(0, 8).copy(terminated.data(), 8);
	return terminated;
}

int W_FindNumFromName2(const std::string_view name, const LumpNamespace ns, const int lump)
{
	return W_FindNumFromName2(W_TerminatedLumpName(name).data(), ns, lump);
}

int W_GetNumForName(const std::string_view name)
{
	return W_GetNumForName(W_TerminatedLumpName(name).data());
}

int W_LumpNameExists(const std::string_view name)
{
	return W_LumpNameExists(W_TerminatedLumpName(name).data());
}

int W_LumpNameExists2(const char* name, LumpNamespace ns)
{
	return W_CheckNumForName2(name, ns) != LUMP_NOT_FOUND;
}

int W_PWADLumpNameExists(const char* name)
{
	return W_PWADLumpNumExists(W_CheckNumForName(name));
}

int W_AUTOLumpNameExists(const char* name)
{
	return W_AUTOLumpNumExists(W_CheckNumForName(name));
}

int W_PWADLumpNameExists2(const char* name)
{
	return (W_AUTOLumpNumExists(W_CheckNumForName(name)) || W_PWADLumpNumExists(W_CheckNumForName(name)));
}

int W_PWADMapsExist()
{
	return W_PWADLumpNameExists2("THINGS") || W_PWADLumpNameExists2("TEXTMAP");
}

void W_Shutdown()
{
	int i;

	W_DoneCache();

	for(i = 0; i < numwadfiles; ++i)
	{
		if(wadfiles[i].handle > 0)
		{
			close(wadfiles[i].handle);
			wadfiles[i].handle = -1;
		}
	}
}

void dsda_ResetInitLumpCache()
{
	W_Shutdown();

	wadfiles = nullptr;
	numwadfiles = 0;
	lumpinfo = nullptr;
	numlumps = 0;
}
