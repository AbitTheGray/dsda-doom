// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      WAD I/O functions.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

//
// TYPES
//

typedef struct
{
	char identification[4]; // Should be "IWAD" or "PWAD".
	int numlumps;
	int infotableofs;
} wadinfo_t;

typedef struct
{
	int filepos;
	int size;
	char name[8];
} filelump_t;

#define LUMP_NOT_FOUND -1

//
// WADFILE I/O related stuff.
//

// CPhipps - defined enum in wider scope
// Ty 08/29/98 - add source field to identify where this lump came from
typedef enum
{
	source_skip = -1,
	source_iwad = 0,       // iwad file load
	source_port_wad,       // predefined lump
	source_auto_load,      // lump auto-loaded by config file
	source_pwad_auto_load, // pwad dir auto-load
	source_pwad,           // pwad file load
	source_lmp,            // lmp file load
	source_net,            // CPhipps

	//e6y
	//  source_deh_auto_load,
	source_deh,
	source_err
} wad_source_t;

// CPhipps - changed wad init
// We _must_ have the wadfiles[] the same as those actually loaded, so there
// is no point having these separate entities. This belongs here.
typedef struct
{
	char* name;
	wad_source_t src;
	int handle;
} wadfile_info_t;

extern wadfile_info_t* wadfiles;

extern size_t numwadfiles; // CPhipps - size of the wadfiles array

extern int MainLumpCache;

void W_Init(); // CPhipps - uses the above array
void W_InitCache();
void W_DoneCache();
void W_Shutdown();
void dsda_ResetInitLumpCache();

typedef enum
{
	ns_global = 0,
	ns_sprites,
	ns_flats,
	ns_colormaps,
	ns_prboom,
	ns_demos,
	ns_hires,
} li_namespace_e; // haleyjd 05/21/02: renamed from "namespace"

typedef struct
{
	// WARNING: order of some fields important (see info.c).

	char name[9];
	int size;

	// killough 1/31/98: hash table fields, used for ultra-fast hash table lookup
	int index, next;

	// killough 4/17/98: namespace tags, to prevent conflicts between resources
	li_namespace_e li_namespace; // haleyjd 05/21/02: renamed from "namespace"

	wadfile_info_t* wadfile;
	int position;
	wad_source_t source;
	int flags; //e6y
} lumpinfo_t;

// e6y: lump flags
#define LUMP_STATIC 0x00000001 /* assigned gltexture should be static */
#define LUMP_PRBOOM 0x00000002 /* from internal resource */

extern lumpinfo_t* lumpinfo;
extern int numlumps;

int W_FindNumFromName2(const char* name, int ns, int lump);

static inline
int W_FindNumFromName(const char* name, int lump)
{
	return W_FindNumFromName2(name, ns_global, lump);
}

static inline
int W_CheckNumForName2(const char* name, int ns)
{
	return W_FindNumFromName2(name, ns, LUMP_NOT_FOUND);
}

static inline
int W_CheckNumForName(const char* name)
{
	return W_CheckNumForName2(name, ns_global);
}

int W_CheckNumForNameInternal(const char* name);
int W_ListNumFromName(const char* name, int lump);
int W_GetNumForName(const char* name);
const lumpinfo_t* W_GetLumpInfoByNum(int lump);
int W_LumpLength(int lump);
int W_SafeLumpLength(int lump);
const char* W_LumpName(int lump);
void W_ReadLump(int lump, void* dest);
char* W_ReadLumpToString(int lump);
// CPhipps - modified for 'new' lump locking
const void* W_SafeLumpByNum(int lump);
const void* W_LumpByNum(int lump);
const void* W_LockLumpNum(int lump);
void* W_GetModifiableLumpData(int lump);

int W_LumpNumExists(int lump);
int W_LumpNameExists(const char* name);
int W_LumpNameExists2(const char* name, int ns);
int W_PWADLumpNumExists(int lump);
int W_PWADLumpNameExists(const char* name);
int W_AUTOLumpNumExists(int lump);
int W_AUTOLumpNameExists(const char* name);
int W_PWADLumpNumExists2(int lump);
int W_PWADLumpNameExists2(const char* name);
int W_PWADMapsExist();

int W_GetAnimatedOrSwitchesLump(const char* lumpname);

// CPhipps - convenience macros
//#define W_LumpByNum(num) (W_LumpByNum)((num),1)
#define W_LumpByName(name) W_LumpByNum (W_GetNumForName(name))

char* AddDefaultExtension(char*, const char*); // killough 1/18/98
void ExtractFileBase(const char*, char*);      // killough
unsigned W_LumpNameHash(const char* s);        // killough 1/31/98
void W_HashLumps();                        // cph 2001/07/07 - made public
int W_LumpNumInPortWad(int lump);

#ifdef __cplusplus
}
#endif
