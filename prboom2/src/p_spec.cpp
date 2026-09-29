// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   -Loads and initializes texture and flat animation sequences
 *   -Implements utility functions for all linedef/sector special handlers
 *   -Dispatches walkover and gun line triggers
 *   -Initializes and implements special sector types
 *   -Implements donut linedef triggers
 *   -Initializes and implements BOOM linedef triggers for
 *     Scrollers/Conveyors
 *     Friction
 *     Wind/Current
 */

#include <array>
#include <utility>

#include "doomstat.hpp"
#include "p_spec.hpp"
#include "p_tick.hpp"
#include "p_setup.hpp"
#include "m_random.hpp"
#include "d_englsh.hpp"
#include "w_wad.hpp"
#include "r_main.hpp"
#include "p_maputl.hpp"
#include "p_map.hpp"
#include "p_user.hpp"
#include "g_game.hpp"
#include "p_inter.hpp"
#include "p_enemy.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "i_sound.hpp"
#include "m_bbox.hpp"                                         // phares 3/20/98
#include "d_deh.hpp"
#include "r_plane.hpp"
#include "hu_stuff.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"//e6y

#include "dsda.hpp"
#include "dsda/args.hpp"
#include "dsda/configuration.hpp"
#include "dsda/global.hpp"
#include "dsda/id_list.hpp"
#include "dsda/line_special.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/messenger.hpp"
#include "dsda/scroll.hpp"
#include "dsda/thing_id.hpp"
#include "dsda/utility.hpp"

//
//      source animation definition
//
//
#ifdef _MSC_VER // proff: This is the same as __attribute__ ((packed)) in GNUC
#pragma pack(push)
#pragma pack(1)
#endif //_MSC_VER

#if defined(__MWERKS__)
#pragma options align=packed
#endif

typedef struct
{
	signed char istexture; //jff 3/23/98 make char for comparison // cph - make signed
	char endname[9];       //  if false, it is a flat
	char startname[9];
	int speed;
}
	PACKEDATTR animdef_t; //jff 3/23/98 pack to read from memory

#if defined(__MWERKS__)
#pragma options align=reset
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif //_MSC_VER

#define MAXANIMS 32                   // no longer a strict limit -- killough

static anim_t* lastanim;
static anim_t* anims; // new structure w/o limits -- killough
static size_t maxanims;

// killough 3/7/98: Initialize generalized scrolling
static void P_SpawnScrollers();

static void P_SpawnFriction(); // phares 3/16/98
static void P_SpawnPushers();  // phares 3/20/98

static const animdef_t heretic_animdefs[] = {
	// false = flat
	// true = texture
	{false, "FLTWAWA3", "FLTWAWA1", 8}, // Water
	{false, "FLTSLUD3", "FLTSLUD1", 8}, // Sludge
	{false, "FLTTELE4", "FLTTELE1", 6}, // Teleport
	{false, "FLTFLWW3", "FLTFLWW1", 9}, // River - West
	{false, "FLTLAVA4", "FLTLAVA1", 8}, // Lava
	{false, "FLATHUH4", "FLATHUH1", 8}, // Super Lava
	{true, "LAVAFL3", "LAVAFL1", 6},    // Texture: Lavaflow
	{true, "WATRWAL3", "WATRWAL1", 4},  // Texture: Waterfall
	{-1}
};

// heretic
#define MAXLINEANIMS 64*256
short numlinespecials;
line_t* linespeciallist[MAXLINEANIMS];

//e6y
void MarkAnimatedTextures()
{
	anim_t* anim;

	anim_textures = static_cast<TAnimItemParam*>(Z_Calloc(numtextures, sizeof(TAnimItemParam)));
	anim_flats = static_cast<TAnimItemParam*>(Z_Calloc(numflats, sizeof(TAnimItemParam)));

	for(anim = anims; anim < lastanim; anim++)
	{
		int i;
		for(i = 0; i < anim->numpics; i++)
		{
			if(anim->istexture)
			{
				anim_textures[anim->basepic + i].anim = anim;
				anim_textures[anim->basepic + i].index = i + 1;
			}
			else
			{
				anim_flats[anim->basepic + i].anim = anim;
				anim_flats[anim->basepic + i].index = i + 1;
			}
		}
	}
}

//
// P_InitPicAnims
//
// Load the table of animation definitions, checking for existence of
// the start and end of each frame. If the start doesn't exist the sequence
// is skipped, if the last doesn't exist, BOOM exits.
//
// Wall/Flat animation sequences, defined by name of first and last frame,
// The full animation sequence is given using all lumps between the start
// and end entry, in the order found in the WAD file.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called ANIMATED rather than a static table in this module to
// allow wad designers to insert or modify animation sequences.
//
// Lump format is an array of byte packed animdef_t structures, terminated
// by a structure with istexture == -1. The lump can be generated from a
// text source file using SWANTBLS.EXE, distributed with the BOOM utils.
// The standard list of switches and animations is contained in the example
// source text file DEFSWANI.DAT also in the BOOM util distribution.
//
//
void P_InitPicAnims()
{
	int i;
	const animdef_t* animdefs; //jff 3/23/98 pointer to animation lump
	int lump = LUMP_NOT_FOUND;
	//  Init animation

	if(map_format.animdefs)
	{
		MarkAnimatedTextures(); //e6y
		return;
	}

	if(heretic)
	{
		lump = W_GetAnimatedOrSwitchesLump("ANIMATED");

		// Heretic keeps using built-in animdefs unless an IWAD/PWAD lump exists
		if(W_LumpNumExists(lump) && !W_LumpNumInPortWad(lump))
			animdefs = (const animdef_t*)W_LumpByNum(lump);
		else
			animdefs = heretic_animdefs;
	}
	else
	{
		lump = W_GetAnimatedOrSwitchesLump("ANIMATED"); // cph - new wad lump handling
		//jff 3/23/98 read from predefined or wad lump instead of table
		animdefs = (const animdef_t*)W_LumpByNum(lump);
	}

	lastanim = anims;
	for(i = 0; animdefs[i].istexture != LUMP_NOT_FOUND; i++)
	{
		// 1/11/98 killough -- removed limit by array-doubling
		if(lastanim >= anims + maxanims)
		{
			size_t newmax = maxanims ? maxanims * 2 : MAXANIMS;
			anims = static_cast<anim_t*>(Z_Realloc(anims, newmax * sizeof(*anims))); // killough
			lastanim = anims + maxanims;
			maxanims = newmax;
		}

		if(animdefs[i].istexture)
		{
			// different episode ?
			if(R_CheckTextureNumForName(animdefs[i].startname) == LUMP_NOT_FOUND)
				continue;

			lastanim->picnum = R_TextureNumForName(animdefs[i].endname);
			lastanim->basepic = R_TextureNumForName(animdefs[i].startname);
		}
		else
		{
			if(!W_LumpNameExists2(animdefs[i].startname, LumpNamespace::Flats)) // killough 4/17/98
				continue;

			lastanim->picnum = R_FlatNumForName(animdefs[i].endname);
			lastanim->basepic = R_FlatNumForName(animdefs[i].startname);
		}

		lastanim->istexture = animdefs[i].istexture;
		lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;
		lastanim->speed = LittleLong(animdefs[i].speed);

		// [crispy] skip reading SMMU swirling flats
		if(lastanim->speed < 65536 && lastanim->numpics != 1)
		{
			if(lastanim->numpics < 2)
				Log::Fatal("P_InitPicAnims: bad cycle from {} to {}",
					std::string_view(animdefs[i].startname),
					std::string_view(animdefs[i].endname));
		}

		if(lastanim->speed == 0)
			Log::Fatal("P_InitPicAnims: {} to {} animation cannot have speed 0",
				std::string_view(animdefs[i].startname),
				std::string_view(animdefs[i].endname));

		lastanim++;
	}

	MarkAnimatedTextures(); //e6y
}

///////////////////////////////////////////////////////////////
//
// Linedef and Sector Special Implementation Utility Functions
//
///////////////////////////////////////////////////////////////

//
// getSide()
//
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
// Note: if side=1 is specified, it must exist or results undefined
//
side_t* getSide
(int currentSector,
	int line,
	int side)
{
	return &sides[(sectors[currentSector].lines[line])->sidenum[side]];
}


//
// getSector()
//
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
// Note: if side=1 is specified, it must exist or results undefined
//
sector_t* getSector
(int currentSector,
	int line,
	int side)
{
	return sides[(sectors[currentSector].lines[line])->sidenum[side]].sector;
}


//
// twoSided()
//
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
// modified to return actual two-sidedness rather than presence
// of 2S flag unless compatibility optioned
//
int twoSided
(int sector,
	int line)
{
	//jff 1/26/98 return what is actually needed, whether the line
	//has two sidedefs, rather than whether the 2S flag is set

	return (comp[std::to_underlying(CompOption::Model)])
		? ((sectors[sector].lines[line])->flags & LineFlag::TwoSided) != LineFlag{}
		: (sectors[sector].lines[line])->sidenum[1] != NO_INDEX;
}


//
// getNextSector()
//
// Return sector_t * of sector next to current across line.
//
// Note: returns NULL if not two-sided line, or both sides refer to sector
//
sector_t* getNextSector
(line_t* line,
	sector_t* sec)
{
	//jff 1/26/98 check unneeded since line->backsector already
	//returns NULL if the line is not two sided, and does so from
	//the actual two-sidedness of the line, rather than its 2S flag

	if(comp[std::to_underlying(CompOption::Model)])
	{
		if((line->flags & LineFlag::TwoSided) == LineFlag{})
			return nullptr;
	}

	if(line->frontsector == sec)
	{
		if(comp[std::to_underlying(CompOption::Model)] || line->backsector != sec)
			return line->backsector; //jff 5/3/98 don't retn sec unless compatibility
		else                         // fixes an intra-sector line breaking functions
			return nullptr;             // like floor->highest floor
	}
	return line->frontsector;
}


//
// P_FindLowestFloorSurrounding()
//
// Returns the fixed point value of the lowest floor height
// in the sector passed or its surrounding sectors.
//
fixed_t P_FindLowestFloorSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t floor = sec->floorheight;

	for(i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->floorheight < floor)
			floor = other->floorheight;
	}
	return floor;
}


//
// P_FindHighestFloorSurrounding()
//
// Passed a sector, returns the fixed point value of the largest
// floor height in the surrounding sectors, not including that passed
//
// NOTE: if no surrounding sector exists -32000*FRACUINT is returned
//       if compatibility then -500*FRACUNIT is the smallest return possible
//
fixed_t P_FindHighestFloorSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t floor = -500 * FRACUNIT;

	//jff 1/26/98 Fix initial value for floor to not act differently
	//in sections of wad that are below -500 units
	if(!comp[std::to_underlying(CompOption::Model)])          /* jff 3/12/98 avoid ovf */
		floor = -32000 * FRACUNIT; // in height calculations

	for(i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->floorheight > floor)
			floor = other->floorheight;
	}
	return floor;
}


//
// P_FindNextHighestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the smallest floor height in a surrounding sector larger than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
// Rewritten by Lee Killough to avoid fixed array and to be faster
//
fixed_t P_FindNextHighestFloor(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;

	// e6y
	// Original P_FindNextHighestFloor() is restored for demo_compatibility
	// Adapted for prboom's complevels
	if(demo_compatibility && !prboom_comp[std::to_underlying(PrboomComp::ForceBoomFindnexthighestfloor)].state)
	{
		int h;
		int min;
		static int MAX_ADJOINING_SECTORS = 0;
		static fixed_t* heightlist = nullptr;
		static int heightlist_size = 0;
		line_t* check;
		fixed_t height = currentheight;
		static fixed_t last_height_0 = 0;

		// 20 adjoining sectors max!
		if(!MAX_ADJOINING_SECTORS)
			MAX_ADJOINING_SECTORS = dsda_Flag(ArgId::Doom95) ? 500 : 20;

		if(sec->linecount > heightlist_size)
		{
			do
			{
				heightlist_size = heightlist_size ? heightlist_size * 2 : 128;
			}
			while(sec->linecount > heightlist_size);
			heightlist = static_cast<fixed_t*>(Z_Realloc(heightlist, heightlist_size * sizeof(heightlist[0])));
		}

		for(i = 0, h = 0; i < sec->linecount; i++)
		{
			check = sec->lines[i];
			other = getNextSector(check, sec);

			if(!other)
				continue;

			if(other->floorheight > height)
			{
				// e6y
				// Emulation of stack overflow.
				// 20: overflow affects nothing - just a luck;
				// 21: can be emulated;
				// 22..26: overflow affects saved registers - unpredictable behaviour, can crash;
				// 27: overflow affects return address - crash with high probability;
				if(compatibility_level < CompLevel::Dosdoom && h >= MAX_ADJOINING_SECTORS)
				{
					if(h == MAX_ADJOINING_SECTORS + 1)
						height = other->floorheight;

					// 20 & 21 are common and not "warning" worthy
					if(h > MAX_ADJOINING_SECTORS + 1)
					{
						Log::Warn("P_FindNextHighestFloor: Overflow of heightlist[{}] array is detected.\n", MAX_ADJOINING_SECTORS);
						Log::Warn(" Sector {}, line {}, heightlist index {}: ", sec->iSectorID, sec->lines[i]->iLineID, h);

						if(h <= MAX_ADJOINING_SECTORS + 6)
							Log::Warn("cannot be emulated - unpredictable behaviour.\n");
						else
							Log::Warn("cannot be emulated - crash with high probability.\n");
					}
				}
				heightlist[h++] = other->floorheight;
			}

			// Check for overflow. Warning.
			if(compatibility_level >= CompLevel::Dosdoom && h >= MAX_ADJOINING_SECTORS)
			{
				Log::Warn("Sector with more than 20 adjoining sectors\n");
				break;
			}
		}

		// Find lowest height in list
		if(!h)
		{
			// cph - my guess at doom v1.2 - 1.4beta compatibility here.
			// If there are no higher neighbouring sectors, Heretic just returned
			// heightlist[0] (local variable), i.e. noise off the stack. 0 is right for
			// RETURN01 E1M2, so let's take that.
			//
			// SmileTheory's response:
			// It's not *quite* random stack noise. If this function is called
			// as part of a loop, heightlist will be at the same location as in
			// the previous call. Doing it this way fixes 1_ON_1.WAD.
			return (compatibility_level < CompLevel::Doom1666 ? last_height_0 : currentheight);
		}

		last_height_0 = heightlist[0];
		min = heightlist[0];

		// Range checking?
		for(i = 1; i < h; i++)
		{
			if(heightlist[i] < min)
				min = heightlist[i];
		}

		return min;
	}


	for(i = 0; i < sec->linecount; i++)
		if((other = getNextSector(sec->lines[i], sec)) &&
			other->floorheight > currentheight)
		{
			int height = other->floorheight;
			while(++i < sec->linecount)
				if((other = getNextSector(sec->lines[i], sec)) &&
					other->floorheight < height &&
					other->floorheight > currentheight)
					height = other->floorheight;
			return height;
		}
	/* cph - my guess at doom v1.2 - 1.4beta compatibility here.
	* If there are no higher neighbouring sectors, Heretic just returned
	* heightlist[0] (local variable), i.e. noise off the stack. 0 is right for
	* RETURN01 E1M2, so let's take that. */
	return (compatibility_level < CompLevel::Doom1666 ? 0 : currentheight);
}


//
// P_FindNextLowestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the largest floor height in a surrounding sector smaller than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextLowestFloor(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;

	for(i = 0; i < sec->linecount; i++)
		if((other = getNextSector(sec->lines[i], sec)) &&
			other->floorheight < currentheight)
		{
			int height = other->floorheight;
			while(++i < sec->linecount)
				if((other = getNextSector(sec->lines[i], sec)) &&
					other->floorheight > height &&
					other->floorheight < currentheight)
					height = other->floorheight;
			return height;
		}
	return currentheight;
}


//
// P_FindNextLowestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the largest ceiling height in a surrounding sector smaller than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextLowestCeiling(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;

	for(i = 0; i < sec->linecount; i++)
		if((other = getNextSector(sec->lines[i], sec)) &&
			other->ceilingheight < currentheight)
		{
			int height = other->ceilingheight;
			while(++i < sec->linecount)
				if((other = getNextSector(sec->lines[i], sec)) &&
					other->ceilingheight > height &&
					other->ceilingheight < currentheight)
					height = other->ceilingheight;
			return height;
		}
	return currentheight;
}


//
// P_FindNextHighestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the smallest ceiling height in a surrounding sector larger than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
// jff 02/03/98 Twiddled Lee's P_FindNextHighestFloor to make this
//
fixed_t P_FindNextHighestCeiling(sector_t* sec, int currentheight)
{
	sector_t* other;
	int i;

	for(i = 0; i < sec->linecount; i++)
		if((other = getNextSector(sec->lines[i], sec)) &&
			other->ceilingheight > currentheight)
		{
			int height = other->ceilingheight;
			while(++i < sec->linecount)
				if((other = getNextSector(sec->lines[i], sec)) &&
					other->ceilingheight < height &&
					other->ceilingheight > currentheight)
					height = other->ceilingheight;
			return height;
		}
	return currentheight;
}


//
// P_FindLowestCeilingSurrounding()
//
// Passed a sector, returns the fixed point value of the smallest
// ceiling height in the surrounding sectors, not including that passed
//
// NOTE: if no surrounding sector exists 32000*FRACUINT is returned
//       but if compatibility then INT_MAX is the return
//
fixed_t P_FindLowestCeilingSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t height = INT_MAX;

	/* jff 3/12/98 avoid ovf in height calculations */
	if(!comp[std::to_underlying(CompOption::Model)]) height = 32000 * FRACUNIT;

	for(i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->ceilingheight < height)
			height = other->ceilingheight;
	}
	return height;
}


//
// P_FindHighestCeilingSurrounding()
//
// Passed a sector, returns the fixed point value of the largest
// ceiling height in the surrounding sectors, not including that passed
//
// NOTE: if no surrounding sector exists -32000*FRACUINT is returned
//       but if compatibility then 0 is the smallest return possible
//
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec)
{
	int i;
	line_t* check;
	sector_t* other;
	fixed_t height = 0;

	/* jff 1/26/98 Fix initial value for floor to not act differently
	* in sections of wad that are below 0 units
	* jff 3/12/98 avoid ovf in height calculations */
	if(!comp[std::to_underlying(CompOption::Model)]) height = -32000 * FRACUNIT;

	for(i = 0; i < sec->linecount; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check, sec);

		if(!other)
			continue;

		if(other->ceilingheight > height)
			height = other->ceilingheight;
	}
	return height;
}


//
// P_FindShortestTextureAround()
//
// Passed a sector number, returns the shortest lower texture on a
// linedef bounding the sector.
//
// Note: If no lower texture exists 32000*FRACUNIT is returned.
//       but if compatibility then INT_MAX is returned
//
// jff 02/03/98 Add routine to find shortest lower texture
//
fixed_t P_FindShortestTextureAround(int secnum)
{
	int minsize = INT_MAX;
	side_t* side;
	int i;
	sector_t* sec = &sectors[secnum];

	if(!comp[std::to_underlying(CompOption::Model)])
		minsize = 32000 << FRACBITS; //jff 3/13/98 prevent overflow in height calcs

	for(i = 0; i < sec->linecount; i++)
	{
		if(twoSided(secnum, i))
		{
			side = getSide(secnum, i, 0);
			if(side->bottomtexture > 0) //jff 8/14/98 texture 0 is a placeholder
				if(textureheight[side->bottomtexture] < minsize)
					minsize = textureheight[side->bottomtexture];
			side = getSide(secnum, i, 1);
			if(side->bottomtexture > 0) //jff 8/14/98 texture 0 is a placeholder
				if(textureheight[side->bottomtexture] < minsize)
					minsize = textureheight[side->bottomtexture];
		}
	}
	return minsize;
}


//
// P_FindShortestUpperAround()
//
// Passed a sector number, returns the shortest upper texture on a
// linedef bounding the sector.
//
// Note: If no upper texture exists 32000*FRACUNIT is returned.
//       but if compatibility then INT_MAX is returned
//
// jff 03/20/98 Add routine to find shortest upper texture
//
fixed_t P_FindShortestUpperAround(int secnum)
{
	int minsize = INT_MAX;
	side_t* side;
	int i;
	sector_t* sec = &sectors[secnum];

	if(!comp[std::to_underlying(CompOption::Model)])
		minsize = 32000 << FRACBITS; //jff 3/13/98 prevent overflow
	// in height calcs
	for(i = 0; i < sec->linecount; i++)
	{
		if(twoSided(secnum, i))
		{
			side = getSide(secnum, i, 0);
			if(side->toptexture > 0) //jff 8/14/98 texture 0 is a placeholder
				if(textureheight[side->toptexture] < minsize)
					minsize = textureheight[side->toptexture];
			side = getSide(secnum, i, 1);
			if(side->toptexture > 0) //jff 8/14/98 texture 0 is a placeholder
				if(textureheight[side->toptexture] < minsize)
					minsize = textureheight[side->toptexture];
		}
	}
	return minsize;
}


//
// P_FindModelFloorSector()
//
// Passed a floor height and a sector number, return a pointer to a
// a sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model floor
//  around a sector specified by sector number
// jff 3/14/98 change first parameter to plain height to allow call
//  from routine not using floormove_t
//
sector_t* P_FindModelFloorSector(fixed_t floordestheight, int secnum)
{
	int i;
	sector_t* sec = nullptr;
	int linecount;

	sec = &sectors[secnum]; //jff 3/2/98 woops! better do this
	//jff 5/23/98 don't disturb sec->linecount while searching
	// but allow early exit in old demos
	linecount = sec->linecount;
	for(i = 0; i < (demo_compatibility && sec->linecount < linecount ? sec->linecount : linecount); i++)
	{
		if(twoSided(secnum, i))
		{
			if(getSide(secnum, i, 0)->sector->iSectorID == secnum)
				sec = getSector(secnum, i, 1);
			else
				sec = getSector(secnum, i, 0);

			if(heretic || sec->floorheight == floordestheight)
				return sec;
		}
	}
	return nullptr;
}


//
// P_FindModelCeilingSector()
//
// Passed a ceiling height and a sector number, return a pointer to a
// a sector with that ceiling height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
// jff 02/03/98 Add routine to find numeric model ceiling
//  around a sector specified by sector number
//  used only from generalized ceiling types
// jff 3/14/98 change first parameter to plain height to allow call
//  from routine not using ceiling_t
//
sector_t* P_FindModelCeilingSector(fixed_t ceildestheight, int secnum)
{
	int i;
	sector_t* sec = nullptr;
	int linecount;

	sec = &sectors[secnum]; //jff 3/2/98 woops! better do this
	//jff 5/23/98 don't disturb sec->linecount while searching
	// but allow early exit in old demos
	linecount = sec->linecount;
	for(i = 0; i < (demo_compatibility && sec->linecount < linecount ? sec->linecount : linecount); i++)
	{
		if(twoSided(secnum, i))
		{
			if(getSide(secnum, i, 0)->sector->iSectorID == secnum)
				sec = getSector(secnum, i, 1);
			else
				sec = getSector(secnum, i, 0);

			if(sec->ceilingheight == ceildestheight)
				return sec;
		}
	}
	return nullptr;
}

// Converts Hexen's 0 (meaning no crush) to the internal value
int P_ConvertHexenCrush(int crush)
{
	return (crush ? crush : NO_CRUSH);
}

//
// P_FindMinSurroundingLight()
//
// Passed a sector and a light level, returns the smallest light level
// in a surrounding sector less than that passed. If no smaller light
// level exists, the light level passed is returned.
//
int P_FindMinSurroundingLight
(sector_t* sector,
	int max)
{
	int i;
	int min;
	line_t* line;
	sector_t* check;

	min = max;
	for(i = 0; i < sector->linecount; i++)
	{
		line = sector->lines[i];
		check = getNextSector(line, sector);

		if(!check)
			continue;

		if(check->lightlevel < min)
			min = check->lightlevel;
	}
	return min;
}


//
// P_CanUnlockGenDoor()
//
// Passed a generalized locked door linedef and a player, returns whether
// the player has the keys necessary to unlock that door.
//
// Note: The linedef passed MUST be a generalized locked door type
//       or results are undefined.
//
// jff 02/05/98 routine added to test for unlockability of
//  generalized locked doors
//
dboolean P_CanUnlockGenDoor
(line_t* line,
	player_t* player)
{
	// does this line special distinguish between skulls and keys?
	int skulliscard = (line->special & LockedNKeys) >> LockedNKeysShift;

	// determine for each case of lock type if player's keys are adequate
	switch(static_cast<KeyKind>((line->special & LockedKey) >> LockedKeyShift))
	{
		case KeyKind::Any:
			if
			(
				!player->cards[std::to_underlying(Card::RedCard)] &&
				!player->cards[std::to_underlying(Card::RedSkull)] &&
				!player->cards[std::to_underlying(Card::BlueCard)] &&
				!player->cards[std::to_underlying(Card::BlueSkull)] &&
				!player->cards[std::to_underlying(Card::YellowCard)] &&
				!player->cards[std::to_underlying(Card::YellowSkull)]
			)
			{
				dsda_AddPlayerMessage(s_PD_ANY, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::RedCard:
			if
			(
				!player->cards[std::to_underlying(Card::RedCard)] &&
				(!skulliscard || !player->cards[std::to_underlying(Card::RedSkull)])
			)
			{
				dsda_AddPlayerMessage(skulliscard ? s_PD_REDK : s_PD_REDC, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::BlueCard:
			if
			(
				!player->cards[std::to_underlying(Card::BlueCard)] &&
				(!skulliscard || !player->cards[std::to_underlying(Card::BlueSkull)])
			)
			{
				dsda_AddPlayerMessage(skulliscard ? s_PD_BLUEK : s_PD_BLUEC, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::YellowCard:
			if
			(
				!player->cards[std::to_underlying(Card::YellowCard)] &&
				(!skulliscard || !player->cards[std::to_underlying(Card::YellowSkull)])
			)
			{
				dsda_AddPlayerMessage(skulliscard ? s_PD_YELLOWK : s_PD_YELLOWC, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::RedSkull:
			if
			(
				!player->cards[std::to_underlying(Card::RedSkull)] &&
				(!skulliscard || !player->cards[std::to_underlying(Card::RedCard)])
			)
			{
				dsda_AddPlayerMessage(skulliscard ? s_PD_REDK : s_PD_REDS, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::BlueSkull:
			if
			(
				!player->cards[std::to_underlying(Card::BlueSkull)] &&
				(!skulliscard || !player->cards[std::to_underlying(Card::BlueCard)])
			)
			{
				dsda_AddPlayerMessage(skulliscard ? s_PD_BLUEK : s_PD_BLUES, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::YellowSkull:
			if
			(
				!player->cards[std::to_underlying(Card::YellowSkull)] &&
				(!skulliscard || !player->cards[std::to_underlying(Card::YellowCard)])
			)
			{
				dsda_AddPlayerMessage(skulliscard ? s_PD_YELLOWK : s_PD_YELLOWS, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
		case KeyKind::All:
			if
			(
				!skulliscard &&
				(
					!player->cards[std::to_underlying(Card::RedCard)] ||
					!player->cards[std::to_underlying(Card::RedSkull)] ||
					!player->cards[std::to_underlying(Card::BlueCard)] ||
					!player->cards[std::to_underlying(Card::BlueSkull)] ||
					!player->cards[std::to_underlying(Card::YellowCard)] ||
					!player->cards[std::to_underlying(Card::YellowSkull)]
				)
			)
			{
				dsda_AddPlayerMessage(s_PD_ALL6, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			if
			(
				skulliscard &&
				(
					(!player->cards[std::to_underlying(Card::RedCard)] &&
						!player->cards[std::to_underlying(Card::RedSkull)]) ||
					(!player->cards[std::to_underlying(Card::BlueCard)] &&
						!player->cards[std::to_underlying(Card::BlueSkull)]) ||
					// e6y
					// Compatibility with buggy MBF behavior when 3-key door works with only 2 keys
					// There is no more desync on 10sector.wad\ts27-137.lmp
					// http://www.doomworld.com/tas/ts27-137.zip
					(!player->cards[std::to_underlying(Card::YellowCard)] &&
						(compatibility_level == CompLevel::Mbf &&
							!prboom_comp[std::to_underlying(PrboomComp::ForceCorrectCodeFor3KeysDoorsInMbf)].state
							? player->cards[std::to_underlying(Card::YellowSkull)]
							: !player->cards[std::to_underlying(Card::YellowSkull)]))
				)
			)
			{
				dsda_AddPlayerMessage(s_PD_ALL3, player);
				S_StartMobjSound(player->mo, SfxId::Oof); // killough 3/20/98
				return false;
			}
			break;
	}
	return true;
}

dboolean P_CheckKeys(mobj_t* mo, ZDoomLock lock, dboolean legacy)
{
	player_t* player;
	const char* message = nullptr;
	SfxId sfx = SfxId::None;
	dboolean successful = true;

	if(!mo || !mo->player)
		return false;

	player = mo->player;

	switch(lock)
	{
		case ZDoomLock::None:
			break;
		case ZDoomLock::RedCard:
			if(!player->cards[std::to_underlying(Card::RedCard)])
			{
				message = legacy ? s_PD_REDC : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::BlueCard:
			if(!player->cards[std::to_underlying(Card::BlueCard)])
			{
				message = legacy ? s_PD_BLUEC : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::YellowCard:
			if(!player->cards[std::to_underlying(Card::YellowCard)])
			{
				message = legacy ? s_PD_YELLOWC : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::RedSkull:
			if(!player->cards[std::to_underlying(Card::RedSkull)])
			{
				message = legacy ? s_PD_REDS : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::BlueSkull:
			if(!player->cards[std::to_underlying(Card::BlueSkull)])
			{
				message = legacy ? s_PD_BLUES : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::YellowSkull:
			if(!player->cards[std::to_underlying(Card::YellowSkull)])
			{
				message = legacy ? s_PD_YELLOWS : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::Any:
			if(
				!player->cards[std::to_underlying(Card::RedCard)] &&
				!player->cards[std::to_underlying(Card::RedSkull)] &&
				!player->cards[std::to_underlying(Card::BlueCard)] &&
				!player->cards[std::to_underlying(Card::BlueSkull)] &&
				!player->cards[std::to_underlying(Card::YellowCard)] &&
				!player->cards[std::to_underlying(Card::YellowSkull)]
			)
			{
				message = legacy ? s_PD_ANY : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::All:
			if(
				!player->cards[std::to_underlying(Card::RedCard)] ||
				!player->cards[std::to_underlying(Card::RedSkull)] ||
				!player->cards[std::to_underlying(Card::BlueCard)] ||
				!player->cards[std::to_underlying(Card::BlueSkull)] ||
				!player->cards[std::to_underlying(Card::YellowCard)] ||
				!player->cards[std::to_underlying(Card::YellowSkull)]
			)
			{
				message = legacy ? s_PD_ALL6 : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::Red:
		case ZDoomLock::Redx:
			if(!player->cards[std::to_underlying(Card::RedCard)] && !player->cards[std::to_underlying(Card::RedSkull)])
			{
				message = legacy ? s_PD_REDK : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::Blue:
		case ZDoomLock::Bluex:
			if(!player->cards[std::to_underlying(Card::BlueCard)] && !player->cards[std::to_underlying(Card::BlueSkull)])
			{
				message = legacy ? s_PD_BLUEK : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::Yellow:
		case ZDoomLock::Yellowx:
			if(!player->cards[std::to_underlying(Card::YellowCard)] && !player->cards[std::to_underlying(Card::YellowSkull)])
			{
				message = legacy ? s_PD_YELLOWK : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
			break;
		case ZDoomLock::EachColor:
			if(
				(!player->cards[std::to_underlying(Card::RedCard)] && !player->cards[std::to_underlying(Card::RedSkull)]) ||
				(!player->cards[std::to_underlying(Card::BlueCard)] && !player->cards[std::to_underlying(Card::BlueSkull)]) ||
				(!player->cards[std::to_underlying(Card::YellowCard)] && !player->cards[std::to_underlying(Card::YellowSkull)])
			)
			{
				message = legacy ? s_PD_ALL3 : nullptr;
				sfx = legacy ? SfxId::Oof : SfxId::None;
				successful = false;
			}
		default:
			break;
	}

	if(message)
	{
		dsda_AddPlayerMessage(message, player);
	}

	if(sfx != SfxId::None)
	{
		S_StartMobjSound(mo, sfx);
	}

	return successful;
}


//
// P_SectorActive()
//
// In old compatibility levels, floor and ceiling data couldn't coexist.
// Lighting data is only relevant in zdoom levels.
//

dboolean PUREFUNC P_PlaneActive(const sector_t* sec)
{
	return sec->ceilingdata != nullptr || sec->floordata != nullptr;
}

dboolean PUREFUNC P_CeilingActive(const sector_t* sec)
{
	return sec->ceilingdata != nullptr || (demo_compatibility && sec->floordata != nullptr);
}

dboolean PUREFUNC P_FloorActive(const sector_t* sec)
{
	return sec->floordata != nullptr || (demo_compatibility && sec->ceilingdata != nullptr);
}

dboolean PUREFUNC P_LightingActive(const sector_t* sec)
{
	return sec->lightingdata != nullptr;
}

short P_FloorLightLevel(const sector_t* sec)
{
	return sec->lightlevel_floor + (
		((sec->flags & SectorFlag::LightFloorAbsolute) != SectorFlag{})
		? 0
		: (
			sec->floorlightsec == -1
			? sec->lightlevel
			: sectors[sec->floorlightsec].lightlevel
		)
	);
}

short P_CeilingLightLevel(const sector_t* sec)
{
	return sec->lightlevel_ceiling + (
		((sec->flags & SectorFlag::LightCeilingAbsolute) != SectorFlag{})
		? 0
		: (
			sec->ceilinglightsec == -1
			? sec->lightlevel
			: sectors[sec->ceilinglightsec].lightlevel
		)
	);
}

dboolean P_FloorPlanesDiffer(const sector_t* sec, const sector_t* other)
{
	return sec->floorpic != other->floorpic ||
		sec->floor_xoffs != other->floor_xoffs ||
		sec->floor_yoffs != other->floor_yoffs ||
		sec->floor_rotation != other->floor_rotation ||
		sec->floor_xscale != other->floor_xscale ||
		sec->floor_yscale != other->floor_yscale ||
		sec->special != other->special ||
		sec->floorlightsec != other->floorlightsec ||
		P_FloorLightLevel(sec) != P_FloorLightLevel(other);
}

dboolean P_CeilingPlanesDiffer(const sector_t* sec, const sector_t* other)
{
	return sec->ceilingpic != other->ceilingpic ||
		sec->ceiling_xoffs != other->ceiling_xoffs ||
		sec->ceiling_yoffs != other->ceiling_yoffs ||
		sec->ceiling_rotation != other->ceiling_rotation ||
		sec->ceiling_xscale != other->ceiling_xscale ||
		sec->ceiling_yscale != other->ceiling_yscale ||
		sec->ceilinglightsec != other->ceilinglightsec ||
		P_CeilingLightLevel(sec) != P_CeilingLightLevel(other);
}

//
// P_CheckTag()
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm. If compatibility, all linedef specials are
// allowed to have zero tag.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
// jff 2/27/98 Added to check for zero tag allowed for regular special types
//
int P_CheckTag(line_t* line)
{
	/* tag not zero, allowed, or
	* killough 11/98: compatibility option */
	if(comp[std::to_underlying(CompOption::ZeroTags)] || line->special_args[0]) //e6y
		return 1;

	switch(line->special)
	{
		case 1: // Manual door specials
		case 26:
		case 27:
		case 28:
		case 31:
		case 32:
		case 33:
		case 34:
		case 117:
		case 118:

		case 139: // Lighting specials
		case 170:
		case 79:
		case 35:
		case 138:
		case 171:
		case 81:
		case 13:
		case 192:
		case 169:
		case 80:
		case 12:
		case 194:
		case 173:
		case 157:
		case 104:
		case 193:
		case 172:
		case 156:
		case 17:

		case 195: // Thing teleporters
		case 174:
		case 97:
		case 39:
		case 126:
		case 125:
		case 210:
		case 209:
		case 208:
		case 207:

		case 11: // Exits
		case 52:
		case 197:
		case 51:
		case 124:
		case 198:

		case 48: // Scrolling walls
		case 85:
			return 1; // zero tag allowed

		default:
			break;
	}
	return 0; // zero tag not allowed
}

static const damage_t no_damage = {0};

static void P_TransferSectorFlags(SectorFlag* dest, const SectorFlag source)
{
	*dest -= SectorFlag::TransferMask;
	*dest |= source & SectorFlag::TransferMask;
}

static void P_ResetSectorTransferFlags(SectorFlag* flags)
{
	*flags -= SectorFlag::TransferMask;
}

void P_CopySectorSpecial(sector_t* dest, sector_t* source)
{
	dest->special = source->special;
	dest->damage = source->damage;
	P_TransferSectorFlags(&dest->flags, source->flags);
}

void P_TransferSpecial(sector_t* sector, newspecial_t* newspecial)
{
	sector->special = newspecial->special;
	sector->damage = newspecial->damage;
	P_TransferSectorFlags(&sector->flags, newspecial->flags);
}

void P_CopyTransferSpecial(newspecial_t* newspecial, sector_t* sector)
{
	newspecial->special = sector->special;
	newspecial->damage = sector->damage;
	P_TransferSectorFlags(&newspecial->flags, sector->flags);
}

void P_ResetTransferSpecial(newspecial_t* newspecial)
{
	newspecial->special = 0;
	newspecial->damage = no_damage;
	P_ResetSectorTransferFlags(&newspecial->flags);
}

void P_ResetSectorSpecial(sector_t* sector)
{
	sector->special = 0;
	sector->damage = no_damage;
	P_ResetSectorTransferFlags(&sector->flags);
}

void P_ClearNonGeneralizedSectorSpecial(sector_t* sector)
{
	// jff 3/14/98 clear non-generalized sector type
	sector->special &= map_format.generalized_mask;
}

dboolean P_IsSpecialSector(sector_t* sector)
{
	return sector->special || (sector->flags & SectorFlag::Secret) != SectorFlag{} || sector->damage.amount;
}

static void P_AddSectorSecret(sector_t* sector)
{
	totalsecret++;
	sector->flags |= SectorFlag::Secret | SectorFlag::WasSecret;
}

void P_AddMobjSecret(mobj_t* mobj)
{
	totalsecret++;
	mobj->flags2 |= MobjFlag2::CountSecret;
}

void P_PlayerCollectSecret(player_t* player)
{
	player->secretcount++;

	if(dsda_IntConfig(ConfigId::HudaddSecretarea))
	{
		SfxId sfx_id = heretic ? SfxId::HereticChat : hexen ? SfxId::HexenChat : SfxId::Itmbk;

		if(I_GetSfxLumpNum(&S_sfx[std::to_underlying(g_sfx_secret)]) != -1)
			sfx_id = g_sfx_secret;

		SetCustomMessage(player - players, s_HUSTR_SECRETFOUND, 2 * TICRATE, sfx_id);
	}
}

static void P_CollectSecretCommon(sector_t* sector, player_t* player)
{
	sector->flags -= SectorFlag::Secret;

	P_PlayerCollectSecret(player);

	dsda_WatchSecret();
}

static void P_CollectSecretVanilla(sector_t* sector, player_t* player)
{
	sector->special = 0;
	P_CollectSecretCommon(sector, player);
}

static void P_CollectSecretBoom(sector_t* sector, player_t* player)
{
	sector->special &= ~std::to_underlying(BoomSectorFlag::Secret);

	if(sector->special < 32) // if all extended bits clear,
		sector->special = 0; // sector is not special anymore

	P_CollectSecretCommon(sector, player);
}

static void P_CollectSecretZDoom(sector_t* sector, player_t* player)
{
	P_CollectSecretCommon(sector, player);
}

//
// P_IsSecret()
//
// Passed a sector, returns if the sector secret type is still active, i.e.
// secret type is set and the secret has not yet been obtained.
//
// jff 3/14/98 added to simplify checks for whether sector is secret
//  in automap and other places
//
dboolean PUREFUNC P_IsSecret(const sector_t* sec)
{
	return (sec->flags & SectorFlag::Secret) != SectorFlag{};
}

//
// P_IsDeathExit()
//
// If the sector a death exit via E1M8 or MBF21 actions
//
dboolean PUREFUNC P_IsDeathExit(const sector_t* sec)
{
	if(sec->special < 32)
	{
		return (sec->special == 11);
	}
	else if(mbf21 && SectorSpecialHas(sec->special, BoomSectorFlag::Death))
	{
		const int i = BoomSectorDamageLevel(sec->special);

		return (i == 2 || i == 3);
	}

	return false;
}


//
// P_WasSecret()
//
// Passed a sector, returns if the sector secret type is was active, i.e.
// secret type was set and the secret has been obtained already.
//
// jff 3/14/98 added to simplify checks for whether sector is secret
//  in automap and other places
//
dboolean PUREFUNC P_WasSecret(const sector_t* sec)
{
	return (sec->flags & SectorFlag::WasSecret) != SectorFlag{};
}

dboolean PUREFUNC P_RevealedSecret(const sector_t* sec)
{
	return P_WasSecret(sec) && !P_IsSecret(sec);
}

extern "C" void P_CrossHexenSpecialLine(line_t* line, int side, mobj_t* thing, dboolean bossaction)
{
	if(thing->player)
	{
		P_ActivateLine(line, thing, side, LineActivation::Cross);
	}
	else if((thing->flags2 & MobjFlag2::MonsterCross) != MobjFlag2{})
	{
		P_ActivateLine(line, thing, side, LineActivation::MonsterCross);
	}
	else if((thing->flags2 & MobjFlag2::ProjectileCross) != MobjFlag2{})
	{
		P_ActivateLine(line, thing, side, LineActivation::ProjectileCross);
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Events
//
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//
/////////////////////////////////////////////////////////////////////////

//
// P_CrossSpecialLine - Walkover Trigger Dispatcher
//
// Called every time a thing origin is about
//  to cross a line with a non 0 special, whether a walkover type or not.
//
// jff 02/12/98 all W1 lines were fixed to check the result from the EV_
//  function before clearing the special. This avoids losing the function
//  of the line, should the sector already be active when the line is
//  crossed. Change is qualified by demo_compatibility.
//
// CPhipps - take a line_t pointer instead of a line number, as in MBF
extern "C" void P_CrossCompatibleSpecialLine(line_t* line, int side, mobj_t* thing, dboolean bossaction)
{
	int ok;

	dsda_WatchLineActivation(line, thing);

	//  Things that should never trigger lines
	//
	// e6y: Improved support for Doom v1.2
	if(compatibility_level == CompLevel::Doom12)
	{
		if(line->special > 98 && line->special != 104)
		{
			return;
		}
	}
	else
	{
		if(!thing->player && !bossaction)
		{
			// Things that should NOT trigger specials...
			switch(thing->type)
			{
				case MobjType::Rocket:
				case MobjType::Plasma:
				case MobjType::Bfg:
				case MobjType::Troopshot:
				case MobjType::Headshot:
				case MobjType::Bruisershot:
					return;
					break;

				default: break;
			}
		}
	}

	//jff 02/04/98 add check here for generalized lindef types
	if(!demo_compatibility) // generalized types not recognized if old demo
	{
		// pointer to line function is NULL by default, set non-null if
		// line special is walkover generalized linedef type
		int (*linefunc)(line_t* line) = nullptr;

		// check each range of generalized linedefs
		if((unsigned)line->special >= GenEnd)
		{
			// Out of range for GenFloors
		}
		else if((unsigned)line->special >= GenFloorBase)
		{
			if(!thing->player && !bossaction)
				if((line->special & FloorChange) || !(line->special & FloorModel))
					return;            // FloorModel is "Allow Monsters" if FloorChange is 0
			if(!line->special_args[0]) //jff 2/27/98 all walk generalized types require tag
				return;
			linefunc = EV_DoGenFloor;
		}
		else if((unsigned)line->special >= GenCeilingBase)
		{
			if(!thing->player && !bossaction)
				if((line->special & CeilingChange) || !(line->special & CeilingModel))
					return;            // CeilingModel is "Allow Monsters" if CeilingChange is 0
			if(!line->special_args[0]) //jff 2/27/98 all walk generalized types require tag
				return;
			linefunc = EV_DoGenCeiling;
		}
		else if((unsigned)line->special >= GenDoorBase)
		{
			if(!thing->player && !bossaction)
			{
				if(!(line->special & DoorMonster))
					return;                 // monsters disallowed from this door
				if((line->flags & LineFlag::Secret) != LineFlag{}) // they can't open secret doors either
					return;
			}
			if(!line->special_args[0]) //3/2/98 move outside the monster check
				return;
			linefunc = EV_DoGenDoor;
		}
		else if((unsigned)line->special >= GenLockedBase)
		{
			if(!thing->player || bossaction) // boss actions can't handle locked doors
				return;                      // monsters disallowed from unlocking doors
			if(((line->special & TriggerType) == std::to_underlying(GenTriggerType::WalkOnce)) || ((line->special & TriggerType) == std::to_underlying(GenTriggerType::WalkMany)))
			{
				//jff 4/1/98 check for being a walk type before reporting door type
				if(!P_CanUnlockGenDoor(line, thing->player))
					return;
			}
			else
				return;
			linefunc = EV_DoGenLockedDoor;
		}
		else if((unsigned)line->special >= GenLiftBase)
		{
			if(!thing->player && !bossaction)
				if(!(line->special & LiftMonster))
					return;            // monsters disallowed
			if(!line->special_args[0]) //jff 2/27/98 all walk generalized types require tag
				return;
			linefunc = EV_DoGenLift;
		}
		else if((unsigned)line->special >= GenStairsBase)
		{
			if(!thing->player && !bossaction)
				if(!(line->special & StairMonster))
					return;            // monsters disallowed
			if(!line->special_args[0]) //jff 2/27/98 all walk generalized types require tag
				return;
			linefunc = EV_DoGenStairs;
		}
		else if(mbf21 && (unsigned)line->special >= GenCrusherBase)
		{
			// haleyjd 06/09/09: This was completely forgotten in BOOM, disabling
			// all generalized walk-over crusher types!
			if(!thing->player && !bossaction)
				if(!(line->special & StairMonster))
					return;            // monsters disallowed
			if(!line->special_args[0]) //jff 2/27/98 all walk generalized types require tag
				return;
			linefunc = EV_DoGenCrusher;
		}

		if(linefunc) // if it was a valid generalized type
			switch(static_cast<GenTriggerType>((line->special & TriggerType) >> TriggerTypeShift))
			{
				case GenTriggerType::WalkOnce:
					if(linefunc(line))
						line->special = 0; // clear special if a walk once type
					return;
				case GenTriggerType::WalkMany:
					linefunc(line);
					return;
				default: // if not a walk type, do nothing here
					return;
			}
	}

	if(!thing->player || bossaction)
	{
		ok = bossaction;
		switch(line->special)
		{
			// teleporters are blocked for boss actions.
			case 39:  // teleport trigger
			case 97:  // teleport retrigger
			case 125: // teleport monsteronly trigger
			case 126: // teleport monsteronly retrigger
			//jff 3/5/98 add ability of monsters etc. to use teleporters
			case 208: //silent thing teleporters
			case 207:
			case 243: //silent line-line teleporter
			case 244: //jff 3/6/98 make fit within DCK's 256 linedef types
			case 262: //jff 4/14/98 add monster only
			case 263: //jff 4/14/98 silent thing,line,line rev types
			case 264: //jff 4/14/98 plus player/monster silent line
			case 265: //            reversed types
			case 266:
			case 267:
			case 268:
			case 269:
				if(bossaction) return;

			case 4:  // raise door
			case 10: // plat down-wait-up-stay trigger
			case 88: // plat down-wait-up-stay retrigger
				ok = 1;
				break;
		}
		if(!ok)
			return;
	}

	if(!P_CheckTag(line)) //jff 2/27/98 disallow zero tag on some types
		return;

	// Dispatch on the line special value to the line's action routine
	// If a once only function, and successful, clear the line special

	switch(line->special)
	{
		// Regular walk once triggers

		case 2:
			// Open Door
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::OpenDoor)) || demo_compatibility)
				line->special = 0;
			break;

		case 3:
			// Close Door
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::CloseDoor)) || demo_compatibility)
				line->special = 0;
			break;

		case 4:
			// Raise Door
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::Normal)) || demo_compatibility)
				line->special = 0;
			break;

		case 5:
			// Raise Floor
			if(EV_DoFloor(line, FloorKind::RaiseFloor) || demo_compatibility)
				line->special = 0;
			break;

		case 6:
			// Fast Ceiling Crush & Raise
			if(EV_DoCeiling(line, CeilingKind::FastCrushAndRaise) || demo_compatibility)
				line->special = 0;
			break;

		case 8:
			// Build Stairs
			if(EV_BuildStairs(line, StairType::Build8) || demo_compatibility)
				line->special = 0;
			break;

		case 10:
			// PlatDownWaitUp
			if(EV_DoPlat(line, PlatType::DownWaitUpStay, 0) || demo_compatibility)
				line->special = 0;
			break;

		case 12:
			// Light Turn On - brightest near
			if(EV_LightTurnOn(line, 0) || demo_compatibility)
				line->special = 0;
			break;

		case 13:
			// Light Turn On 255
			if(EV_LightTurnOn(line, 255) || demo_compatibility)
				line->special = 0;
			break;

		case 16:
			// Close Door 30
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::Close30ThenOpen)) || demo_compatibility)
				line->special = 0;
			break;

		case 17:
			// Start Light Strobing
			if(EV_StartLightStrobing(line) || demo_compatibility)
				line->special = 0;
			break;

		case 19:
			// Lower Floor
			if(EV_DoFloor(line, FloorKind::LowerFloor) || demo_compatibility)
				line->special = 0;
			break;

		case 22:
			// Raise floor to nearest height and change texture
			if(EV_DoPlat(line, PlatType::RaiseToNearestAndChange, 0) || demo_compatibility)
				line->special = 0;
			break;

		case 25:
			// Ceiling Crush and Raise
			if(EV_DoCeiling(line, CeilingKind::CrushAndRaise) || demo_compatibility)
				line->special = 0;
			break;

		case 30:
			// Raise floor to shortest texture height
			//  on either side of lines.
			if(EV_DoFloor(line, FloorKind::RaiseToTexture) || demo_compatibility)
				line->special = 0;
			break;

		case 35:
			// Lights Very Dark
			if(EV_LightTurnOn(line, 35) || demo_compatibility)
				line->special = 0;
			break;

		case 36:
			// Lower Floor (TURBO)
			if(EV_DoFloor(line, FloorKind::TurboLower) || demo_compatibility)
				line->special = 0;
			break;

		case 37:
			// LowerAndChange
			if(EV_DoFloor(line, FloorKind::LowerAndChange) || demo_compatibility)
				line->special = 0;
			break;

		case 38:
			// Lower Floor To Lowest
			if(EV_DoFloor(line, FloorKind::LowerFloorToLowest) || demo_compatibility)
				line->special = 0;
			break;

		case 39:
			// TELEPORT! //jff 02/09/98 fix using up with wrong side crossing
			if(map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Vanilla) || demo_compatibility)
				line->special = 0;
			break;

		case 40:
			// RaiseCeilingLowerFloor
			if(demo_compatibility)
			{
				EV_DoCeiling(line, CeilingKind::RaiseToHighest);
				EV_DoFloor(line, FloorKind::LowerFloorToLowest); //jff 02/12/98 doesn't work
				line->special = 0;
			}
			else if(EV_DoCeiling(line, CeilingKind::RaiseToHighest))
				line->special = 0;
			break;

		case 44:
			// Ceiling Crush
			if(EV_DoCeiling(line, CeilingKind::LowerAndCrush) || demo_compatibility)
				line->special = 0;
			break;

		case 52:
			// EXIT!
			// killough 10/98: prevent zombies from exiting levels
			if(bossaction || (!(thing->player && thing->player->health <= 0 && !comp[std::to_underlying(CompOption::Zombie)])))
				G_ExitLevel(0);
			break;

		case 53:
			// Perpetual Platform Raise
			if(EV_DoPlat(line, PlatType::PerpetualRaise, 0) || demo_compatibility)
				line->special = 0;
			break;

		case 54:
			// Platform Stop
			if(EV_StopPlat(line) || demo_compatibility)
				line->special = 0;
			break;

		case 56:
			// Raise Floor Crush
			if(EV_DoFloor(line, FloorKind::RaiseFloorCrush) || demo_compatibility)
				line->special = 0;
			break;

		case 57:
			// Ceiling Crush Stop
			if(EV_CeilingCrushStop(line) || demo_compatibility)
				line->special = 0;
			break;

		case 58:
			// Raise Floor 24
			if(EV_DoFloor(line, FloorKind::RaiseFloor24) || demo_compatibility)
				line->special = 0;
			break;

		case 59:
			// Raise Floor 24 And Change
			if(EV_DoFloor(line, FloorKind::RaiseFloor24AndChange) || demo_compatibility)
				line->special = 0;
			break;

		case 100:
			// Build Stairs Turbo 16
			if(EV_BuildStairs(line, StairType::Turbo16) || demo_compatibility)
				line->special = 0;
			break;

		case 104:
			// Turn lights off in sector(tag)
			if(EV_TurnTagLightsOff(line) || demo_compatibility)
				line->special = 0;
			break;

		case 108:
			// Blazing Door Raise (faster than TURBO!)
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::BlazeRaise)) || demo_compatibility)
				line->special = 0;
			break;

		case 109:
			// Blazing Door Open (faster than TURBO!)
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::BlazeOpen)) || demo_compatibility)
				line->special = 0;
			break;

		case 110:
			// Blazing Door Close (faster than TURBO!)
			if(EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::BlazeClose)) || demo_compatibility)
				line->special = 0;
			break;

		case 119:
			// Raise floor to nearest surr. floor
			if(EV_DoFloor(line, FloorKind::RaiseFloorToNearest) || demo_compatibility)
				line->special = 0;
			break;

		case 121:
			// Blazing PlatDownWaitUpStay
			if(EV_DoPlat(line, PlatType::BlazeDWUS, 0) || demo_compatibility)
				line->special = 0;
			break;

		case 124:
			// Secret EXIT
			// killough 10/98: prevent zombies from exiting levels
			// CPhipps - change for lxdoom's compatibility handling
			if(bossaction || (!(thing->player && thing->player->health <= 0 && !comp[std::to_underlying(CompOption::Zombie)])))
				G_SecretExitLevel(0);
			break;

		case 125:
			// TELEPORT MonsterONLY
			if(!thing->player &&
				(map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Vanilla) || demo_compatibility))
				line->special = 0;
			break;

		case 130:
			// Raise Floor Turbo
			if(EV_DoFloor(line, FloorKind::RaiseFloorTurbo) || demo_compatibility)
				line->special = 0;
			break;

		case 141:
			// Silent Ceiling Crush & Raise
			if(EV_DoCeiling(line, CeilingKind::SilentCrushAndRaise) || demo_compatibility)
				line->special = 0;
			break;

		// Regular walk many retriggerable

		case 72:
			// Ceiling Crush
			EV_DoCeiling(line, CeilingKind::LowerAndCrush);
			break;

		case 73:
			// Ceiling Crush and Raise
			EV_DoCeiling(line, CeilingKind::CrushAndRaise);
			break;

		case 74:
			// Ceiling Crush Stop
			EV_CeilingCrushStop(line);
			break;

		case 75:
			// Close Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::CloseDoor));
			break;

		case 76:
			// Close Door 30
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::Close30ThenOpen));
			break;

		case 77:
			// Fast Ceiling Crush & Raise
			EV_DoCeiling(line, CeilingKind::FastCrushAndRaise);
			break;

		case 79:
			// Lights Very Dark
			EV_LightTurnOn(line, 35);
			break;

		case 80:
			// Light Turn On - brightest near
			EV_LightTurnOn(line, 0);
			break;

		case 81:
			// Light Turn On 255
			EV_LightTurnOn(line, 255);
			break;

		case 82:
			// Lower Floor To Lowest
			EV_DoFloor(line, FloorKind::LowerFloorToLowest);
			break;

		case 83:
			// Lower Floor
			EV_DoFloor(line, FloorKind::LowerFloor);
			break;

		case 84:
			// LowerAndChange
			EV_DoFloor(line, FloorKind::LowerAndChange);
			break;

		case 86:
			// Open Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::OpenDoor));
			break;

		case 87:
			// Perpetual Platform Raise
			EV_DoPlat(line, PlatType::PerpetualRaise, 0);
			break;

		case 88:
			// PlatDownWaitUp
			EV_DoPlat(line, PlatType::DownWaitUpStay, 0);
			break;

		case 89:
			// Platform Stop
			EV_StopPlat(line);
			break;

		case 90:
			// Raise Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::Normal));
			break;

		case 91:
			// Raise Floor
			EV_DoFloor(line, FloorKind::RaiseFloor);
			break;

		case 92:
			// Raise Floor 24
			EV_DoFloor(line, FloorKind::RaiseFloor24);
			break;

		case 93:
			// Raise Floor 24 And Change
			EV_DoFloor(line, FloorKind::RaiseFloor24AndChange);
			break;

		case 94:
			// Raise Floor Crush
			EV_DoFloor(line, FloorKind::RaiseFloorCrush);
			break;

		case 95:
			// Raise floor to nearest height
			// and change texture.
			EV_DoPlat(line, PlatType::RaiseToNearestAndChange, 0);
			break;

		case 96:
			// Raise floor to shortest texture height
			// on either side of lines.
			EV_DoFloor(line, FloorKind::RaiseToTexture);
			break;

		case 97:
			// TELEPORT!
			map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Vanilla);
			break;

		case 98:
			// Lower Floor (TURBO)
			EV_DoFloor(line, FloorKind::TurboLower);
			break;

		case 105:
			// Blazing Door Raise (faster than TURBO!)
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::BlazeRaise));
			break;

		case 106:
			// Blazing Door Open (faster than TURBO!)
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::BlazeOpen));
			break;

		case 107:
			// Blazing Door Close (faster than TURBO!)
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::BlazeClose));
			break;

		case 120:
			// Blazing PlatDownWaitUpStay.
			EV_DoPlat(line, PlatType::BlazeDWUS, 0);
			break;

		case 126:
			// TELEPORT MonsterONLY.
			if(!thing->player)
				map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Vanilla);
			break;

		case 128:
			// Raise To Nearest Floor
			EV_DoFloor(line, FloorKind::RaiseFloorToNearest);
			break;

		case 129:
			// Raise Floor Turbo
			EV_DoFloor(line, FloorKind::RaiseFloorTurbo);
			break;

		// Extended walk triggers

		// jff 1/29/98 added new linedef types to fill all functions out so that
		// all have varieties SR, S1, WR, W1

		// killough 1/31/98: "factor out" compatibility test, by
		// adding inner switch qualified by compatibility flag.
		// relax test to demo_compatibility

		// killough 2/16/98: Fix problems with W1 types being cleared too early

		default:
			if(!demo_compatibility)
				switch(line->special)
				{
					// Extended walk once triggers

					case 142:
						// Raise Floor 512
						// 142 W1  EV_DoFloor(raiseFloor512)
						if(EV_DoFloor(line, FloorKind::RaiseFloor512))
							line->special = 0;
						break;

					case 143:
						// Raise Floor 24 and change
						// 143 W1  EV_DoPlat(raiseAndChange,24)
						if(EV_DoPlat(line, PlatType::RaiseAndChange, 24))
							line->special = 0;
						break;

					case 144:
						// Raise Floor 32 and change
						// 144 W1  EV_DoPlat(raiseAndChange,32)
						if(EV_DoPlat(line, PlatType::RaiseAndChange, 32))
							line->special = 0;
						break;

					case 145:
						// Lower Ceiling to Floor
						// 145 W1  EV_DoCeiling(lowerToFloor)
						if(EV_DoCeiling(line, CeilingKind::LowerToFloor))
							line->special = 0;
						break;

					case 146:
						// Lower Pillar, Raise Donut
						// 146 W1  EV_DoDonut()
						if(EV_DoDonut(line))
							line->special = 0;
						break;

					case 199:
						// Lower ceiling to lowest surrounding ceiling
						// 199 W1 EV_DoCeiling(lowerToLowest)
						if(EV_DoCeiling(line, CeilingKind::LowerToLowest))
							line->special = 0;
						break;

					case 200:
						// Lower ceiling to highest surrounding floor
						// 200 W1 EV_DoCeiling(lowerToMaxFloor)
						if(EV_DoCeiling(line, CeilingKind::LowerToMaxFloor))
							line->special = 0;
						break;

					case 207:
						// killough 2/16/98: W1 silent teleporter (normal kind)
						if(map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Silent))
							line->special = 0;
						break;

					//jff 3/16/98 renumber 215->153
					case 153: //jff 3/15/98 create texture change no motion type
						// Texture/Type Change Only (Trig)
						// 153 W1 Change Texture/Type Only
						if(EV_DoChange(line, ChangeKind::TriggerOnly, line->special_args[0]))
							line->special = 0;
						break;

					case 239: //jff 3/15/98 create texture change no motion type
						// Texture/Type Change Only (Numeric)
						// 239 W1 Change Texture/Type Only
						if(EV_DoChange(line, ChangeKind::NumericOnly, line->special_args[0]))
							line->special = 0;
						break;

					case 219:
						// Lower floor to next lower neighbor
						// 219 W1 Lower Floor Next Lower Neighbor
						if(EV_DoFloor(line, FloorKind::LowerFloorToNearest))
							line->special = 0;
						break;

					case 227:
						// Raise elevator next floor
						// 227 W1 Raise Elevator next floor
						if(EV_DoElevator(line, ElevatorType::Up))
							line->special = 0;
						break;

					case 231:
						// Lower elevator next floor
						// 231 W1 Lower Elevator next floor
						if(EV_DoElevator(line, ElevatorType::Down))
							line->special = 0;
						break;

					case 235:
						// Elevator to current floor
						// 235 W1 Elevator to current floor
						if(EV_DoElevator(line, ElevatorType::Current))
							line->special = 0;
						break;

					case 243: //jff 3/6/98 make fit within DCK's 256 linedef types
						// killough 2/16/98: W1 silent teleporter (linedef-linedef kind)
						if(EV_SilentLineTeleport(line, side, thing, line->special_args[0], false))
							line->special = 0;
						break;

					case 262: //jff 4/14/98 add silent line-line reversed
						if(EV_SilentLineTeleport(line, side, thing, line->special_args[0], true))
							line->special = 0;
						break;

					case 264: //jff 4/14/98 add monster-only silent line-line reversed
						if(!thing->player &&
							EV_SilentLineTeleport(line, side, thing, line->special_args[0], true))
							line->special = 0;
						break;

					case 266: //jff 4/14/98 add monster-only silent line-line
						if(!thing->player &&
							EV_SilentLineTeleport(line, side, thing, line->special_args[0], false))
							line->special = 0;
						break;

					case 268: //jff 4/14/98 add monster-only silent
						if(!thing->player && map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Silent))
							line->special = 0;
						break;

					//jff 1/29/98 end of added W1 linedef types

					// Extended walk many retriggerable

					//jff 1/29/98 added new linedef types to fill all functions
					//out so that all have varieties SR, S1, WR, W1

					case 147:
						// Raise Floor 512
						// 147 WR  EV_DoFloor(raiseFloor512)
						EV_DoFloor(line, FloorKind::RaiseFloor512);
						break;

					case 148:
						// Raise Floor 24 and Change
						// 148 WR  EV_DoPlat(raiseAndChange,24)
						EV_DoPlat(line, PlatType::RaiseAndChange, 24);
						break;

					case 149:
						// Raise Floor 32 and Change
						// 149 WR  EV_DoPlat(raiseAndChange,32)
						EV_DoPlat(line, PlatType::RaiseAndChange, 32);
						break;

					case 150:
						// Start slow silent crusher
						// 150 WR  EV_DoCeiling(silentCrushAndRaise)
						EV_DoCeiling(line, CeilingKind::SilentCrushAndRaise);
						break;

					case 151:
						// RaiseCeilingLowerFloor
						// 151 WR  EV_DoCeiling(raiseToHighest),
						//         EV_DoFloor(lowerFloortoLowest)
						EV_DoCeiling(line, CeilingKind::RaiseToHighest);
						EV_DoFloor(line, FloorKind::LowerFloorToLowest);
						break;

					case 152:
						// Lower Ceiling to Floor
						// 152 WR  EV_DoCeiling(lowerToFloor)
						EV_DoCeiling(line, CeilingKind::LowerToFloor);
						break;

					//jff 3/16/98 renumber 153->256
					case 256:
						// Build stairs, step 8
						// 256 WR EV_BuildStairs(build8)
						EV_BuildStairs(line, StairType::Build8);
						break;

					//jff 3/16/98 renumber 154->257
					case 257:
						// Build stairs, step 16
						// 257 WR EV_BuildStairs(turbo16)
						EV_BuildStairs(line, StairType::Turbo16);
						break;

					case 155:
						// Lower Pillar, Raise Donut
						// 155 WR  EV_DoDonut()
						EV_DoDonut(line);
						break;

					case 156:
						// Start lights strobing
						// 156 WR Lights EV_StartLightStrobing()
						EV_StartLightStrobing(line);
						break;

					case 157:
						// Lights to dimmest near
						// 157 WR Lights EV_TurnTagLightsOff()
						EV_TurnTagLightsOff(line);
						break;

					case 201:
						// Lower ceiling to lowest surrounding ceiling
						// 201 WR EV_DoCeiling(lowerToLowest)
						EV_DoCeiling(line, CeilingKind::LowerToLowest);
						break;

					case 202:
						// Lower ceiling to highest surrounding floor
						// 202 WR EV_DoCeiling(lowerToMaxFloor)
						EV_DoCeiling(line, CeilingKind::LowerToMaxFloor);
						break;

					case 208:
						// killough 2/16/98: WR silent teleporter (normal kind)
						map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Silent);
						break;

					case 212: //jff 3/14/98 create instant toggle floor type
						// Toggle floor between C and F instantly
						// 212 WR Instant Toggle Floor
						EV_DoPlat(line, PlatType::ToggleUpDn, 0);
						break;

					//jff 3/16/98 renumber 216->154
					case 154: //jff 3/15/98 create texture change no motion type
						// Texture/Type Change Only (Trigger)
						// 154 WR Change Texture/Type Only
						EV_DoChange(line, ChangeKind::TriggerOnly, line->special_args[0]);
						break;

					case 240: //jff 3/15/98 create texture change no motion type
						// Texture/Type Change Only (Numeric)
						// 240 WR Change Texture/Type Only
						EV_DoChange(line, ChangeKind::NumericOnly, line->special_args[0]);
						break;

					case 220:
						// Lower floor to next lower neighbor
						// 220 WR Lower Floor Next Lower Neighbor
						EV_DoFloor(line, FloorKind::LowerFloorToNearest);
						break;

					case 228:
						// Raise elevator next floor
						// 228 WR Raise Elevator next floor
						EV_DoElevator(line, ElevatorType::Up);
						break;

					case 232:
						// Lower elevator next floor
						// 232 WR Lower Elevator next floor
						EV_DoElevator(line, ElevatorType::Down);
						break;

					case 236:
						// Elevator to current floor
						// 236 WR Elevator to current floor
						EV_DoElevator(line, ElevatorType::Current);
						break;

					case 244: //jff 3/6/98 make fit within DCK's 256 linedef types
						// killough 2/16/98: WR silent teleporter (linedef-linedef kind)
						EV_SilentLineTeleport(line, side, thing, line->special_args[0], false);
						break;

					case 263: //jff 4/14/98 add silent line-line reversed
						EV_SilentLineTeleport(line, side, thing, line->special_args[0], true);
						break;

					case 265: //jff 4/14/98 add monster-only silent line-line reversed
						if(!thing->player)
							EV_SilentLineTeleport(line, side, thing, line->special_args[0], true);
						break;

					case 267: //jff 4/14/98 add monster-only silent line-line
						if(!thing->player)
							EV_SilentLineTeleport(line, side, thing, line->special_args[0], false);
						break;

					case 269: //jff 4/14/98 add monster-only silent
						if(!thing->player)
							map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Silent);
						break;

						//jff 1/29/98 end of added WR linedef types
				}
			break;
	}
}

extern "C" void P_CrossZDoomSpecialLine(line_t* line, int side, mobj_t* thing, dboolean bossaction)
{
	if(thing->player)
	{
		P_ActivateLine(line, thing, side, LineActivation::Cross);
	}
	else if((thing->flags2 & MobjFlag2::MonsterCross) != MobjFlag2{})
	{
		P_ActivateLine(line, thing, side, LineActivation::MonsterCross);
	}
	else if((thing->flags2 & MobjFlag2::ProjectileCross) != MobjFlag2{})
	{
		P_ActivateLine(line, thing, side, LineActivation::ProjectileCross);
	}
	else if(line->special == std::to_underlying(ZDoomLineSpecial::Teleport) ||
		line->special == std::to_underlying(ZDoomLineSpecial::TeleportNoFog) ||
		line->special == std::to_underlying(ZDoomLineSpecial::TeleportLine))
	{
		// [RH] Just a little hack for BOOM compatibility
		P_ActivateLine(line, thing, side, LineActivation::MonsterCross);
	}
	else
	{
		P_ActivateLine(line, thing, side, LineActivation::AnyCross);
	}
}

//
// P_ShootSpecialLine - Gun trigger special dispatcher
//
// Called when a thing shoots a special line with bullet, shell, saw, or fist.
//
// jff 02/12/98 all G1 lines were fixed to check the result from the EV_
// function before clearing the special. This avoids losing the function
// of the line, should the sector already be in motion when the line is
// impacted. Change is qualified by demo_compatibility.
//

extern "C" void P_ShootCompatibleSpecialLine(mobj_t* thing, line_t* line)
{
	//jff 02/04/98 add check here for generalized linedef
	if(!demo_compatibility)
	{
		// pointer to line function is NULL by default, set non-null if
		// line special is gun triggered generalized linedef type
		int (*linefunc)(line_t* line) = nullptr;

		// check each range of generalized linedefs
		if((unsigned)line->special >= GenEnd)
		{
			// Out of range for GenFloors
		}
		else if((unsigned)line->special >= GenFloorBase)
		{
			if(!thing->player)
				if((line->special & FloorChange) || !(line->special & FloorModel))
					return;            // FloorModel is "Allow Monsters" if FloorChange is 0
			if(!line->special_args[0]) //jff 2/27/98 all gun generalized types require tag
				return;

			linefunc = EV_DoGenFloor;
		}
		else if((unsigned)line->special >= GenCeilingBase)
		{
			if(!thing->player)
				if((line->special & CeilingChange) || !(line->special & CeilingModel))
					return;            // CeilingModel is "Allow Monsters" if CeilingChange is 0
			if(!line->special_args[0]) //jff 2/27/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenCeiling;
		}
		else if((unsigned)line->special >= GenDoorBase)
		{
			if(!thing->player)
			{
				if(!(line->special & DoorMonster))
					return;                 // monsters disallowed from this door
				if((line->flags & LineFlag::Secret) != LineFlag{}) // they can't open secret doors either
					return;
			}
			if(!line->special_args[0]) //jff 3/2/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenDoor;
		}
		else if((unsigned)line->special >= GenLockedBase)
		{
			if(!thing->player)
				return; // monsters disallowed from unlocking doors
			if(((line->special & TriggerType) == std::to_underlying(GenTriggerType::GunOnce)) || ((line->special & TriggerType) == std::to_underlying(GenTriggerType::GunMany)))
			{
				//jff 4/1/98 check for being a gun type before reporting door type
				if(!P_CanUnlockGenDoor(line, thing->player))
					return;
			}
			else
				return;
			if(!line->special_args[0]) //jff 2/27/98 all gun generalized types require tag
				return;

			linefunc = EV_DoGenLockedDoor;
		}
		else if((unsigned)line->special >= GenLiftBase)
		{
			if(!thing->player)
				if(!(line->special & LiftMonster))
					return; // monsters disallowed
			linefunc = EV_DoGenLift;
		}
		else if((unsigned)line->special >= GenStairsBase)
		{
			if(!thing->player)
				if(!(line->special & StairMonster))
					return;            // monsters disallowed
			if(!line->special_args[0]) //jff 2/27/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenStairs;
		}
		else if((unsigned)line->special >= GenCrusherBase)
		{
			if(!thing->player)
				if(!(line->special & StairMonster))
					return;            // monsters disallowed
			if(!line->special_args[0]) //jff 2/27/98 all gun generalized types require tag
				return;
			linefunc = EV_DoGenCrusher;
		}

		if(linefunc)
			switch(static_cast<GenTriggerType>((line->special & TriggerType) >> TriggerTypeShift))
			{
				case GenTriggerType::GunOnce:
					if(linefunc(line))
						P_ChangeSwitchTexture(line, 0);
					return;
				case GenTriggerType::GunMany:
					if(linefunc(line))
						P_ChangeSwitchTexture(line, 1);
					return;
				default: // if not a gun type, do nothing here
					return;
			}
	}

	// Impacts that other things can activate.
	if(!thing->player)
	{
		int ok = 0;
		switch(line->special)
		{
			case 46:
				// 46 GR Open door on impact weapon is monster activatable
				ok = 1;
				break;
		}
		if(!ok)
			return;
	}

	if(!P_CheckTag(line)) //jff 2/27/98 disallow zero tag on some types
		return;

	switch(line->special)
	{
		case 24:
			// 24 G1 raise floor to highest adjacent
			if(EV_DoFloor(line, FloorKind::RaiseFloor) || demo_compatibility)
				P_ChangeSwitchTexture(line, 0);
			break;

		case 46:
			// 46 GR open door, stay open
			EV_DoDoor(line, static_cast<VerticalDoorType>(g_door_open));
			P_ChangeSwitchTexture(line, 1);
			break;

		case 47:
			// 47 G1 raise floor to nearest and change texture and type
			if(EV_DoPlat(line, PlatType::RaiseToNearestAndChange, 0) || demo_compatibility)
				P_ChangeSwitchTexture(line, 0);
			break;

		//jff 1/30/98 added new gun linedefs here
		// killough 1/31/98: added demo_compatibility check, added inner switch

		default:
			if(!demo_compatibility)
				switch(line->special)
				{
					case 197:
						// Exit to next level
						// killough 10/98: prevent zombies from exiting levels
						if(thing->player && thing->player->health <= 0 && !comp[std::to_underlying(CompOption::Zombie)])
							break;
						P_ChangeSwitchTexture(line, 0);
						G_ExitLevel(0);
						break;

					case 198:
						// Exit to secret level
						// killough 10/98: prevent zombies from exiting levels
						if(thing->player && thing->player->health <= 0 && !comp[std::to_underlying(CompOption::Zombie)])
							break;
						P_ChangeSwitchTexture(line, 0);
						G_SecretExitLevel(0);
						break;
						//jff end addition of new gun linedefs
				}
			break;
	}
}

extern "C" void P_ShootHexenSpecialLine(mobj_t* thing, line_t* line)
{
	P_ActivateLine(line, thing, 0, LineActivation::Impact);
}

static void P_ApplySectorDamage(player_t* player, int damage, int leak)
{
	if(!player->powers[std::to_underlying(PowerType::IronFeet)] || (leak && P_Random(RandomClass::Slimehurt) < leak))
		if(!(leveltime & 0x1f))
			P_DamageMobj(player->mo, nullptr, nullptr, damage);
}

static void P_ApplySectorDamageEndLevel(player_t* player)
{
	if(comp[std::to_underlying(CompOption::God)])
		player->cheats -= CheatFlag::GodMode;

	if(!(leveltime & 0x1f))
		P_DamageMobj(player->mo, nullptr, nullptr, 20);

	if(player->health <= 10)
		G_ExitLevel(0);
}

static void P_ApplyGeneralizedSectorDamage(player_t* player, int level)
{
	switch(level & 3)
	{
		case 0:
			break;
		case 1:
			P_ApplySectorDamage(player, 5, 0);
			break;
		case 2:
			P_ApplySectorDamage(player, 10, 0);
			break;
		case 3:
			P_ApplySectorDamage(player, 20, 5);
			break;
	}
}

extern "C" void P_PlayerInCompatibleSector(player_t* player, sector_t* sector)
{
	//jff add if to handle old vs generalized types
	if(sector->special < 32) // regular sector specials
	{
		switch(sector->special)
		{
			case 5:
				P_ApplySectorDamage(player, 10, 0);
				break;
			case 7:
				P_ApplySectorDamage(player, 5, 0);
				break;
			case 16:
			case 4:
				P_ApplySectorDamage(player, 20, 5);
				break;
			case 9:
				P_CollectSecretVanilla(sector, player);
				break;
			case 11:
				P_ApplySectorDamageEndLevel(player);
				break;
			default:
				break;
		}
	}
	else //jff 3/14/98 handle extended sector damage
	{
		if(mbf21 && SectorSpecialHas(sector->special, BoomSectorFlag::Death))
		{
			int i;

			switch(BoomSectorDamageLevel(sector->special))
			{
				case 0:
					if(!player->powers[std::to_underlying(PowerType::Invulnerability)] && !player->powers[std::to_underlying(PowerType::IronFeet)])
						P_DamageMobj(player->mo, nullptr, nullptr, 10000);
					break;
				case 1:
					P_DamageMobj(player->mo, nullptr, nullptr, 10000);
					break;
				case 2:
					for(i = 0; i < g_maxplayers; i++)
						if(playeringame[i])
							P_DamageMobj(players[i].mo, nullptr, nullptr, 10000);
					G_ExitLevel(0);
					break;
				case 3:
					for(i = 0; i < g_maxplayers; i++)
						if(playeringame[i])
							P_DamageMobj(players[i].mo, nullptr, nullptr, 10000);
					G_SecretExitLevel(0);
					break;
			}
		}
		else
		{
			P_ApplyGeneralizedSectorDamage(player, BoomSectorDamageLevel(sector->special));
		}
	}

	if((sector->flags & SectorFlag::Secret) != SectorFlag{})
	{
		P_CollectSecretBoom(sector, player);
	}
}

extern "C" void P_PlayerInZDoomSector(player_t* player, sector_t* sector)
{
	static const int heretic_carry[5] = {
		2048 * 5,
		2048 * 10,
		2048 * 25,
		2048 * 30,
		2048 * 35
	};

	static const int hexen_carry[3] = {
		2048 * 5,
		2048 * 10,
		2048 * 25
	};

	if(sector->damage.amount > 0)
	{
		if((sector->flags & SectorFlag::EndGodMode) != SectorFlag{})
		{
			player->cheats -= CheatFlag::GodMode;
		}

		if(
			(sector->flags & SectorFlag::DamageUnblockable) != SectorFlag{} ||
			!player->powers[std::to_underlying(PowerType::IronFeet)] ||
			(sector->damage.leakrate && P_Random(RandomClass::Slimehurt) < sector->damage.leakrate)
		)
		{
			if((sector->flags & SectorFlag::Hazard) != SectorFlag{})
			{
				player->hazardcount += sector->damage.amount;
				player->hazardinterval = sector->damage.interval;
			}
			else
			{
				if(leveltime % sector->damage.interval == 0)
				{
					P_DamageMobj(player->mo, nullptr, nullptr, sector->damage.amount);

					if((sector->flags & SectorFlag::EndLevel) != SectorFlag{} && player->health <= 10)
					{
						G_ExitLevel(0);
					}

					if((sector->flags & SectorFlag::DamageTerrainEffect) != SectorFlag{})
					{
						// MAP_FORMAT_TODO: damage special effects
					}
				}
			}
		}
	}
	else if(sector->damage.amount < 0)
	{
		if(leveltime % sector->damage.interval == 0)
		{
			P_GiveBody(player, -sector->damage.amount);
		}
	}

	switch(static_cast<ZDoomSectorSpecial>(sector->special))
	{
		case ZDoomSectorSpecial::DScrollEastLavaDamage:
			P_Thrust(player, 0, 2048 * 28);
			break;
		case ZDoomSectorSpecial::ScrollStrifeCurrent:
		{
			int anglespeed;
			fixed_t carryspeed;
			angle_t angle;

			anglespeed = sector->tag - 100;
			carryspeed = (anglespeed % 10) * 4096;
			angle = (anglespeed / 10) * ANG45;
			P_Thrust(player, angle, carryspeed);
		}
		break;
		case ZDoomSectorSpecial::CarryEast5:
		case ZDoomSectorSpecial::CarryEast10:
		case ZDoomSectorSpecial::CarryEast25:
		case ZDoomSectorSpecial::CarryEast30:
		case ZDoomSectorSpecial::CarryEast35:
			P_Thrust(player, 0, heretic_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::CarryEast5)]);
			break;
		case ZDoomSectorSpecial::CarryNorth5:
		case ZDoomSectorSpecial::CarryNorth10:
		case ZDoomSectorSpecial::CarryNorth25:
		case ZDoomSectorSpecial::CarryNorth30:
		case ZDoomSectorSpecial::CarryNorth35:
			P_Thrust(player, ANG90, heretic_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::CarryNorth5)]);
			break;
		case ZDoomSectorSpecial::CarrySouth5:
		case ZDoomSectorSpecial::CarrySouth10:
		case ZDoomSectorSpecial::CarrySouth25:
		case ZDoomSectorSpecial::CarrySouth30:
		case ZDoomSectorSpecial::CarrySouth35:
			P_Thrust(player, ANG270, heretic_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::CarrySouth5)]);
			break;
		case ZDoomSectorSpecial::CarryWest5:
		case ZDoomSectorSpecial::CarryWest10:
		case ZDoomSectorSpecial::CarryWest25:
		case ZDoomSectorSpecial::CarryWest30:
		case ZDoomSectorSpecial::CarryWest35:
			P_Thrust(player, ANG180, heretic_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::CarryWest5)]);
			break;
		case ZDoomSectorSpecial::ScrollNorthSlow:
		case ZDoomSectorSpecial::ScrollNorthMedium:
		case ZDoomSectorSpecial::ScrollNorthFast:
			P_Thrust(player, ANG90, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollNorthSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollEastSlow:
		case ZDoomSectorSpecial::ScrollEastMedium:
		case ZDoomSectorSpecial::ScrollEastFast:
			P_Thrust(player, 0, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollEastSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollSouthSlow:
		case ZDoomSectorSpecial::ScrollSouthMedium:
		case ZDoomSectorSpecial::ScrollSouthFast:
			P_Thrust(player, ANG270, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollSouthSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollWestSlow:
		case ZDoomSectorSpecial::ScrollWestMedium:
		case ZDoomSectorSpecial::ScrollWestFast:
			P_Thrust(player, ANG180, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollWestSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollNorthwestSlow:
		case ZDoomSectorSpecial::ScrollNorthwestMedium:
		case ZDoomSectorSpecial::ScrollNorthwestFast:
			P_Thrust(player, ANG135, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollNorthwestSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollNortheastSlow:
		case ZDoomSectorSpecial::ScrollNortheastMedium:
		case ZDoomSectorSpecial::ScrollNortheastFast:
			P_Thrust(player, ANG45, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollNortheastSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollSoutheastSlow:
		case ZDoomSectorSpecial::ScrollSoutheastMedium:
		case ZDoomSectorSpecial::ScrollSoutheastFast:
			P_Thrust(player, ANG315, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollSoutheastSlow)]);
			break;
		case ZDoomSectorSpecial::ScrollSouthwestSlow:
		case ZDoomSectorSpecial::ScrollSouthwestMedium:
		case ZDoomSectorSpecial::ScrollSouthwestFast:
			P_Thrust(player, ANG225, hexen_carry[sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollSouthwestSlow)]);
			break;
		default:
			break;
	}

	if((sector->flags & SectorFlag::Secret) != SectorFlag{})
	{
		P_CollectSecretZDoom(sector, player);
	}
}

//
// P_PlayerInSpecialSector()
//
// Called every tick frame
//  that the player origin is in a special sector
//
// Changed to ignore sector types the engine does not recognize
//

void P_PlayerInSpecialSector(player_t* player)
{
	sector_t* sector;

	sector = player->mo->subsector->sector;

	// Falling, not all the way down yet?
	// Sector specials don't apply in mid-air
	if(player->mo->z != sector->floorheight)
		return;

	map_format.player_in_special_sector(player, sector);
}

extern "C" dboolean P_MobjInCompatibleSector(mobj_t* mobj)
{
	if(mbf21)
	{
		sector_t* sector = mobj->subsector->sector;

		if(
			SectorSpecialHas(sector->special, BoomSectorFlag::KillMonsters) &&
			mobj->z == mobj->floorz &&
			mobj->player == nullptr &&
			(mobj->flags & MobjFlag::Shootable) != MobjFlag{} &&
			(mobj->flags & MobjFlag::Float) == MobjFlag{}
		)
		{
			P_DamageMobj(mobj, nullptr, nullptr, 10000);

			// must have been removed
			if(mobj->thinker.function != reinterpret_cast<think_t>(P_MobjThinker))
				return true;
		}
	}

	return false;
}

extern "C" dboolean P_MobjInHereticSector(mobj_t* mobj)
{
	return false;
}

extern "C" dboolean P_MobjInHexenSector(mobj_t* mobj)
{
	return false;
}

extern "C" dboolean P_MobjInZDoomSector(mobj_t* mobj)
{
	return false;
}

//
// P_UpdateSpecials()
//
// Check level timer, frag counter,
// animate flats, scroll walls,
// change button textures
//
// Reads and modifies globals:
//  levelTimer, levelTimeCount,
//  levelFragLimit, levelFragLimitCount
//

static dboolean levelTimer;
static int levelTimeCount;
dboolean levelFragLimit; // Ty 03/18/98 Added -frags support
int levelFragLimitCount; // Ty 03/18/98 Added -frags support

void P_UpdateSpecials()
{
	anim_t* anim;
	int pic;
	int i;

	// hexen_note: possibly not needed?
	if(!hexen)
	{
		// Downcount level timer, exit level if elapsed
		if(levelTimer == true)
		{
			levelTimeCount--;
			if(!levelTimeCount)
				G_ExitLevel(0);
		}

		// Check frag counters, if frag limit reached, exit level // Ty 03/18/98
		//  Seems like the total frags should be kept in a simple
		//  array somewhere, but until they are...
		if(levelFragLimit == true) // we used -frags so compare count
		{
			int k, m, fragcount, exitflag = false;
			for(k = 0; k < g_maxplayers; k++)
			{
				if(!playeringame[k]) continue;
				fragcount = 0;
				for(m = 0; m < g_maxplayers; m++)
				{
					if(!playeringame[m]) continue;
					fragcount += (m != k) ? players[k].frags[m] : -players[k].frags[m];
				}
				if(fragcount >= levelFragLimitCount) exitflag = true;
				if(exitflag == true) break; // skip out of the loop--we're done
			}
			if(exitflag == true)
				G_ExitLevel(0);
		}
	}

	// MAP_FORMAT_TODO: needs investigation
	if(!map_format.animdefs)
	{
		// Animate flats and textures globally
		for(anim = anims; anim < lastanim; anim++)
		{
			for(i = 0; i < anim->numpics; ++i)
			{
				pic = anim->basepic + ((leveltime / anim->speed + i) % anim->numpics);
				if(anim->istexture)
					texturetranslation[anim->basepic + i] = pic;
				else
					flattranslation[anim->basepic + i] = pic;
			}
		}
	}

	// Check buttons (retriggerable switches) and change texture on timeout
	for(i = 0; i < MAXBUTTONS; i++)
		if(buttonlist[i].btimer)
		{
			buttonlist[i].btimer--;
			if(!buttonlist[i].btimer)
			{
				switch(buttonlist[i].where)
				{
					case ButtonWhere::Top:
						sides[buttonlist[i].line->sidenum[0]].toptexture =
							buttonlist[i].btexture;
						break;

					case ButtonWhere::Middle:
						sides[buttonlist[i].line->sidenum[0]].midtexture =
							buttonlist[i].btexture;
						break;

					case ButtonWhere::Bottom:
						sides[buttonlist[i].line->sidenum[0]].bottomtexture =
							buttonlist[i].btexture;
						break;
				}
				if(!hexen)
				{
					/* don't take the address of the switch's sound origin,
					* unless in a compatibility mode. */
					degenmobj_t* so = buttonlist[i].soundorg;
					if(comp[std::to_underlying(CompOption::Sound)] || compatibility_level < CompLevel::Prboom6)
						/* since the buttonlist array is usually zeroed out,
						* button popouts generally appear to come from (0,0) */
						so = (degenmobj_t*)&buttonlist[i].soundorg;
					S_StartLineSound(buttonlist[i].line, so, g_sfx_swtchn);
				}
				memset(&buttonlist[i], 0, sizeof(button_t));
			}
		}
}

//////////////////////////////////////////////////////////////////////
//
// Sector and Line special thinker spawning at level startup
//
//////////////////////////////////////////////////////////////////////

//
// P_SpawnSpecials
// After the map has been loaded,
//  scan for specials that spawn thinkers
//

extern "C" void P_SpawnCompatibleSectorSpecial(sector_t* sector, int i)
{
	if(SectorSpecialHas(sector->special, BoomSectorFlag::Secret)) //jff 3/15/98 count extended
		P_AddSectorSecret(sector);

	if(SectorSpecialHas(sector->special, BoomSectorFlag::Friction))
		sector->flags |= SectorFlag::Friction;

	if(SectorSpecialHas(sector->special, BoomSectorFlag::Push))
		sector->flags |= SectorFlag::Push;

	switch((demo_compatibility && !prboom_comp[std::to_underlying(PrboomComp::TruncatedSectorSpecials)].state) ? sector->special : sector->special & 31)
	{
		case 1:
			// random off
			P_SpawnLightFlash(sector);
			break;

		case 2:
			// strobe fast
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			break;

		case 3:
			// strobe slow
			P_SpawnStrobeFlash(sector, SLOWDARK, 0);
			break;

		case 4:
			// strobe fast/death slime
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			if(heretic)
				sector->special = 4;
			else
				sector->special |= 3 << k_boomSectorDamageLevelShift; //jff 3/14/98 put damage bits in
			break;

		case 8:
			// glowing light
			P_SpawnGlowingLight(sector);
			break;
		case 9:
			// secret sector
			if(sector->special < 32) //jff 3/14/98 bits don't count unless not
				P_AddSectorSecret(sector);
			break;

		case 10:
			// door close in 30 seconds
			P_SpawnDoorCloseIn30(sector);
			break;

		case 12:
			// sync strobe slow
			P_SpawnStrobeFlash(sector, SLOWDARK, 1);
			break;

		case 13:
			// sync strobe fast
			P_SpawnStrobeFlash(sector, FASTDARK, 1);
			break;

		case 14:
			// door raise in 5 minutes
			P_SpawnDoorRaiseIn5Mins(sector, i);
			break;

		case 17:
			// fire flickering
			P_SpawnFireFlicker(sector);
			break;
	}
}

void P_SpawnZDoomLights(sector_t* sector)
{
	switch(static_cast<ZDoomSectorSpecial>(sector->special))
	{
		case ZDoomSectorSpecial::LightPhased:
			P_SpawnPhasedLight(sector, 80, -1);
			break;
		case ZDoomSectorSpecial::LightSequenceStart:
			P_SpawnLightSequence(sector, 1);
			break;
		case ZDoomSectorSpecial::DLightFlicker:
			P_SpawnLightFlash(sector);
			break;
		case ZDoomSectorSpecial::DLightStrobeFast:
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			break;
		case ZDoomSectorSpecial::DLightStrobeSlow:
			P_SpawnStrobeFlash(sector, SLOWDARK, 0);
			break;
		case ZDoomSectorSpecial::DLightStrobeHurt:
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			sector->special |= std::to_underlying(ZDoomSectorSpecial::DLightStrobeHurt);
			break;
		case ZDoomSectorSpecial::DLightGlow:
			P_SpawnGlowingLight(sector);
			break;
		case ZDoomSectorSpecial::DLightStrobeSlowSync:
			P_SpawnStrobeFlash(sector, SLOWDARK, 1);
			break;
		case ZDoomSectorSpecial::DLightStrobeFastSync:
			P_SpawnStrobeFlash(sector, FASTDARK, 1);
			break;
		case ZDoomSectorSpecial::DLightFireFlicker:
			P_SpawnFireFlicker(sector);
			break;
		case ZDoomSectorSpecial::DScrollEastLavaDamage:
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			sector->special |= std::to_underlying(ZDoomSectorSpecial::DScrollEastLavaDamage);
			break;
		case ZDoomSectorSpecial::SLightStrobeHurt:
			P_SpawnStrobeFlash(sector, FASTDARK, 0);
			sector->special |= std::to_underlying(ZDoomSectorSpecial::SLightStrobeHurt);
			break;
		default:
			break;
	}
}

void P_SetupSectorDamage(sector_t* sector, short amount,
	byte interval, byte leakrate, const SectorFlag flags)
{
	// Only set if damage is not yet initialized.
	if(sector->damage.amount)
		return;

	sector->damage.amount = amount;
	sector->damage.interval = interval;
	sector->damage.leakrate = leakrate;
	sector->flags = (sector->flags - SectorFlag::DamageFlags) | (flags & SectorFlag::DamageFlags);
}

static void P_SpawnZDoomGeneralizedSpecials(sector_t* sector)
{
	switch(ZDoomSectorDamageLevel(sector->special))
	{
		case 0:
			break;
		case 1:
			P_SetupSectorDamage(sector, 5, 32, 0, SectorFlag{});
			break;
		case 2:
			P_SetupSectorDamage(sector, 10, 32, 0, SectorFlag{});
			break;
		case 3:
			P_SetupSectorDamage(sector, 20, 32, 5, SectorFlag{});
			break;
	}

	if(SectorSpecialHas(sector->special, ZDoomSectorFlag::Secret))
		P_AddSectorSecret(sector);

	if(SectorSpecialHas(sector->special, ZDoomSectorFlag::Friction))
		sector->flags |= SectorFlag::Friction;

	if(SectorSpecialHas(sector->special, ZDoomSectorFlag::Push))
		sector->flags |= SectorFlag::Push;
}

extern "C" void P_SpawnZDoomSectorSpecial(sector_t* sector, int i)
{
	P_SpawnZDoomGeneralizedSpecials(sector);

	sector->special &= 0xff;

	P_SpawnZDoomLights(sector);

	switch(static_cast<ZDoomSectorSpecial>(sector->special))
	{
		case ZDoomSectorSpecial::DScrollEastLavaDamage:
			dsda_AddFloorScroller(-4, 0, sector - sectors, 0);
			P_SetupSectorDamage(sector, 5, 32, 0, SectorFlag::DamageTerrainEffect | SectorFlag::DamageUnblockable);
			break;
		case ZDoomSectorSpecial::SLightStrobeHurt:
		case ZDoomSectorSpecial::DDamageNukage:
			P_SetupSectorDamage(sector, 5, 32, 0, SectorFlag{});
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DDamageHellslime:
			P_SetupSectorDamage(sector, 10, 32, 0, SectorFlag{});
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DLightStrobeHurt:
		case ZDoomSectorSpecial::DDamageSuperHellslime:
			P_SetupSectorDamage(sector, 20, 32, 5, SectorFlag{});
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DDamageEnd:
			P_SetupSectorDamage(sector, 20, 32, 0, SectorFlag::EndGodMode | SectorFlag::EndLevel | SectorFlag::DamageUnblockable);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DamageInstantDeath:
			P_SetupSectorDamage(sector, 10000, 1, 0, SectorFlag::DamageUnblockable);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::HDamageSludge:
			P_SetupSectorDamage(sector, 4, 32, 0, SectorFlag{});
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DDamageLavaWimpy:
			P_SetupSectorDamage(sector, 5, 32, 0, SectorFlag::DamageTerrainEffect | SectorFlag::DamageUnblockable);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DDamageLavaHefty:
			P_SetupSectorDamage(sector, 8, 32, 0, SectorFlag::DamageTerrainEffect | SectorFlag::DamageUnblockable);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::SDamageHellslime:
			P_SetupSectorDamage(sector, 2, 32, 0, SectorFlag::Hazard);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::SDamageSuperHellslime:
			P_SetupSectorDamage(sector, 4, 32, 0, SectorFlag::Hazard);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::SectorHeal:
			P_SetupSectorDamage(sector, -1, 32, 0, SectorFlag{});
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DSectorDoorCloseIn30:
			P_SpawnDoorCloseIn30(sector);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DSectorDoorRaiseIn5Mins:
			P_SpawnDoorRaiseIn5Mins(sector, i);
			sector->special = 0;
			break;
		case ZDoomSectorSpecial::DFrictionLow:
			sector->friction = FRICTION_LOW;
			sector->movefactor = 0x269;
			sector->flags |= SectorFlag::Friction;
			break;
		case ZDoomSectorSpecial::SectorHidden:
			sector->flags |= SectorFlag::Hidden;
			sector->special = 0;
			break;
		default:
			if(sector->special >= std::to_underlying(ZDoomSectorSpecial::ScrollNorthSlow) &&
				sector->special <= std::to_underlying(ZDoomSectorSpecial::ScrollSouthwestFast))
			{
				// Hexen scroll special
				static const fixed_t hexenScrollies[24][2] =
				{
					{0, 1}, {0, 2}, {0, 4},
					{-1, 0}, {-2, 0}, {-4, 0},
					{0, -1}, {0, -2}, {0, -4},
					{1, 0}, {2, 0}, {4, 0},
					{1, 1}, {2, 2}, {4, 4},
					{-1, 1}, {-2, 2}, {-4, 4},
					{-1, -1}, {-2, -2}, {-4, -4},
					{1, -1}, {2, -2}, {4, -4}
				};

				int i;
				fixed_t dx, dy;

				i = sector->special - std::to_underlying(ZDoomSectorSpecial::ScrollNorthSlow);
				dx = FixedDiv(hexenScrollies[i][0] << FRACBITS, 2);
				dy = FixedDiv(hexenScrollies[i][1] << FRACBITS, 2);
				dsda_AddFloorScroller(dx, dy, sector - sectors, 0);
			}
			else if(sector->special >= std::to_underlying(ZDoomSectorSpecial::CarryEast5) &&
				sector->special <= std::to_underlying(ZDoomSectorSpecial::CarryEast35))
			{
				// Heretic scroll special
				// Only east scrollers also scroll the texture
				fixed_t dx = FixedDiv((1 << (sector->special - std::to_underlying(ZDoomSectorSpecial::CarryEast5))) << FRACBITS, 2);
				dsda_AddFloorScroller(dx, 0, sector - sectors, 0);
			}
			break;
	}
}

static void P_SpawnVanillaExtras()
{
	int i;

	// allow MBF sky transfers in all complevels
	if(!heretic)
		for(i = 0; i < numlines; ++i)
			switch(lines[i].special)
			{
					const int* id_p;

				case 271: // Regular sky
				case 272: // Same, only flipped
					FIND_SECTORS(id_p, lines[i].special_args[0])
					{
						sectors[*id_p].floorsky = SkyFlatTagged(i, SkyFlatTag::Line);
						sectors[*id_p].ceilingsky = SkyFlatTagged(i, SkyFlatTag::Line);
					}
					break;
			}
}

extern "C" void P_SpawnCompatibleExtra(line_t* l, int i)
{
	const int* id_p;
	int sec;

	switch(l->special)
	{
		// killough 3/7/98:
		// support for drawn heights coming from different sector
		case 242:
			sec = sides[*l->sidenum].sector->iSectorID;
			FIND_SECTORS(id_p, lines[i].special_args[0])
				sectors[*id_p].heightsec = sec;
			break;

		// killough 3/16/98: Add support for setting
		// floor lighting independently (e.g. lava)
		case 213:
			sec = sides[*l->sidenum].sector->iSectorID;
			FIND_SECTORS(id_p, lines[i].special_args[0])
				sectors[*id_p].floorlightsec = sec;
			break;

		// killough 4/11/98: Add support for setting
		// ceiling lighting independently
		case 261:
			sec = sides[*l->sidenum].sector->iSectorID;
			FIND_SECTORS(id_p, lines[i].special_args[0])
				sectors[*id_p].ceilinglightsec = sec;
			break;

		// killough 10/98:
		//
		// Support for sky textures being transferred from sidedefs.
		// Allows scrolling and other effects (but if scrolling is
		// used, then the same sector tag needs to be used for the
		// sky sector, the sky-transfer linedef, and the scroll-effect
		// linedef). Still requires user to use F_SKY1 for the floor
		// or ceiling texture, to distinguish floor and ceiling sky.

		case 271: // Regular sky
		case 272: // Same, only flipped
			FIND_SECTORS(id_p, lines[i].special_args[0])
			{
				sectors[*id_p].floorsky = SkyFlatTagged(i, SkyFlatTag::Line);
				sectors[*id_p].ceilingsky = SkyFlatTagged(i, SkyFlatTag::Line);
			}
			break;
	}
}

extern "C" void P_SpawnZDoomExtra(line_t* l, int i)
{
	const int* id_p;
	int sec;

	switch(l->special)
	{
		// killough 3/7/98:
		// support for drawn heights coming from different sector
		case std::to_underlying(ZDoomLineSpecial::TransferHeights):
			sec = sides[*l->sidenum].sector->iSectorID;
			FIND_SECTORS(id_p, l->special_args[0])
				sectors[*id_p].heightsec = sec;
			break;

		// killough 3/16/98: Add support for setting
		// floor lighting independently (e.g. lava)
		case std::to_underlying(ZDoomLineSpecial::TransferFloorLight):
			sec = sides[*l->sidenum].sector->iSectorID;
			FIND_SECTORS(id_p, l->special_args[0])
				sectors[*id_p].floorlightsec = sec;
			break;

		// killough 4/11/98: Add support for setting
		// ceiling lighting independently
		case std::to_underlying(ZDoomLineSpecial::TransferCeilingLight):
			sec = sides[*l->sidenum].sector->iSectorID;
			FIND_SECTORS(id_p, l->special_args[0])
				sectors[*id_p].ceilinglightsec = sec;
			break;

		// [RH] ZDoom Static_Init settings
		case std::to_underlying(ZDoomLineSpecial::StaticInit):
			switch(static_cast<ZDoomStaticInit>(l->special_args[1]))
			{
				case ZDoomStaticInit::Gravity:
				{
					fixed_t grav = FixedDiv(P_AproxDistance(l->dx, l->dy), 100 * FRACUNIT);
					sec = sides[*l->sidenum].sector->iSectorID;
					FIND_SECTORS(id_p, l->special_args[0])
						sectors[*id_p].gravity = grav;
				}
				break;

				case ZDoomStaticInit::Damage:
				{
					damage_t damage;
					SectorFlag flags = {};

					damage.amount = P_AproxDistance(l->dx, l->dy) >> FRACBITS;
					if(damage.amount < 20)
					{
						damage.leakrate = 0;
						damage.interval = 32;
					}
					else if(damage.amount < 50)
					{
						damage.leakrate = 5;
						damage.interval = 32;
					}
					else
					{
						flags |= SectorFlag::DamageUnblockable;
						damage.leakrate = 0;
						damage.interval = 1;
					}

					sec = sides[*l->sidenum].sector->iSectorID;
					FIND_SECTORS(id_p, l->special_args[0])
					{
						sectors[*id_p].damage = damage;
						sectors[*id_p].flags |= flags;
					}
				}
				break;

				// killough 10/98:
				//
				// Support for sky textures being transferred from sidedefs.
				// Allows scrolling and other effects (but if scrolling is
				// used, then the same sector tag needs to be used for the
				// sky sector, the sky-transfer linedef, and the scroll-effect
				// linedef). Still requires user to use F_SKY1 for the floor
				// or ceiling texture, to distinguish floor and ceiling sky.
				case ZDoomStaticInit::TransferSky:
					FIND_SECTORS(id_p, l->special_args[0])
					{
						sectors[*id_p].floorsky = SkyFlatTagged(i, SkyFlatTag::Line);
						sectors[*id_p].ceilingsky = SkyFlatTagged(i, SkyFlatTag::Line);
					}
					break;
			}
			break;
	}
}

static void P_SpawnExtras()
{
	int i;
	line_t* l;

	for(i = 0, l = lines; i < numlines; i++, l++)
		map_format.spawn_extra(l, i);
}

static void P_EvaluateDeathmatchParams()
{
	dsda_arg_t* arg;

	levelTimer = false;

	arg = dsda_Arg(ArgId::Timer);
	if(arg->found && deathmatch)
	{
		levelTimer = true;
		levelTimeCount = arg->value.v_int * 60 * TICRATE;
	}

	levelFragLimit = false;

	arg = dsda_Arg(ArgId::Frags);
	if(arg->found && deathmatch)
	{
		levelFragLimit = true;
		levelFragLimitCount = arg->value.v_int;
	}
}

static void P_InitSectorSpecials()
{
	int i;
	sector_t* sector;

	sector = sectors;
	for(i = 0; i < numsectors; i++, sector++)
		if(sector->special)
			map_format.init_sector_special(sector, i);
}

static void P_InitButtons()
{
	int i;

	for(i = 0; i < MAXBUTTONS; i++)
		memset(&buttonlist[i], 0, sizeof(button_t));
}

static void Hexen_P_SpawnSpecials();

// Parses command line parameters.
void P_SpawnSpecials()
{
	if(hexen) return Hexen_P_SpawnSpecials();

	P_EvaluateDeathmatchParams();

	P_InitSectorSpecials();

	if(heretic) P_SpawnLineSpecials();

	P_RemoveAllActiveCeilings(); // jff 2/22/98 use killough's scheme
	P_RemoveAllActivePlats();    // killough

	P_InitButtons();

	P_SpawnScrollers(); // killough 3/7/98: Add generalized scrollers

	if(demo_compatibility) return P_SpawnVanillaExtras();

	P_SpawnFriction(); // phares 3/12/98: New friction model using linedefs
	P_SpawnPushers();  // phares 3/20/98: New pusher model using linedefs
	P_SpawnExtras();

	// MAP_FORMAT_TODO: Start "Open" scripts
}

// Adds wall scroller. Scroll amount is rotated with respect to wall's
// linedef first, so that scrolling towards the wall in a perpendicular
// direction is translated into vertical motion, while scrolling along
// the wall in a parallel direction is translated into horizontal motion.
//
// killough 5/25/98: cleaned up arithmetic to avoid drift due to roundoff
//
// killough 10/98:
// fix scrolling aliasing problems, caused by long linedefs causing overflowing

static void Add_WallScroller(fixed_t dx, fixed_t dy, const line_t* l,
	int control, int accel)
{
	fixed_t x = D_abs(l->dx);
	fixed_t y = D_abs(l->dy);
	fixed_t d;

	if(y > x)
		d = x, x = y, y = d;

	d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y, x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);

	// CPhipps - Import scroller calc overflow fix, compatibility optioned
	if(compatibility_level >= CompLevel::Lxdoom1)
	{
		x = (fixed_t)(((int64_t)dy * -l->dy - (int64_t)dx * l->dx) / d);
		y = (fixed_t)(((int64_t)dy * l->dx - (int64_t)dx * l->dy) / d);
	}
	else
	{
		x = -FixedDiv(FixedMul(dy, l->dy) + FixedMul(dx, l->dx), d);
		y = -FixedDiv(FixedMul(dx, l->dy) - FixedMul(dy, l->dx), d);
	}

	dsda_AddControlSideScroller(x, y, control, *l->sidenum, accel, 0);
}

// Amount (dx,dy) vector linedef is shifted right to get scroll amount
#define SCROLL_SHIFT 5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// (This is so scrolling floors and objects on them can move at same speed.)
#define CARRYFACTOR ((fixed_t)(FRACUNIT*.09375))

extern "C" void P_SpawnCompatibleScroller(line_t* l, int i)
{
	fixed_t dx = l->dx >> SCROLL_SHIFT; // direction and speed of scrolling
	fixed_t dy = l->dy >> SCROLL_SHIFT;
	int control = -1, accel = 0; // no control sector or acceleration
	int special = l->special;

	if(demo_compatibility && special != 48) return; //e6y

	// killough 3/7/98: Types 245-249 are same as 250-254 except that the
	// first side's sector's heights cause scrolling when they change, and
	// this linedef controls the direction and speed of the scrolling. The
	// most complicated linedef since donuts, but powerful :)
	//
	// killough 3/15/98: Add acceleration. Types 214-218 are the same but
	// are accelerative.

	if(special >= 245 && special <= 249) // displacement scrollers
	{
		special += 250 - 245;
		control = sides[*l->sidenum].sector->iSectorID;
	}
	else if(special >= 214 && special <= 218) // accelerative scrollers
	{
		accel = 1;
		special += 250 - 214;
		control = sides[*l->sidenum].sector->iSectorID;
	}

	switch(special)
	{
			int side;
			const int* id_p;

		case 250: // scroll effect ceiling
			FIND_SECTORS(id_p, l->special_args[0])
				dsda_AddControlCeilingScroller(-dx, dy, control, *id_p, accel, 0);
			break;

		case 251: // scroll effect floor
		case 253: // scroll and carry objects on floor
			FIND_SECTORS(id_p, l->special_args[0])
				dsda_AddControlFloorScroller(-dx, dy, control, *id_p, accel, 0);
			if(special != 253)
				break;
		// fallthrough

		case 252: // carry objects on floor
			dx = FixedMul(dx, CARRYFACTOR);
			dy = FixedMul(dy, CARRYFACTOR);
			FIND_SECTORS(id_p, l->special_args[0])
				dsda_AddControlFloorCarryScroller(dx, dy, control, *id_p, accel, 0);
			break;

		// killough 3/1/98: scroll wall according to linedef
		// (same direction and speed as scrolling floors)
		case 254:
			FIND_LINES(id_p, l->special_args[0])
				if(*id_p != i)
					Add_WallScroller(dx, dy, lines + *id_p, control, accel);
			break;

		case 255: // killough 3/2/98: scroll according to sidedef offsets
			side = lines[i].sidenum[0];
			dsda_AddSideScroller(-sides[side].textureoffset, sides[side].rowoffset, side, 0);
			break;

		case 1024: // special 255 with tag control
		case 1025:
		case 1026:
			if(l->special_args[0] == 0)
				Log::Fatal("Line {} is missing a tag!", i);

			if(special > 1024)
				control = sides[*l->sidenum].sector->iSectorID;

			if(special == 1026)
				accel = 1;

			side = lines[i].sidenum[0];
			dx = -sides[side].textureoffset / 8;
			dy = sides[side].rowoffset / 8;
			FIND_LINES(id_p, l->special_args[0])
				if(*id_p != i)
					dsda_AddControlSideScroller(dx, dy, control, lines[*id_p].sidenum[0], accel, 0);

			break;

		case 48: // scroll first side
			dsda_AddSideScroller(FRACUNIT, 0, lines[i].sidenum[0], 0);
			break;

		case 85: // jff 1/30/98 2-way scroll
			dsda_AddSideScroller(-FRACUNIT, 0, lines[i].sidenum[0], 0);
			break;
	}
}

static int copyscroller_count = 0;
static int copyscroller_max = 0;
static line_t** copyscrollers;

static void P_AddCopyScroller(line_t* l)
{
	while(copyscroller_count >= copyscroller_max)
	{
		copyscroller_max = copyscroller_max ? copyscroller_max * 2 : 8;
		copyscrollers = static_cast<line_t**>(Z_Realloc(copyscrollers, copyscroller_max * sizeof(*copyscrollers)));
	}

	copyscrollers[copyscroller_count++] = l;
}

static void P_InitCopyScrollers()
{
	int i;
	line_t* l;

	if(!map_format.zdoom) return;

	for(i = 0, l = lines; i < numlines; i++, l++)
		if(l->special == std::to_underlying(ZDoomLineSpecial::SectorCopyScroller))
		{
			// don't allow copying the scroller if the sector has the same tag
			//   as it would just duplicate it.
			if(l->frontsector->tag == l->special_args[0])
				P_AddCopyScroller(l);

			l->special = 0;
		}
}

static void P_FreeCopyScrollers()
{
	if(copyscrollers)
	{
		copyscroller_count = 0;
		copyscroller_max = 0;
		Z_Free(copyscrollers);
	}
}

extern "C" void P_SpawnZDoomScroller(line_t* l, int i)
{
	fixed_t dx = 0; // direction and speed of scrolling
	fixed_t dy = 0;
	int control = -1, accel = 0; // no control sector or acceleration
	int special = l->special;

	if(special == std::to_underlying(ZDoomLineSpecial::ScrollCeiling) ||
		special == std::to_underlying(ZDoomLineSpecial::ScrollFloor) ||
		special == std::to_underlying(ZDoomLineSpecial::ScrollTextureModel))
	{
		if(l->special_args[1] & 3)
		{
			// if 1, then displacement
			// if 2, then accelerative (also if 3)
			control = sides[*l->sidenum].sector->iSectorID;
			if(l->special_args[1] & 2)
				accel = 1;
		}

		if(special == std::to_underlying(ZDoomLineSpecial::ScrollTextureModel) || l->special_args[1] & 4)
		{
			// The line housing the special controls the
			// direction and speed of scrolling.
			dx = l->dx >> SCROLL_SHIFT;
			dy = l->dy >> SCROLL_SHIFT;
		}
		else
		{
			// The speed and direction are parameters to the special.
			dx = (fixed_t)(l->special_args[3] - 128) * FRACUNIT / 32;
			dy = (fixed_t)(l->special_args[4] - 128) * FRACUNIT / 32;
		}
	}

	switch(special)
	{
			int j;
			const int* id_p;

		case std::to_underlying(ZDoomLineSpecial::ScrollCeiling):
			FIND_SECTORS(id_p, l->special_args[0])
				dsda_AddControlCeilingScroller(-dx, dy, control, *id_p, accel, 0);

			for(j = 0; j < copyscroller_count; ++j)
			{
				line_t* cs = copyscrollers[j];

				if(cs->special_args[0] == l->special_args[0] && cs->special_args[1] & 1)
					dsda_AddControlCeilingScroller(-dx, dy, control, cs->frontsector->iSectorID, accel, 0);
			}

			l->special = 0;
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollFloor):
			if(l->special_args[2] != 1)
			{
				// scroll the floor texture
				FIND_SECTORS(id_p, l->special_args[0])
					dsda_AddControlFloorScroller(-dx, dy, control, *id_p, accel, 0);

				for(j = 0; j < copyscroller_count; ++j)
				{
					line_t* cs = copyscrollers[j];

					if(cs->special_args[0] == l->special_args[0] && cs->special_args[1] & 2)
						dsda_AddControlFloorScroller(-dx, dy, control, cs->frontsector->iSectorID, accel, 0);
				}
			}

			if(l->special_args[2] > 0)
			{
				// carry objects on the floor
				dx = FixedMul(dx, CARRYFACTOR);
				dy = FixedMul(dy, CARRYFACTOR);
				FIND_SECTORS(id_p, l->special_args[0])
					dsda_AddControlFloorCarryScroller(dx, dy, control, *id_p, accel, 0);

				for(j = 0; j < copyscroller_count; ++j)
				{
					line_t* cs = copyscrollers[j];

					if(cs->special_args[0] == l->special_args[0] && cs->special_args[1] & 4)
						dsda_AddControlFloorCarryScroller(dx, dy, control, cs->frontsector->iSectorID, accel, 0);
				}
			}

			l->special = 0;
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureModel):
			// killough 3/1/98: scroll wall according to linedef
			// (same direction and speed as scrolling floors)
			FIND_LINES(id_p, l->special_args[0])
				if(*id_p != i)
					Add_WallScroller(dx, dy, lines + *id_p, control, accel);

			l->special = 0;
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureOffsets):
			// killough 3/2/98: scroll according to sidedef offsets
			j = lines[i].sidenum[0];
			dsda_AddSideScroller(-sides[j].textureoffset, sides[j].rowoffset, j, l->special_args[0]);
			l->special = 0;
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureLeft):
			j = lines[i].sidenum[0];
			dsda_AddSideScroller(FRACUNIT * l->special_args[0] / 64, 0, j, l->special_args[1]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureRight):
			j = lines[i].sidenum[0];
			dsda_AddSideScroller(-FRACUNIT * l->special_args[0] / 64, 0, j, l->special_args[1]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureUp):
			j = lines[i].sidenum[0];
			dsda_AddSideScroller(0, FRACUNIT * l->special_args[0] / 64, j, l->special_args[1]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureDown):
			j = lines[i].sidenum[0];
			dsda_AddSideScroller(0, -FRACUNIT * l->special_args[0] / 64, j, l->special_args[1]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollTextureBoth):
			j = lines[i].sidenum[0];

			if(l->special_args[0] == 0)
			{
				dx = FRACUNIT * (l->special_args[1] - l->special_args[2]) / 64;
				dy = FRACUNIT * (l->special_args[4] - l->special_args[3]) / 64;
				dsda_AddSideScroller(dx, dy, j, 0);
			}

			l->special = 0;
			break;
		default:
			break;
	}
}

// Initialize the scrollers
static void P_SpawnScrollers()
{
	int i;
	line_t* l;

	P_InitCopyScrollers();

	for(i = 0, l = lines; i < numlines; i++, l++)
		map_format.spawn_scroller(l, i);

	P_FreeCopyScrollers();
}

// e6y
// restored boom's friction code

/////////////////////////////
//
// Add a friction thinker to the thinker list
//
// Add_Friction adds a new friction thinker to the list of active thinkers.
//

static void Add_Friction(int friction, int movefactor, int affectee)
{
	friction_t* f = static_cast<friction_t*>(Z_MallocLevel(sizeof *f));

	f->thinker.function/*.acp1*/ = /*(actionf_p1) */reinterpret_cast<think_t>(T_Friction);
	f->friction = friction;
	f->movefactor = movefactor;
	f->affectee = affectee;
	P_AddThinker(&f->thinker);
}

/////////////////////////////
//
// This is where abnormal friction is applied to objects in the sectors.
// A friction thinker has been spawned for each sector where less or
// more friction should be applied. The amount applied is proportional to
// the length of the controlling linedef.

void T_Friction(friction_t* f)
{
	sector_t* sec;
	mobj_t* thing;
	msecnode_t* node;

	if(compatibility || !variable_friction)
		return;

	sec = sectors + f->affectee;

	// Be sure the special sector type is still turned on. If so, proceed.
	// Else, bail out; the sector type has been changed on us.

	if((sec->flags & SectorFlag::Friction) == SectorFlag{})
		return;

	// Assign the friction value to players on the floor, non-floating,
	// and clipped. Normally the object's friction value is kept at
	// ORIG_FRICTION and this thinker changes it for icy or muddy floors.

	// In Phase II, you can apply friction to Things other than players.

	// When the object is straddling sectors with the same
	// floorheight that have different frictions, use the lowest
	// friction value (muddy has precedence over icy).

	node = sec->touching_thinglist; // things touching this sector
	while(node)
	{
		thing = node->m_thing;
		if(thing->player &&
			(thing->flags & (MobjFlag::NoGravity | MobjFlag::NoClip)) == MobjFlag{} &&
			thing->z <= sec->floorheight)
		{
			if((thing->friction == ORIG_FRICTION) || // normal friction?
				(f->friction < thing->friction))
			{
				thing->friction = f->friction;
				thing->movefactor = f->movefactor;
			}
		}
		node = node->m_snext;
	}
}


// killough 3/7/98 -- end generalized scroll effects

////////////////////////////////////////////////////////////////////////////
//
// FRICTION EFFECTS
//
// phares 3/12/98: Start of friction effects
//
// As the player moves, friction is applied by decreasing the x and y
// momentum values on each tic. By varying the percentage of decrease,
// we can simulate muddy or icy conditions. In mud, the player slows
// down faster. In ice, the player slows down more slowly.
//
// The amount of friction change is controlled by the length of a linedef
// with type 223. A length < 100 gives you mud. A length > 100 gives you ice.
//
// Also, each sector where these effects are to take place is given a
// new special type _______. Changing the type value at runtime allows
// these effects to be turned on or off.
//
// Sector boundaries present problems. The player should experience these
// friction changes only when his feet are touching the sector floor. At
// sector boundaries where floor height changes, the player can find
// himself still 'in' one sector, but with his feet at the floor level
// of the next sector (steps up or down). To handle this, Thinkers are used
// in icy/muddy sectors. These thinkers examine each object that is touching
// their sectors, looking for players whose feet are at the same level as
// their floors. Players satisfying this condition are given new friction
// values that are applied by the player movement code later.
//
// killough 8/28/98:
//
// Completely redid code, which did not need thinkers, and which put a heavy
// drag on CPU. Friction is now a property of sectors, NOT objects inside
// them. All objects, not just players, are affected by it, if they touch
// the sector's floor. Code simpler and faster, only calling on friction
// calculations when an object needs friction considered, instead of doing
// friction calculations on every sector during every tic.
//
// Although this -might- ruin Boom demo sync involving friction, it's the only
// way, short of code explosion, to fix the original design bug. Fixing the
// design bug in Boom's original friction code, while maintaining demo sync
// under every conceivable circumstance, would double or triple code size, and
// would require maintenance of buggy legacy code which is only useful for old
// demos. Doom demos, which are more important IMO, are not affected by this
// change.
//
/////////////////////////////
//
// Initialize the sectors where friction is increased or decreased

void P_ResolveFrictionFactor(fixed_t friction_factor, sector_t* sec)
{
	sec->friction = friction_factor;

	if(sec->friction > FRACUNIT)
		sec->friction = FRACUNIT;
	else if(sec->friction < 0)
		sec->friction = 0;

	if(sec->friction > ORIG_FRICTION) // ice
		sec->movefactor = ((0x10092 - sec->friction) * (0x70)) / 0x158;
	else
		sec->movefactor = ((sec->friction - 0xDB34) * (0xA)) / 0x80;

	if(sec->movefactor < 32)
		sec->movefactor = 32;

	sec->flags |= SectorFlag::Friction;
}

static void P_ApplySectorFriction(int tag, int value, int use_thinker)
{
	const int* id_p;
	int friction, movefactor;

	friction = (0x1EB8 * value) / 0x80 + 0xD000;

	// The following check might seem odd. At the time of movement,
	// the move distance is multiplied by 'friction/0x10000', so a
	// higher friction value actually means 'less friction'.

	if(friction > ORIG_FRICTION) // ice
		movefactor = ((0x10092 - friction) * (0x70)) / 0x158;
	else
		movefactor = ((friction - 0xDB34) * (0xA)) / 0x80;

	if(mbf_features)
	{
		// killough 8/28/98: prevent odd situations
		if(friction > FRACUNIT)
			friction = FRACUNIT;
		if(friction < 0)
			friction = 0;
		if(movefactor < 32)
			movefactor = 32;
	}

	FIND_SECTORS(id_p, tag)
	{
		// killough 8/28/98:
		//
		// Instead of spawning thinkers, which are slow and expensive,
		// modify the sector's own friction values. Friction should be
		// a property of sectors, not objects which reside inside them.
		// Original code scanned every object in every friction sector
		// on every tic, adjusting its friction, putting unnecessary
		// drag on CPU. New code adjusts friction of sector only once
		// at level startup, and then uses this friction value.

		//e6y: boom's friction code for boom compatibility
		if(use_thinker)
			Add_Friction(friction, movefactor, *id_p);

		sectors[*id_p].friction = friction;
		sectors[*id_p].movefactor = movefactor;
	}
}

extern "C" void P_SpawnCompatibleFriction(line_t* l)
{
	if(l->special == 223)
	{
		int value, use_thinker;

		value = P_AproxDistance(l->dx, l->dy) >> FRACBITS;
		use_thinker = !demo_compatibility && !mbf_features && !prboom_comp[std::to_underlying(PrboomComp::PrboomFriction)].state;

		P_ApplySectorFriction(l->special_args[0], value, use_thinker);
	}
}

extern "C" void P_SpawnZDoomFriction(line_t* l)
{
	if(l->special == std::to_underlying(ZDoomLineSpecial::SectorSetFriction))
	{
		int value;

		if(l->special_args[1])
			value = l->special_args[1] <= 200 ? l->special_args[1] : 200;
		else
			value = P_AproxDistance(l->dx, l->dy) >> FRACBITS;

		P_ApplySectorFriction(l->special_args[0], value, false);

		l->special = 0;
	}
}

static void P_SpawnFriction()
{
	int i;
	line_t* l = lines;

	for(i = 0; i < numlines; i++, l++)
		map_format.spawn_friction(l);
}

//
// phares 3/12/98: End of friction effects
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// PUSH/PULL EFFECT
//
// phares 3/20/98: Start of push/pull effects
//
// This is where push/pull effects are applied to objects in the sectors.
//
// There are four kinds of push effects
//
// 1) Pushing Away
//
//    Pushes you away from a point source defined by the location of an
//    MT_PUSH Thing. The force decreases linearly with distance from the
//    source. This force crosses sector boundaries and is felt w/in a circle
//    whose center is at the MT_PUSH. The force is felt only if the point
//    MT_PUSH can see the target object.
//
// 2) Pulling toward
//
//    Same as Pushing Away except you're pulled toward an MT_PULL point
//    source. This force crosses sector boundaries and is felt w/in a circle
//    whose center is at the MT_PULL. The force is felt only if the point
//    MT_PULL can see the target object.
//
// 3) Wind
//
//    Pushes you in a constant direction. Full force above ground, half
//    force on the ground, nothing if you're below it (water).
//
// 4) Current
//
//    Pushes you in a constant direction. No force above ground, full
//    force if on the ground or below it (water).
//
// The magnitude of the force is controlled by the length of a controlling
// linedef. The force vector for types 3 & 4 is determined by the angle
// of the linedef, and is constant.
//
// For each sector where these effects occur, the sector special type has
// to have the BoomSectorFlag::Push bit set. If this bit is turned off by a switch
// at run-time, the effect will not occur. The controlling sector for
// types 1 & 2 is the sector containing the MT_PUSH/MT_PULL Thing.


#define PUSH_FACTOR 7

/////////////////////////////
//
// Add a push thinker to the thinker list

static void Add_Pusher(PusherType type, int x_mag, int y_mag, mobj_t* source, int affectee)
{
	pusher_t* p = static_cast<pusher_t*>(Z_MallocLevel(sizeof *p));

	p->thinker.function = reinterpret_cast<think_t>(T_Pusher);
	p->source = source;
	p->type = static_cast<PusherType>(type);
	p->x_mag = x_mag >> FRACBITS;
	p->y_mag = y_mag >> FRACBITS;
	p->magnitude = P_AproxDistance(p->x_mag, p->y_mag);
	if(source) // point source exist?
	{
		p->radius = (p->magnitude) << (FRACBITS + 1); // where force goes to zero
		p->x = p->source->x;
		p->y = p->source->y;
	}
	p->affectee = affectee;
	P_AddThinker(&p->thinker);
}

/////////////////////////////
//
// PIT_PushThing determines the angle and magnitude of the effect.
// The object's x and y momentum values are changed.
//
// tmpusher belongs to the point source (MT_PUSH/MT_PULL).
//
// killough 10/98: allow to affect things besides players

pusher_t* tmpusher; // pusher structure for blockmap searches

static dboolean PIT_PushThing(mobj_t* thing)
{
	/* killough 10/98: made more general */
	if(!mbf_features
		? thing->player && (thing->flags & (MobjFlag::NoClip | MobjFlag::NoGravity)) == MobjFlag{}
		: (sentient(thing) || (thing->flags & MobjFlag::Shootable) != MobjFlag{}) &&
		(thing->flags & MobjFlag::NoClip) == MobjFlag{})
	{
		angle_t pushangle;
		fixed_t speed;
		fixed_t sx = tmpusher->x;
		fixed_t sy = tmpusher->y;

		speed = (tmpusher->magnitude -
			((P_AproxDistance(thing->x - sx, thing->y - sy)
				>> FRACBITS) >> 1)) << (FRACBITS - PUSH_FACTOR - 1);

		// killough 10/98: make magnitude decrease with square
		// of distance, making it more in line with real nature,
		// so long as it's still in range with original formula.
		//
		// Removes angular distortion, and makes effort required
		// to stay close to source, grow increasingly hard as you
		// get closer, as expected. Still, it doesn't consider z :(

		if(speed > 0 && mbf_features)
		{
			int x = (thing->x - sx) >> FRACBITS;
			int y = (thing->y - sy) >> FRACBITS;
			speed = (int)(((uint64_t)tmpusher->magnitude << 23) / (x * x + y * y + 1));
		}

		// If speed <= 0, you're outside the effective radius. You also have
		// to be able to see the push/pull source point.

		if(speed > 0 && P_CheckSight(thing, tmpusher->source))
		{
			pushangle = R_PointToAngle2(thing->x, thing->y, sx, sy);
			if(tmpusher->source->type == MobjType::Push)
				pushangle += ANG180; // away
			pushangle >>= ANGLETOFINESHIFT;
			thing->momx += FixedMul(speed, finecosine[pushangle]);
			thing->momy += FixedMul(speed, finesine[pushangle]);
			thing->intflags |= MobjIntFlag::Scrolling;
		}
	}
	return true;
}

/////////////////////////////
//
// T_Pusher looks for all objects that are inside the radius of
// the effect.
//

void T_Pusher(pusher_t* p)
{
	sector_t* sec;
	mobj_t* thing;
	msecnode_t* node;
	int xspeed, yspeed;
	int xl, xh, yl, yh, bx, by;
	int radius;
	int ht = 0;

	if(!allow_pushers)
		return;

	sec = sectors + p->affectee;

	// Be sure the special sector type is still turned on. If so, proceed.
	// Else, bail out; the sector type has been changed on us.

	if((sec->flags & SectorFlag::Push) == SectorFlag{})
		return;

	// For constant pushers (wind/current) there are 3 situations:
	//
	// 1) Affected Thing is above the floor.
	//
	//    Apply the full force if wind, no force if current.
	//
	// 2) Affected Thing is on the ground.
	//
	//    Apply half force if wind, full force if current.
	//
	// 3) Affected Thing is below the ground (underwater effect).
	//
	//    Apply no force if wind, full force if current.

	if(p->type == PusherType::Push)
	{
		// Seek out all pushable things within the force radius of this
		// point pusher. Crosses sectors, so use blockmap.

		tmpusher = p;       // MT_PUSH/MT_PULL point source
		radius = p->radius; // where force goes to zero
		tmbbox[std::to_underlying(BoxEdge::Top)] = p->y + radius;
		tmbbox[std::to_underlying(BoxEdge::Bottom)] = p->y - radius;
		tmbbox[std::to_underlying(BoxEdge::Right)] = p->x + radius;
		tmbbox[std::to_underlying(BoxEdge::Left)] = p->x - radius;

		xl = P_GetSafeBlockX(tmbbox[std::to_underlying(BoxEdge::Left)] - bmaporgx - MAXRADIUS);
		xh = P_GetSafeBlockX(tmbbox[std::to_underlying(BoxEdge::Right)] - bmaporgx + MAXRADIUS);
		yl = P_GetSafeBlockY(tmbbox[std::to_underlying(BoxEdge::Bottom)] - bmaporgy - MAXRADIUS);
		yh = P_GetSafeBlockY(tmbbox[std::to_underlying(BoxEdge::Top)] - bmaporgy + MAXRADIUS);
		for(bx = xl; bx <= xh; bx++)
			for(by = yl; by <= yh; by++)
				P_BlockThingsIterator(bx, by, PIT_PushThing);
		return;
	}

	// constant pushers p_wind and p_current

	if(sec->heightsec != -1) // special water sector?
		ht = sectors[sec->heightsec].floorheight;
	node = sec->touching_thinglist; // things touching this sector
	for(; node; node = node->m_snext)
	{
		thing = node->m_thing;
		if(!thing->player || (thing->flags & (MobjFlag::NoGravity | MobjFlag::NoClip)) != MobjFlag{})
			continue;
		if(p->type == PusherType::Wind)
		{
			if(sec->heightsec == -1)         // NOT special water sector
				if(thing->z > thing->floorz) // above ground
				{
					xspeed = p->x_mag; // full force
					yspeed = p->y_mag;
				}
				else // on ground
				{
					xspeed = (p->x_mag) >> 1; // half force
					yspeed = (p->y_mag) >> 1;
				}
			else // special water sector
			{
				if(thing->z > ht) // above ground
				{
					xspeed = p->x_mag; // full force
					yspeed = p->y_mag;
				}
				else if(thing->player->viewz < ht) // underwater
					xspeed = yspeed = 0;           // no force
				else                               // wading in water
				{
					xspeed = (p->x_mag) >> 1; // half force
					yspeed = (p->y_mag) >> 1;
				}
			}
		}
		else // p_current
		{
			if(sec->heightsec == -1)            // NOT special water sector
				if(thing->z > sec->floorheight) // above ground
					xspeed = yspeed = 0;        // no force
				else                            // on ground
				{
					xspeed = p->x_mag; // full force
					yspeed = p->y_mag;
				}
			else                         // special water sector
				if(thing->z > ht)        // above ground
					xspeed = yspeed = 0; // no force
				else                     // underwater
				{
					xspeed = p->x_mag; // full force
					yspeed = p->y_mag;
				}
		}
		thing->momx += xspeed << (FRACBITS - PUSH_FACTOR);
		thing->momy += yspeed << (FRACBITS - PUSH_FACTOR);
		thing->intflags |= MobjIntFlag::Scrolling;
	}
}

/////////////////////////////
//
// P_GetPushThing() returns a pointer to an MT_PUSH or MT_PULL thing,
// NULL otherwise.

mobj_t* P_GetPushThing(int s)
{
	mobj_t* thing;
	sector_t* sec;

	sec = sectors + s;
	thing = sec->thinglist;
	while(thing)
	{
		switch(thing->type)
		{
			case MobjType::Push:
			case MobjType::Pull:
				return thing;
			default:
				break;
		}
		thing = thing->snext;
	}
	return nullptr;
}

/////////////////////////////
//
// Initialize the sectors where pushers are present
//

extern "C" void P_SpawnCompatiblePusher(line_t* l)
{
	const int* id_p;
	mobj_t* thing;

	switch(l->special)
	{
		case 224: // wind
			FIND_SECTORS(id_p, l->special_args[0])
				Add_Pusher(PusherType::Wind, l->dx, l->dy, nullptr, *id_p);
			break;
		case 225: // current
			FIND_SECTORS(id_p, l->special_args[0])
				Add_Pusher(PusherType::Current, l->dx, l->dy, nullptr, *id_p);
			break;
		case 226: // push/pull
			FIND_SECTORS(id_p, l->special_args[0])
			{
				thing = P_GetPushThing(*id_p);
				if(thing) // No MT_P* means no effect
					Add_Pusher(PusherType::Push, l->dx, l->dy, thing, *id_p);
			}
			break;
	}
}

static void CalculatePushVector(line_t* l, int magnitude, int angle, fixed_t* dx, fixed_t* dy)
{
	if(l->special_args[3])
	{
		*dx = l->dx;
		*dy = l->dy;
		return;
	}

	angle = angle * (ANG180 >> 7); // 256 is 360
	angle >>= ANGLETOFINESHIFT;
	magnitude <<= FRACBITS;

	*dx = FixedMul(magnitude, finecosine[angle]);
	*dy = FixedMul(magnitude, finesine[angle]);
}

extern "C" void P_SpawnZDoomPusher(line_t* l)
{
	const int* id_p;
	mobj_t* thing;
	fixed_t dx, dy;

	switch(l->special)
	{
		case std::to_underlying(ZDoomLineSpecial::SectorSetWind): // wind
			CalculatePushVector(l, l->special_args[1], l->special_args[2], &dx, &dy);
			FIND_SECTORS(id_p, l->special_args[0])
				Add_Pusher(PusherType::Wind, dx, dy, nullptr, *id_p);
			l->special = 0;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetCurrent): // current
			CalculatePushVector(l, l->special_args[1], l->special_args[2], &dx, &dy);
			FIND_SECTORS(id_p, l->special_args[0])
				Add_Pusher(PusherType::Current, dx, dy, nullptr, *id_p);
			l->special = 0;
			break;
		case std::to_underlying(ZDoomLineSpecial::PointPushSetForce): // push/pull
			CalculatePushVector(l, l->special_args[2], 0, &dx, &dy);
			if(l->special_args[0])
			{
				// [RH] Find thing by sector
				FIND_SECTORS(id_p, l->special_args[0])
				{
					thing = P_GetPushThing(*id_p);
					if(thing) // No MT_P* means no effect
					{
						// [RH] Allow narrowing it down by tid
						if(!l->special_args[1] || l->special_args[1] == thing->tid)
							Add_Pusher(PusherType::Push, dx, dy, thing, *id_p);
					}
				}
			}
			else
			{
				// [RH] Find thing by tid
				thing_id_search_t search;

				dsda_ResetThingIDSearch(&search);
				while((thing = dsda_FindMobjFromThingID(l->special_args[1], &search)) != nullptr)
					if(thing->type == map_format.mt_push || thing->type == map_format.mt_pull)
						Add_Pusher(PusherType::Push, dx, dy, thing, thing->subsector->sector->iSectorID);
			}
			l->special = 0;
			break;
	}
}

static void P_SpawnPushers()
{
	int i;
	line_t* l = lines;

	for(i = 0; i < numlines; i++, l++)
		map_format.spawn_pusher(l);
}

//
// phares 3/20/98: End of Pusher effects
//
////////////////////////////////////////////////////////////////////////////

// heretic

#include "heretic/def.hpp"

enum struct AmbientCmd : int32_t
{
	Play,       // (sound)
	PlayAbsVol, // (sound, volume)
	PlayRelVol, // (sound, volume)
	Delay,      // (ticks)
	DelayRand,  // (andbits)
	End         // ()
};

int* LevelAmbientSfx[MAX_AMBIENT_SFX];
int* AmbSfxPtr;
int AmbSfxPtrIndex;
int AmbSfxCount;
int AmbSfxTics;
int AmbSfxVolume;

int AmbSndSeqInit[] = {
	// Startup
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq1[] = {
	// Scream
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb1),
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq2[] = {
	// Squish
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb2),
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq3[] = {
	// Drops
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb3),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::DelayRand), 31,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb7),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::DelayRand), 31,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb3),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::DelayRand), 31,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb7),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::DelayRand), 31,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb3),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::DelayRand), 31,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb7),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::DelayRand), 31,
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq4[] = {
	// SlowFootSteps
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb4),
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb4), -3,
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb4), -3,
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb4), -3,
	std::to_underlying(AmbientCmd::Delay), 15,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq5[] = {
	// Heartbeat
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb5),
	std::to_underlying(AmbientCmd::Delay), TICRATE,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb5),
	std::to_underlying(AmbientCmd::Delay), TICRATE,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb5),
	std::to_underlying(AmbientCmd::Delay), TICRATE,
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb5),
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq6[] = {
	// Bells
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb6),
	std::to_underlying(AmbientCmd::Delay), 17,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb6), -8,
	std::to_underlying(AmbientCmd::Delay), 17,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb6), -8,
	std::to_underlying(AmbientCmd::Delay), 17,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb6), -8,
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq7[] = {
	// Growl
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticBstsit),
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq8[] = {
	// Magic
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb8),
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq9[] = {
	// Laughter
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb9),
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb9), -4,
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb9), -4,
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb10), -4,
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb10), -4,
	std::to_underlying(AmbientCmd::Delay), 16,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb10), -4,
	std::to_underlying(AmbientCmd::End)
};
int AmbSndSeq10[] = {
	// FastFootsteps
	std::to_underlying(AmbientCmd::Play), std::to_underlying(SfxId::HereticAmb4),
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb4), -3,
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb4), -3,
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb4), -3,
	std::to_underlying(AmbientCmd::Delay), 8,
	std::to_underlying(AmbientCmd::PlayRelVol), std::to_underlying(SfxId::HereticAmb11), -3,
	std::to_underlying(AmbientCmd::End)
};

int* AmbientSfx[] = {
	AmbSndSeq1, // Scream
	AmbSndSeq2, // Squish
	AmbSndSeq3, // Drops
	AmbSndSeq4, // SlowFootsteps
	AmbSndSeq5, // Heartbeat
	AmbSndSeq6, // Bells
	AmbSndSeq7, // Growl
	AmbSndSeq8, // Magic
	AmbSndSeq9, // Laughter
	AmbSndSeq10 // FastFootsteps
};

int* TerrainTypes;

struct
{
	const char* name;
	int type;
} TerrainTypeDefs[2][6] =
{
	{
		{"FLTWAWA1", std::to_underlying(FloorType::Water)},
		{"FLTFLWW1", std::to_underlying(FloorType::Water)},
		{"FLTLAVA1", std::to_underlying(FloorType::Lava)},
		{"FLATHUH1", std::to_underlying(FloorType::Lava)},
		{"FLTSLUD1", std::to_underlying(FloorType::Sludge)},
		{"END", -1}
	},
	{
		{"X_005", std::to_underlying(FloorType::Water)},
		{"X_001", std::to_underlying(FloorType::Lava)},
		{"X_009", std::to_underlying(FloorType::Sludge)},
		{"F_033", std::to_underlying(FloorType::Ice)},
		{"END", -1}
	}
};

mobj_t LavaInflictor;

void P_AddAmbientSfx(int sequence)
{
	if(AmbSfxCount == MAX_AMBIENT_SFX)
	{
		Log::Fatal("Too many ambient sound sequences");
	}
	LevelAmbientSfx[AmbSfxCount++] = AmbientSfx[sequence];
}

void P_InitAmbientSound()
{
	AmbSfxCount = 0;
	AmbSfxVolume = 0;
	AmbSfxTics = 10 * TICRATE;
	AmbSfxPtrIndex = -1;
	AmbSfxPtr = AmbSndSeqInit;
}

void P_AmbientSound()
{
	AmbientCmd cmd;
	SfxId sound;
	dboolean done;

	if(!AmbSfxCount)
	{
		// No ambient sound sequences on current level
		return;
	}
	if(--AmbSfxTics)
	{
		return;
	}
	done = false;
	do
	{
		cmd = static_cast<AmbientCmd>(*AmbSfxPtr++);
		switch(cmd)
		{
			case AmbientCmd::Play:
				AmbSfxVolume = P_Random(RandomClass::Heretic) >> 2;
				S_StartAmbientSound(nullptr, static_cast<SfxId>(*AmbSfxPtr++), AmbSfxVolume);
				break;
			case AmbientCmd::PlayAbsVol:
				sound = static_cast<SfxId>(*AmbSfxPtr++);
				AmbSfxVolume = *AmbSfxPtr++;
				S_StartAmbientSound(nullptr, sound, AmbSfxVolume);
				break;
			case AmbientCmd::PlayRelVol:
				sound = static_cast<SfxId>(*AmbSfxPtr++);
				AmbSfxVolume += *AmbSfxPtr++;
				if(AmbSfxVolume < 0)
				{
					AmbSfxVolume = 0;
				}
				else if(AmbSfxVolume > 127)
				{
					AmbSfxVolume = 127;
				}
				S_StartAmbientSound(nullptr, sound, AmbSfxVolume);
				break;
			case AmbientCmd::Delay:
				AmbSfxTics = *AmbSfxPtr++;
				done = true;
				break;
			case AmbientCmd::DelayRand:
				AmbSfxTics = P_Random(RandomClass::Heretic) & (*AmbSfxPtr++);
				done = true;
				break;
			case AmbientCmd::End:
				AmbSfxTics = 6 * TICRATE + P_Random(RandomClass::Heretic);
				AmbSfxPtrIndex = P_Random(RandomClass::Heretic) % AmbSfxCount;
				AmbSfxPtr = LevelAmbientSfx[AmbSfxPtrIndex];
				done = true;
				break;
			default:
				Log::Fatal("P_AmbientSound: Unknown afxcmd {}", std::to_underlying(cmd));
				break;
		}
	}
	while(done == false);
}

void P_InitLava()
{
	if(!raven) return;

	memset(&LavaInflictor, 0, sizeof(mobj_t));
	LavaInflictor.type = static_cast<MobjType>(g_lava_type);
	LavaInflictor.flags2 = MobjFlag2::FireDamage | MobjFlag2::NoDmgThrust;
}

void P_InitTerrainTypes()
{
	int i;
	int lump;
	int size;

	if(!raven) return;

	size = (numflats + 1) * sizeof(int);
	TerrainTypes = static_cast<int*>(Z_Malloc(size));
	memset(TerrainTypes, 0, size);
	for(i = 0; TerrainTypeDefs[hexen][i].type != -1; i++)
	{
		lump = W_CheckNumForName2(TerrainTypeDefs[hexen][i].name, LumpNamespace::Flats);
		if(lump != LUMP_NOT_FOUND)
		{
			TerrainTypes[lump - firstflat] = TerrainTypeDefs[hexen][i].type;
		}
	}
}

extern "C" void P_CrossHereticSpecialLine(line_t* line, int side, mobj_t* thing, dboolean bossaction)
{
	if(!thing->player)
	{
		// Check if trigger allowed by non-player mobj
		switch(line->special)
		{
			case 39: // Trigger_TELEPORT
			case 97: // Retrigger_TELEPORT
			case 4:  // Trigger_Raise_Door
				//case 10:      // PLAT DOWN-WAIT-UP-STAY TRIGGER
				//case 88:      // PLAT DOWN-WAIT-UP-STAY RETRIGGER
				break;
			default:
				return;
				break;
		}
	}
	switch(line->special)
	{
		//====================================================
		// TRIGGERS
		//====================================================
		case 2: // Open Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldOpen));
			line->special = 0;
			break;
		case 3: // Close Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldClose));
			line->special = 0;
			break;
		case 4: // Raise Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldNormal));
			line->special = 0;
			break;
		case 5: // Raise Floor
			EV_DoFloor(line, FloorKind::RaiseFloor);
			line->special = 0;
			break;
		case 6: // Fast Ceiling Crush & Raise
			EV_DoCeiling(line, CeilingKind::FastCrushAndRaise);
			line->special = 0;
			break;
		case 8: // Trigger_Build_Stairs (8 pixel steps)
			EV_BuildStairs(line, StairType::HereticBuild8);
			line->special = 0;
			break;
		case 106: // Trigger_Build_Stairs_16 (16 pixel steps)
			EV_BuildStairs(line, StairType::HereticTurbo16);
			line->special = 0;
			break;
		case 10: // PlatDownWaitUp
			EV_DoPlat(line, PlatType::DownWaitUpStay, 0);
			line->special = 0;
			break;
		case 12: // Light Turn On - brightest near
			EV_LightTurnOn(line, 0);
			line->special = 0;
			break;
		case 13: // Light Turn On 255
			EV_LightTurnOn(line, 255);
			line->special = 0;
			break;
		case 16: // Close Door 30
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldClose30ThenOpen));
			line->special = 0;
			break;
		case 17: // Start Light Strobing
			EV_StartLightStrobing(line);
			line->special = 0;
			break;
		case 19: // Lower Floor
			EV_DoFloor(line, FloorKind::LowerFloor);
			line->special = 0;
			break;
		case 22: // Raise floor to nearest height and change texture
			EV_DoPlat(line, PlatType::RaiseToNearestAndChange, 0);
			line->special = 0;
			break;
		case 25: // Ceiling Crush and Raise
			EV_DoCeiling(line, CeilingKind::CrushAndRaise);
			line->special = 0;
			break;
		case 30: // Raise floor to shortest texture height
			// on either side of lines
			EV_DoFloor(line, FloorKind::RaiseToTexture);
			line->special = 0;
			break;
		case 35: // Lights Very Dark
			EV_LightTurnOn(line, 35);
			line->special = 0;
			break;
		case 36: // Lower Floor (TURBO)
			EV_DoFloor(line, FloorKind::TurboLower);
			line->special = 0;
			break;
		case 37: // LowerAndChange
			EV_DoFloor(line, FloorKind::LowerAndChange);
			line->special = 0;
			break;
		case 38: // Lower Floor To Lowest
			EV_DoFloor(line, FloorKind::LowerFloorToLowest);
			line->special = 0;
			break;
		case 39: // TELEPORT!
			map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Vanilla);
			line->special = 0;
			break;
		case 40: // RaiseCeilingLowerFloor
			EV_DoCeiling(line, CeilingKind::RaiseToHighest);
			EV_DoFloor(line, FloorKind::LowerFloorToLowest);
			line->special = 0;
			break;
		case 44: // Ceiling Crush
			EV_DoCeiling(line, CeilingKind::LowerAndCrush);
			line->special = 0;
			break;
		case 52: // EXIT!
			G_ExitLevel(0);
			line->special = 0;
			break;
		case 53: // Perpetual Platform Raise
			EV_DoPlat(line, PlatType::PerpetualRaise, 0);
			line->special = 0;
			break;
		case 54: // Platform Stop
			EV_StopPlat(line);
			line->special = 0;
			break;
		case 56: // Raise Floor Crush
			EV_DoFloor(line, FloorKind::RaiseFloorCrush);
			line->special = 0;
			break;
		case 57: // Ceiling Crush Stop
			EV_CeilingCrushStop(line);
			line->special = 0;
			break;
		case 58: // Raise Floor 24
			EV_DoFloor(line, FloorKind::RaiseFloor24);
			line->special = 0;
			break;
		case 59: // Raise Floor 24 And Change
			EV_DoFloor(line, FloorKind::RaiseFloor24AndChange);
			line->special = 0;
			break;
		case 104: // Turn lights off in sector(tag)
			EV_TurnTagLightsOff(line);
			line->special = 0;
			break;
		case 105: // Trigger_SecretExit
			G_SecretExitLevel(0);
			line->special = 0;
			break;

		//====================================================
		// RE-DOABLE TRIGGERS
		//====================================================

		case 72: // Ceiling Crush
			EV_DoCeiling(line, CeilingKind::LowerAndCrush);
			break;
		case 73: // Ceiling Crush and Raise
			EV_DoCeiling(line, CeilingKind::CrushAndRaise);
			break;
		case 74: // Ceiling Crush Stop
			EV_CeilingCrushStop(line);
			break;
		case 75: // Close Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldClose));
			break;
		case 76: // Close Door 30
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldClose30ThenOpen));
			break;
		case 77: // Fast Ceiling Crush & Raise
			EV_DoCeiling(line, CeilingKind::FastCrushAndRaise);
			break;
		case 79: // Lights Very Dark
			EV_LightTurnOn(line, 35);
			break;
		case 80: // Light Turn On - brightest near
			EV_LightTurnOn(line, 0);
			break;
		case 81: // Light Turn On 255
			EV_LightTurnOn(line, 255);
			break;
		case 82: // Lower Floor To Lowest
			EV_DoFloor(line, FloorKind::LowerFloorToLowest);
			break;
		case 83: // Lower Floor
			EV_DoFloor(line, FloorKind::LowerFloor);
			break;
		case 84: // LowerAndChange
			EV_DoFloor(line, FloorKind::LowerAndChange);
			break;
		case 86: // Open Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldOpen));
			break;
		case 87: // Perpetual Platform Raise
			EV_DoPlat(line, PlatType::PerpetualRaise, 0);
			break;
		case 88: // PlatDownWaitUp
			EV_DoPlat(line, PlatType::DownWaitUpStay, 0);
			break;
		case 89: // Platform Stop
			EV_StopPlat(line);
			break;
		case 90: // Raise Door
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldNormal));
			break;
		case 100: // Retrigger_Raise_Door_Turbo
			EV_DoDoor(line, static_cast<VerticalDoorType>(VerticalDoorType::VldNormalTurbo));
			break;
		case 91: // Raise Floor
			EV_DoFloor(line, FloorKind::RaiseFloor);
			break;
		case 92: // Raise Floor 24
			EV_DoFloor(line, FloorKind::RaiseFloor24);
			break;
		case 93: // Raise Floor 24 And Change
			EV_DoFloor(line, FloorKind::RaiseFloor24AndChange);
			break;
		case 94: // Raise Floor Crush
			EV_DoFloor(line, FloorKind::RaiseFloorCrush);
			break;
		case 95: // Raise floor to nearest height and change texture
			EV_DoPlat(line, PlatType::RaiseToNearestAndChange, 0);
			break;
		case 96: // Raise floor to shortest texture height
			// on either side of lines
			EV_DoFloor(line, FloorKind::RaiseToTexture);
			break;
		case 97: // TELEPORT!
			map_format.ev_teleport(0, line->special_args[0], line, side, thing, TeleportFlag::Vanilla);
			break;
		case 98: // Lower Floor (TURBO)
			EV_DoFloor(line, FloorKind::TurboLower);
			break;
	}
}

extern "C" void P_PlayerInHereticSector(player_t* player, sector_t* sector)
{
	static int pushTab[5] = {
		2048 * 5,
		2048 * 10,
		2048 * 25,
		2048 * 30,
		2048 * 35
	};

	switch(sector->special)
	{
		case 7: // Damage_Sludge
			if(!(leveltime & 31))
			{
				P_DamageMobj(player->mo, nullptr, nullptr, 4);
			}
			break;
		case 5: // Damage_LavaWimpy
			if(!(leveltime & 15))
			{
				P_DamageMobj(player->mo, &LavaInflictor, nullptr, 5);
				P_HitFloor(player->mo);
			}
			break;
		case 16: // Damage_LavaHefty
			if(!(leveltime & 15))
			{
				P_DamageMobj(player->mo, &LavaInflictor, nullptr, 8);
				P_HitFloor(player->mo);
			}
			break;
		case 4: // Scroll_EastLavaDamage
			P_Thrust(player, 0, 2048 * 28);
			if(!(leveltime & 15))
			{
				P_DamageMobj(player->mo, &LavaInflictor, nullptr, 5);
				P_HitFloor(player->mo);
			}
			break;
		case 9: // SecretArea
			P_CollectSecretVanilla(sector, player);

			break;
		case 11: // Exit_SuperDamage (DOOM E1M8 finale)
			break;

		case 25:
		case 26:
		case 27:
		case 28:
		case 29: // Scroll_North
			P_Thrust(player, ANG90, pushTab[sector->special - 25]);
			break;
		case 20:
		case 21:
		case 22:
		case 23:
		case 24: // Scroll_East
			P_Thrust(player, 0, pushTab[sector->special - 20]);
			break;
		case 30:
		case 31:
		case 32:
		case 33:
		case 34: // Scroll_South
			P_Thrust(player, ANG270, pushTab[sector->special - 30]);
			break;
		case 35:
		case 36:
		case 37:
		case 38:
		case 39: // Scroll_West
			P_Thrust(player, ANG180, pushTab[sector->special - 35]);
			break;

		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
			// Wind specials are handled in (P_mobj):P_XYMovement
			break;

		case 15: // Friction_Low
			// Only used in (P_mobj):P_XYMovement and (P_user):P_Thrust
			break;

		default:
			Log::Fatal("P_PlayerInSpecialSector: "
				"unknown special {}", sector->special);
	}
}

void P_SpawnLineSpecials()
{
	int i;

	if(!heretic) return;

	//
	//      Init line EFFECTs
	//

	numlinespecials = 0;
	for(i = 0; i < numlines; i++)
		switch(lines[i].special)
		{
			case 48: // Effect_Scroll_Left
			case 99: // Effect_Scroll_Right
				linespeciallist[numlinespecials] = &lines[i];
				numlinespecials++;
				break;
		}
}

// hexen

#define MAX_TAGGED_LINES 64

static struct
{
	line_t* line;
	int lineTag;
} TaggedLines[MAX_TAGGED_LINES];

static int TaggedLineCount;

void P_PlayerOnSpecialFlat(player_t* player, FloorType floorType)
{
	if(player->mo->z != player->mo->floorz)
	{
		// Player is not touching the floor
		return;
	}
	switch(static_cast<FloorType>(floorType))
	{
		case FloorType::Lava:
			if(!(leveltime & 31))
			{
				P_DamageMobj(player->mo, &LavaInflictor, nullptr, 10);
				S_StartMobjSound(player->mo, SfxId::HexenLavaSizzle);
			}
			break;
		default:
			break;
	}
}

line_t* P_FindLine(int lineTag, int* searchPosition)
{
	int i;

	for(i = *searchPosition + 1; i < TaggedLineCount; i++)
	{
		if(TaggedLines[i].lineTag == lineTag)
		{
			*searchPosition = i;
			return TaggedLines[i].line;
		}
	}
	*searchPosition = -1;
	return nullptr;
}

dboolean EV_SectorSoundChange(byte* args)
{
	const int* id_p;
	dboolean rtn;

	if(!args[0])
	{
		return false;
	}
	rtn = false;
	FIND_SECTORS(id_p, args[0])
	{
		sectors[*id_p].seqType = static_cast<SeqType>(args[1]);
		rtn = true;
	}
	return rtn;
}

char LockedBuffer[80];

static dboolean CheckedLockedDoor(mobj_t* mo, byte lock)
{
	extern char* TextKeyMessages[11];

	if(!mo->player)
	{
		return false;
	}
	if(!lock)
	{
		return true;
	}
	if(!mo->player->cards[lock - 1])
	{
		snprintf(LockedBuffer, sizeof(LockedBuffer),
			"YOU NEED THE %s\n", TextKeyMessages[lock - 1]);
		P_SetMessage(mo->player, LockedBuffer, true);
		S_StartMobjSound(mo, SfxId::HexenDoorLocked);
		return false;
	}
	return true;
}

dboolean EV_LineSearchForPuzzleItem(line_t* line, byte* args, mobj_t* mo)
{
	player_t* player;
	int i;
	int type;
	ArtiType arti;

	if(!mo)
		return false;
	player = mo->player;
	if(!player)
		return false;

	// Search player's inventory for puzzle items
	for(i = 0; i < player->artifactCount; i++)
	{
		arti = static_cast<ArtiType>(player->inventory[i].type);
		type = std::to_underlying(arti) - std::to_underlying(ArtiType::HexenFirstpuzzitem);
		if(type < 0)
			continue;
		if(type == line->special_args[0])
		{
			// A puzzle item was found for the line
			if(P_UseArtifact(player, arti))
			{
				// A puzzle item was found for the line
				P_PlayerRemoveArtifact(player, i);
				if(player == &players[consoleplayer])
				{
					if(arti < ArtiType::HexenFirstpuzzitem)
					{
						S_StartVoidSound(SfxId::HexenArtifactUse);
					}
					else
					{
						S_StartVoidSound(SfxId::HexenPuzzleSuccess);
					}
					ArtifactFlash = 4;
				}
				return true;
			}
		}
	}
	return false;
}

extern "C" dboolean P_TestActivateZDoomLine(line_t* line, mobj_t* mo, int side, LineActivation activationType)
{
	LineActivation lineActivation;

	lineActivation = line->activation;

	if((line->flags & LineFlag::FirstSideOnly) != LineFlag{} && side)
	{
		return false;
	}

	if(
		line->special == std::to_underlying(ZDoomLineSpecial::Teleport) &&
		(lineActivation & LineActivation::Cross) != LineActivation{} &&
		activationType == LineActivation::ProjectileCross &&
		mo && (mo->flags & MobjFlag::Missile) != MobjFlag{}
	)
	{
		// Let missiles use regular player teleports
		lineActivation |= LineActivation::ProjectileCross;
	}

	if(activationType == LineActivation::Use || activationType == LineActivation::UseBack)
	{
		// TODO: possible "check switch range" mapinfo flag
		if((line->flags & LineFlag::CheckSwitchRange) != LineFlag{} && !P_CheckSwitchRange(line, mo, side))
		{
			return false;
		}
	}

	if(activationType == LineActivation::Use &&
		(lineActivation & LineActivation::MonsterUse) != LineActivation{} &&
		mo && !mo->player && (mo->flags2 & MobjFlag2::CanUseWalls) != MobjFlag2{})
	{
		return true;
	}

	if(activationType == LineActivation::Push &&
		(lineActivation & LineActivation::MonsterPush) != LineActivation{} &&
		mo && !mo->player && (mo->flags2 & MobjFlag2::PushWall) != MobjFlag2{})
	{
		return true;
	}

	if((lineActivation & activationType) == LineActivation{})
	{
		if(activationType != LineActivation::MonsterCross || lineActivation != LineActivation::Cross)
		{
			return false;
		}
	}

	if(activationType == LineActivation::AnyCross)
	{
		return true;
	}

	if(
		mo && !mo->player &&
		(mo->flags & MobjFlag::Missile) == MobjFlag{} &&
		(line->flags & LineFlag::MonstersCanActivate) == LineFlag{} &&
		(activationType != LineActivation::MonsterCross || (lineActivation & LineActivation::MonsterCross) == LineActivation{})
	)
	{
		dboolean noway = true;

		// [RH] monsters' ability to activate this line depends on its type
		// In Hexen, only MCROSS lines could be activated by monsters. With
		// lax activation checks, monsters can also activate certain lines
		// even without them being marked as monster activate-able. This is
		// the default for non-Hexen maps in Hexen format.
		// TODO: possible "check switch range" mapinfo flag

		if((activationType == LineActivation::Use || activationType == LineActivation::Push) && (line->flags & LineFlag::Secret) != LineFlag{})
			return false; // never open secret doors

		switch(activationType)
		{
			case LineActivation::Use:
			case LineActivation::Push:
				switch(line->special)
				{
					case std::to_underlying(ZDoomLineSpecial::DoorRaise):
						if(line->special_args[0] == 0 && line->special_args[1] < 64)
							noway = false;
						break;
					case std::to_underlying(ZDoomLineSpecial::Teleport):
					case std::to_underlying(ZDoomLineSpecial::TeleportNoFog):
						noway = false;
				}
				break;

			case LineActivation::MonsterCross:
				if((lineActivation & LineActivation::MonsterCross) == LineActivation{})
				{
					switch(line->special)
					{
						case std::to_underlying(ZDoomLineSpecial::DoorRaise):
							if(line->special_args[1] >= 64)
								break;
						case std::to_underlying(ZDoomLineSpecial::Teleport):
						case std::to_underlying(ZDoomLineSpecial::TeleportNoFog):
						case std::to_underlying(ZDoomLineSpecial::TeleportLine):
						case std::to_underlying(ZDoomLineSpecial::PlatDownWaitUpStayLip):
						case std::to_underlying(ZDoomLineSpecial::PlatDownWaitUpStay):
							noway = false;
					}
				}
				else
					noway = false;
				break;

			default:
				noway = false;
		}
		return !noway;
	}

	if(
		activationType == LineActivation::MonsterCross &&
		(lineActivation & LineActivation::MonsterCross) == LineActivation{} &&
		(line->flags & LineFlag::MonstersCanActivate) == LineFlag{}
	)
	{
		return false;
	}

	return true;
}

extern "C" dboolean P_TestActivateHexenLine(line_t* line, mobj_t* mo, int side, LineActivation activationType)
{
	LineActivation lineActivation;

	lineActivation = line->activation;

	if(lineActivation != activationType)
	{
		return false;
	}

	if(!mo->player && (mo->flags & MobjFlag::Missile) == MobjFlag{})
	{
		if(lineActivation != LineActivation::MonsterCross)
		{
			// currently, monsters can only activate the MCROSS activation type
			return false;
		}
		if((line->flags & LineFlag::Secret) != LineFlag{})
			return false; // never open secret doors
	}

	return true;
}

dboolean P_ActivateLine(line_t* line, mobj_t* mo, int side, LineActivation activationType)
{
	dboolean repeat;
	dboolean buttonSuccess;

	if(!map_format.test_activate_line(line, mo, side, activationType))
	{
		return false;
	}

	if(line->locknumber)
	{
		dboolean legacy;

		switch(line->special)
		{
			case std::to_underlying(ZDoomLineSpecial::DoorClose):
			case std::to_underlying(ZDoomLineSpecial::DoorOpen):
			case std::to_underlying(ZDoomLineSpecial::DoorRaise):
			case std::to_underlying(ZDoomLineSpecial::DoorLockedRaise):
			case std::to_underlying(ZDoomLineSpecial::DoorCloseWaitOpen):
			case std::to_underlying(ZDoomLineSpecial::DoorWaitRaise):
			case std::to_underlying(ZDoomLineSpecial::DoorWaitClose):
			case std::to_underlying(ZDoomLineSpecial::GenericDoor):
				legacy = true;
				break;
			default:
				legacy = false;
		}

		if(!P_CheckKeys(mo, static_cast<ZDoomLock>(line->locknumber), legacy))
		{
			return false;
		}
	}

	repeat = (line->flags & LineFlag::RepeatSpecial) != LineFlag{};

	buttonSuccess =
		map_format.execute_line_special(line->special, line->special_args, line, side, mo);

	if(!repeat && buttonSuccess)
	{
		// clear the special on non-retriggerable lines
		line->special = 0;
	}

	if(buttonSuccess && (line->activation & map_format.switch_activation) != LineActivation{})
	{
		P_ChangeSwitchTexture(line, repeat);
	}

	return true;
}

extern "C" void P_PlayerInHexenSector(player_t* player, sector_t* sector)
{
	static int pushTab[3] = {
		2048 * 5,
		2048 * 10,
		2048 * 25
	};

	switch(sector->special)
	{
		case 9: // SecretArea
			P_CollectSecretVanilla(sector, player);
			break;

		case 201:
		case 202:
		case 203: // Scroll_North_xxx
			P_Thrust(player, ANG90, pushTab[sector->special - 201]);
			break;
		case 204:
		case 205:
		case 206: // Scroll_East_xxx
			P_Thrust(player, 0, pushTab[sector->special - 204]);
			break;
		case 207:
		case 208:
		case 209: // Scroll_South_xxx
			P_Thrust(player, ANG270, pushTab[sector->special - 207]);
			break;
		case 210:
		case 211:
		case 212: // Scroll_West_xxx
			P_Thrust(player, ANG180, pushTab[sector->special - 210]);
			break;
		case 213:
		case 214:
		case 215: // Scroll_NorthWest_xxx
			P_Thrust(player, ANG90 + ANG45, pushTab[sector->special - 213]);
			break;
		case 216:
		case 217:
		case 218: // Scroll_NorthEast_xxx
			P_Thrust(player, ANG45, pushTab[sector->special - 216]);
			break;
		case 219:
		case 220:
		case 221: // Scroll_SouthEast_xxx
			P_Thrust(player, ANG270 + ANG45, pushTab[sector->special - 219]);
			break;
		case 222:
		case 223:
		case 224: // Scroll_SouthWest_xxx
			P_Thrust(player, ANG180 + ANG45, pushTab[sector->special - 222]);
			break;

		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
			// Wind specials are handled in (P_mobj):P_XYMovement
			break;

		case 26: // Stairs_Special1
		case 27: // Stairs_Special2
			// Used in (P_floor):ProcessStairSector
			break;

		case 198: // Lightning Special
		case 199: // Lightning Flash special
		case 200: // Sky2
			// Used in (R_plane):R_Drawplanes
			break;
		default:
			Log::Fatal("P_PlayerInSpecialSector: "
				"unknown special {}", sector->special);
	}
}

#include "hexen/a_action.hpp"
#include "hexen/p_anim.hpp"
#include "hexen/p_things.hpp"
#include "hexen/p_acs.hpp"
#include "hexen/po_man.hpp"

static dboolean P_ArgToCrushType(int arg)
{
	return arg == 1 ? false : arg == 2 ? true : hexen;
}

static CrushMode P_ArgToCrushMode(int arg, dboolean slowdown)
{
	static const CrushMode map[] = {CrushMode::Doom, CrushMode::Hexen, CrushMode::Slowdown};

	if(arg >= 1 && arg <= 3) return map[arg - 1];

	return hexen ? CrushMode::Hexen : slowdown ? CrushMode::Slowdown : CrushMode::Doom;
}

static int P_ArgToCrush(int arg)
{
	return (arg > 0) ? arg : NO_CRUSH;
}

static byte P_ArgToChange(int arg)
{
	static const byte ChangeMap[8] = {0, 1, 5, 3, 7, 2, 6, 0};

	return (arg >= 0 && arg < 8) ? ChangeMap[arg] : 0;
}

static fixed_t P_ArgToSpeed(fixed_t arg)
{
	return arg * FRACUNIT / 8;
}

static fixed_t P_ArgsToFixed(fixed_t arg_i, fixed_t arg_f)
{
	return (arg_i << FRACBITS) + (arg_f << FRACBITS) / 100;
}

static angle_t P_ArgToAngle(angle_t arg)
{
	return arg * (ANG180 / 128);
}

extern "C" void P_NoiseAlert(mobj_t* target, mobj_t* emitter);
extern "C" dboolean P_ExecuteZDoomLineSpecial(int special, int* args, line_t* line, int side, mobj_t* mo)
{
	dboolean buttonSuccess = false;

	switch(special)
	{
		case std::to_underlying(ZDoomLineSpecial::DoorClose):
			buttonSuccess = EV_DoZDoomDoor(VerticalDoorType::CloseDoor, line, mo, args[0],
				args[1], 0, static_cast<ZDoomLock>(0), args[2], false, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::DoorOpen):
			buttonSuccess = EV_DoZDoomDoor(VerticalDoorType::OpenDoor, line, mo, args[0],
				args[1], 0, static_cast<ZDoomLock>(0), args[2], false, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::DoorRaise):
			buttonSuccess = EV_DoZDoomDoor(VerticalDoorType::Normal, line, mo, args[0],
				args[1], args[2], static_cast<ZDoomLock>(0), args[3], false, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::DoorLockedRaise):
			buttonSuccess = EV_DoZDoomDoor(args[2] ? VerticalDoorType::Normal : VerticalDoorType::OpenDoor, line, mo, args[0],
				args[1], args[2], static_cast<ZDoomLock>(args[3]), args[4], false, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::DoorCloseWaitOpen):
			buttonSuccess = EV_DoZDoomDoor(VerticalDoorType::GenCdO, line, mo, args[0],
				args[1], args[2] * TICRATE / 8, static_cast<ZDoomLock>(0), args[3], false, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::DoorWaitRaise):
			buttonSuccess = EV_DoZDoomDoor(VerticalDoorType::WaitRaiseDoor, line, mo, args[0],
				args[1], args[2], static_cast<ZDoomLock>(0), args[4], false, args[3]);
			break;
		case std::to_underlying(ZDoomLineSpecial::DoorWaitClose):
			buttonSuccess = EV_DoZDoomDoor(VerticalDoorType::WaitCloseDoor, line, mo, args[0],
				args[1], 0, static_cast<ZDoomLock>(0), args[3], false, args[2]);
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericDoor):
		{
			int tag, lightTag;
			VerticalDoorType type;
			dboolean boomgen = false;

			switch(args[2] & 63)
			{
				case 0:
					type = VerticalDoorType::Normal;
					break;
				case 1:
					type = VerticalDoorType::OpenDoor;
					break;
				case 2:
					type = VerticalDoorType::GenCdO;
					break;
				case 3:
					type = VerticalDoorType::CloseDoor;
					break;
				default:
					return 0;
			}

			// Boom doesn't allow manual generalized doors to be activated while they move
			if(args[2] & 64)
				boomgen = true;

			if(args[2] & 128)
			{
				tag = 0;
				lightTag = args[0];
			}
			else
			{
				tag = args[0];
				lightTag = 0;
			}

			buttonSuccess = EV_DoZDoomDoor(type, line, mo, tag, args[1],
				args[3] * TICRATE / 8, static_cast<ZDoomLock>(args[4]), lightTag, boomgen, 0);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::PillarBuild):
			buttonSuccess = EV_DoZDoomPillar(PillarType::Build, line, args[0], P_ArgToSpeed(args[1]),
				args[2], 0, NO_CRUSH, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::PillarBuildAndCrush):
			buttonSuccess = EV_DoZDoomPillar(PillarType::Build, line, args[0], P_ArgToSpeed(args[1]),
				args[2], 0, args[3], P_ArgToCrushType(args[4]));
			break;
		case std::to_underlying(ZDoomLineSpecial::PillarOpen):
			buttonSuccess = EV_DoZDoomPillar(PillarType::Open, line, args[0], P_ArgToSpeed(args[1]),
				args[2], args[3], NO_CRUSH, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::ElevatorMoveToFloor):
			buttonSuccess = EV_DoZDoomElevator(line, ElevatorType::Current, P_ArgToSpeed(args[1]),
				0, args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ElevatorRaiseToNearest):
			buttonSuccess = EV_DoZDoomElevator(line, ElevatorType::Up, P_ArgToSpeed(args[1]),
				0, args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ElevatorLowerToNearest):
			buttonSuccess = EV_DoZDoomElevator(line, ElevatorType::Down, P_ArgToSpeed(args[1]),
				0, args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorAndCeilingLowerByValue):
			buttonSuccess = EV_DoZDoomElevator(line, ElevatorType::Lower, P_ArgToSpeed(args[1]),
				args[2], args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorAndCeilingRaiseByValue):
			buttonSuccess = EV_DoZDoomElevator(line, ElevatorType::Raise, P_ArgToSpeed(args[1]),
				args[2], args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerByValue):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerByValue, line, args[0], args[1], args[2],
				NO_CRUSH, P_ArgToChange(args[3]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerToLowest):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerToLowest, line, args[0], args[1], 0,
				NO_CRUSH, P_ArgToChange(args[2]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerToHighest):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerToHighest, line, args[0], args[1],
				args[2] - 128, NO_CRUSH, 0, false, args[3] == 1);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerToHighestEe):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerToHighest, line, args[0], args[1], 0,
				NO_CRUSH, P_ArgToChange(args[2]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerToNearest):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerToNearest, line, args[0], args[1], 0,
				NO_CRUSH, P_ArgToChange(args[2]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseByValue):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseByValue, line, args[0], args[1], args[2],
				P_ArgToCrush(args[4]), P_ArgToChange(args[3]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseToHighest):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseToHighest, line, args[0], args[1], 0,
				P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseToNearest):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseToNearest, line, args[0], args[1], 0,
				P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseToLowest):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseToLowest, line, args[0], 2, 0,
				P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseAndCrush):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseAndCrush, line, args[0], args[1], 0,
				args[2], 0, P_ArgToCrushType(args[3]), false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseAndCrushdoom):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseAndCrushDoom, line, args[0], args[1], 0,
				args[2], 0, P_ArgToCrushType(args[3]), false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseByValueTimes8):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseByValue, line, args[0], args[1], args[2] * 8,
				P_ArgToCrush(args[4]), P_ArgToChange(args[3]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerByValueTimes8):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerByValue, line, args[0], args[1], args[2] * 8,
				NO_CRUSH, P_ArgToChange(args[3]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerInstant):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerInstant, line, args[0], 0, args[2] * 8,
				NO_CRUSH, P_ArgToChange(args[3]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseInstant):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseInstant, line, args[0], 0, args[2] * 8,
				P_ArgToCrush(args[4]), P_ArgToChange(args[3]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorToCeilingInstant):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerToCeiling, line, args[0], 0, args[3],
				P_ArgToCrush(args[2]), P_ArgToChange(args[1]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorMoveToValue):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorMoveToValue, line, args[0], args[1],
				args[2] * (args[3] ? -1 : 1),
				NO_CRUSH, P_ArgToChange(args[4]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorMoveToValueTimes8):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorMoveToValue, line, args[0], args[1],
				args[2] * 8 * (args[3] ? -1 : 1),
				NO_CRUSH, P_ArgToChange(args[4]), false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorMoveToValueAndCrush):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorMoveToValue, line, args[0], args[1],
				args[2], P_ArgToCrush(args[3]),
				0, P_ArgToCrushType(args[4]), false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseToLowestCeiling):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseToLowestCeiling, line, args[0], args[1], 0,
				P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerToLowestCeiling):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerToLowestCeiling, line, args[0], args[1], args[4],
				NO_CRUSH, P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseByTexture):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseByTexture, line, args[0], args[1], 0,
				P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerByTexture):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerByTexture, line, args[0], args[1], 0,
				NO_CRUSH, P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseToCeiling):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseToCeiling, line, args[0], args[1], args[4],
				P_ArgToCrush(args[3]), P_ArgToChange(args[2]), true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorRaiseByValueTxTy):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorRaiseAndChange, line, args[0], args[1], args[2],
				NO_CRUSH, 0, false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorLowerToLowestTxTy):
			buttonSuccess = EV_DoZDoomFloor(FloorKind::FloorLowerAndChange, line, args[0], args[1], args[2],
				NO_CRUSH, 0, false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericFloor):
		{
			FloorKind type;
			dboolean raise_or_lower;
			byte index;

			static FloorKind floor_type[2][7] = {
				{
					FloorKind::FloorLowerByValue,
					FloorKind::FloorLowerToHighest,
					FloorKind::FloorLowerToLowest,
					FloorKind::FloorLowerToNearest,
					FloorKind::FloorLowerToLowestCeiling,
					FloorKind::FloorLowerToCeiling,
					FloorKind::FloorLowerByTexture,
				},
				{
					FloorKind::FloorRaiseByValue,
					FloorKind::FloorRaiseToHighest,
					FloorKind::FloorRaiseToLowest,
					FloorKind::FloorRaiseToNearest,
					FloorKind::FloorRaiseToLowestCeiling,
					FloorKind::FloorRaiseToCeiling,
					FloorKind::FloorRaiseByTexture,
				}
			};

			raise_or_lower = (args[4] & 8) >> 3;
			index = (args[3] < 7) ? args[3] : 0;
			type = floor_type[raise_or_lower][index];

			buttonSuccess = EV_DoZDoomFloor(type, line, args[0], args[1], args[2],
				(args[4] & 16) ? 20 : NO_CRUSH, args[4] & 7, false, false);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::FloorCrushStop):
			buttonSuccess = EV_ZDoomFloorCrushStop(args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorStop):
			buttonSuccess = EV_ZDoomFloorStop(args[0], line);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorDonut):
			buttonSuccess = EV_DoZDoomDonut(args[0], line, P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerByValue):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerByValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2], P_ArgToCrush(args[4]), 0,
				P_ArgToChange(args[3]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseByValue):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseByValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2], P_ArgToCrush(args[4]), 0,
				P_ArgToChange(args[3]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerByValueTimes8):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerByValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2] * 8, NO_CRUSH, 0,
				P_ArgToChange(args[3]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseByValueTimes8):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseByValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2] * 8, NO_CRUSH, 0,
				P_ArgToChange(args[3]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushAndRaise):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]) / 2,
				8, args[2], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[3], false)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerAndCrush):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerAndCrush, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]),
				8, args[2], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[3], args[1] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerAndCrushDist):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerAndCrush, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]),
				args[3], args[2], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], args[1] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushRaiseAndStay):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushRaiseAndStay, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]) / 2,
				8, args[2], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[3], false)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingMoveToValueTimes8):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilMoveToValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2] * 8 * (args[3] ? -1 : 1), NO_CRUSH, 0,
				P_ArgToChange(args[4]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingMoveToValue):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilMoveToValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2] * (args[3] ? -1 : 1), NO_CRUSH, 0,
				P_ArgToChange(args[4]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingMoveToValueAndCrush):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilMoveToValue, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[2], P_ArgToCrush(args[3]), 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], args[1] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerToHighestFloor):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerToHighestFloor, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[4], P_ArgToCrush(args[3]), 0,
				P_ArgToChange(args[2]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerInstant):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerInstant, line, args[0],
				0, 0,
				args[2] * 8, P_ArgToCrush(args[4]), 0,
				P_ArgToChange(args[3]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseInstant):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseInstant, line, args[0],
				0, 0,
				args[2] * 8, NO_CRUSH, 0,
				P_ArgToChange(args[3]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushRaiseAndStayA):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushRaiseAndStay, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
				0, args[3], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], false)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushRaiseAndStaySilA):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushRaiseAndStay, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
				0, args[3], 1,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], false)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushAndRaiseA):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
				0, args[3], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], args[1] == 8 && args[2] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushAndRaiseDist):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[2]), P_ArgToSpeed(args[2]),
				args[1], args[3], 0,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], args[2] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushAndRaiseSilentA):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
				0, args[3], 1,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], args[1] == 8 && args[2] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushAndRaiseSilentDist):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[2]), P_ArgToSpeed(args[2]),
				args[1], args[3], 1,
				0, static_cast<CrushMode>(P_ArgToCrushMode(args[4], args[2] == 8)));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseToNearest):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseToNearest, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, NO_CRUSH, P_ArgToChange(args[2]),
				0, static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseToHighest):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseToHighest, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, NO_CRUSH, P_ArgToChange(args[2]),
				0, static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseToLowest):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseToLowest, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, NO_CRUSH, P_ArgToChange(args[2]),
				0, static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseToHighestFloor):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseToHighestFloor, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, NO_CRUSH, P_ArgToChange(args[2]),
				0, static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingRaiseByTexture):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseByTexture, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, NO_CRUSH, P_ArgToChange(args[2]),
				0, static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerToLowest):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerToLowest, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, P_ArgToCrush(args[3]), 0,
				P_ArgToChange(args[2]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerToNearest):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerToNearest, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, P_ArgToCrush(args[3]), 0,
				P_ArgToChange(args[2]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingToHighestInstant):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerToHighest, line, args[0],
				2 * FRACUNIT, 0,
				0, P_ArgToCrush(args[2]), 0,
				P_ArgToChange(args[1]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingToFloorInstant):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseToFloor, line, args[0],
				2 * FRACUNIT, 0,
				args[3], P_ArgToCrush(args[2]), 0,
				P_ArgToChange(args[1]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerToFloor):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerToFloor, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				args[4], P_ArgToCrush(args[3]), 0,
				P_ArgToChange(args[4]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingLowerByTexture):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilLowerByTexture, line, args[0],
				P_ArgToSpeed(args[1]), 0,
				0, P_ArgToCrush(args[3]), 0,
				P_ArgToChange(args[4]), static_cast<CrushMode>(false));
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericCeiling):
		{
			CeilingKind type;
			dboolean raise_or_lower;
			byte index;

			static CeilingKind ceiling_type[2][7] = {
				{
					CeilingKind::CeilLowerByValue,
					CeilingKind::CeilLowerToHighest,
					CeilingKind::CeilLowerToLowest,
					CeilingKind::CeilLowerToNearest,
					CeilingKind::CeilLowerToHighestFloor,
					CeilingKind::CeilLowerToFloor,
					CeilingKind::CeilLowerByTexture,
				},
				{
					CeilingKind::CeilRaiseByValue,
					CeilingKind::CeilRaiseToHighest,
					CeilingKind::CeilRaiseToLowest,
					CeilingKind::CeilRaiseToNearest,
					CeilingKind::CeilRaiseToHighestFloor,
					CeilingKind::CeilRaiseToFloor,
					CeilingKind::CeilRaiseByTexture,
				}
			};

			raise_or_lower = (args[4] & 8) >> 3;
			index = (args[3] < 7) ? args[3] : 0;
			type = ceiling_type[raise_or_lower][index];

			buttonSuccess = EV_DoZDoomCeiling(type, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[1]),
				args[2], (args[4] & 16) ? 20 : NO_CRUSH, 0,
				args[4] & 7, static_cast<CrushMode>(false));
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::CeilingCrushStop):
		{
			dboolean remove;

			switch(args[3])
			{
				case 1:
					remove = false;
					break;
				case 2:
					remove = true;
					break;
				default:
					remove = hexen;
					break;
			}

			buttonSuccess = EV_ZDoomCeilingCrushStop(args[0], remove);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::CeilingStop):
			buttonSuccess = EV_ZDoomCeilingStop(args[0], line);
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericCrusher):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
				0, args[4], args[3] ? 2 : 0,
				0, static_cast<CrushMode>((args[1] <= 24 && args[2] <= 24) ? CrushMode::Slowdown : CrushMode::Doom));
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericCrusher2):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilCrushAndRaise, line, args[0],
				P_ArgToSpeed(args[1]), P_ArgToSpeed(args[2]),
				0, args[4], args[3] ? 2 : 0,
				0, static_cast<CrushMode>(CrushMode::Hexen));
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorWaggle):
			buttonSuccess = EV_StartPlaneWaggle(args[0], line, args[1], args[2], args[3], args[4], false);
			break;
		case std::to_underlying(ZDoomLineSpecial::CeilingWaggle):
			buttonSuccess = EV_StartPlaneWaggle(args[0], line, args[1], args[2], args[3], args[4], true);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorAndCeilingLowerRaise):
			buttonSuccess = EV_DoZDoomCeiling(CeilingKind::CeilRaiseToHighest, line, args[0],
				P_ArgToSpeed(args[2]), 0, 0, 0, 0, 0, static_cast<CrushMode>(false));
			buttonSuccess |= EV_DoZDoomFloor(FloorKind::FloorLowerToLowest, line, args[0],
				args[1], 0, NO_CRUSH, 0, false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildDown):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildDown, line,
				args[2], P_ArgToSpeed(args[1]), args[3],
				args[4], 0, StairFlag::UseSpecials);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildUp):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildUp, line,
				args[2], P_ArgToSpeed(args[1]), args[3],
				args[4], 0, StairFlag::UseSpecials);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildDownSync):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildDown, line,
				args[2], P_ArgToSpeed(args[1]), 0,
				args[3], 0, StairFlag::UseSpecials | StairFlag::Sync);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildUpSync):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildUp, line,
				args[2], P_ArgToSpeed(args[1]), 0,
				args[3], 0, StairFlag::UseSpecials | StairFlag::Sync);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildDownDoom):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildDown, line,
				args[2], P_ArgToSpeed(args[1]), args[3],
				args[4], 0, StairFlag{});
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildUpDoom):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildUp, line,
				args[2], P_ArgToSpeed(args[1]), args[3],
				args[4], 0, StairFlag{});
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildDownDoomSync):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildDown, line,
				args[2], P_ArgToSpeed(args[1]), 0,
				args[3], 0, StairFlag::Sync);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildUpDoomSync):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildUp, line,
				args[2], P_ArgToSpeed(args[1]), 0,
				args[3], 0, StairFlag::Sync);
			break;
		case std::to_underlying(ZDoomLineSpecial::StairsBuildUpDoomCrush):
			buttonSuccess = EV_BuildZDoomStairs(args[0], StairType::BuildUp, line,
				args[2], P_ArgToSpeed(args[1]), args[3],
				args[4], 0, StairFlag::Crush);
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericStairs):
		{
			StairType type;

			type = (args[3] & 1) ? StairType::BuildUp : StairType::BuildDown;
			buttonSuccess = EV_BuildZDoomStairs(args[0], type, line,
				args[2], P_ArgToSpeed(args[1]), 0,
				args[4], args[3] & 2, StairFlag{});

			// Toggle direction of next activation of repeatable stairs
			if(buttonSuccess && line &&
				(line->flags & LineFlag::RepeatSpecial) != LineFlag{} &&
				line->special == std::to_underlying(ZDoomLineSpecial::GenericStairs))
			{
				line->special_args[3] ^= 1;
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::PlatStop):
		{
			dboolean remove;

			switch(args[3])
			{
				case 1:
					remove = false;
					break;
				case 2:
					remove = true;
					break;
				default:
					remove = hexen;
					break;
			}

			EV_StopZDoomPlat(args[0], remove);
			buttonSuccess = 1;
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::PlatPerpetualRaise):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatPerpetualRaise, 0,
				P_ArgToSpeed(args[1]), args[2], 8, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatPerpetualRaiseLip):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatPerpetualRaise, 0,
				P_ArgToSpeed(args[1]), args[2], args[3], 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatDownWaitUpStay):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatDownWaitUpStay, 0,
				P_ArgToSpeed(args[1]), args[2], 8, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatDownWaitUpStayLip):
			buttonSuccess = EV_DoZDoomPlat(args[0], line,
				args[4] ? PlatType::PlatDownWaitUpStayStone : PlatType::PlatDownWaitUpStay, 0,
				P_ArgToSpeed(args[1]), args[2], args[3], 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatDownByValue):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatDownByValue, args[3] * 8,
				P_ArgToSpeed(args[1]), args[2], 0, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatUpByValue):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatUpByValue, args[3] * 8,
				P_ArgToSpeed(args[1]), args[2], 0, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatUpWaitDownStay):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatUpWaitDownStay, 0,
				P_ArgToSpeed(args[1]), args[2], 0, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatUpNearestWaitDownStay):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatUpNearestWaitDownStay, 0,
				P_ArgToSpeed(args[1]), args[2], 0, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatRaiseAndStayTx0):
		{
			PlatType type;

			switch(args[3])
			{
				case 1:
					type = PlatType::PlatRaiseAndStay;
					break;
				case 2:
					type = PlatType::PlatRaiseAndStayLockout;
					break;
				default:
					type = (heretic ? PlatType::PlatRaiseAndStayLockout : PlatType::PlatRaiseAndStay);
					break;
			}

			buttonSuccess = EV_DoZDoomPlat(args[0], line, type, 0, P_ArgToSpeed(args[1]), 0, 0, 1);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::PlatUpByValueStayTx):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatUpByValueStay, args[2] * 8,
				P_ArgToSpeed(args[1]), 0, 0, 2);
			break;
		case std::to_underlying(ZDoomLineSpecial::PlatToggleCeiling):
			buttonSuccess = EV_DoZDoomPlat(args[0], line, PlatType::PlatToggle, 0, 0, 0, 0, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::GenericLift):
		{
			PlatType type;

			switch(args[3])
			{
				case 1:
					type = PlatType::PlatDownWaitUpStay;
					break;
				case 2:
					type = PlatType::PlatDownToNearestFloor;
					break;
				case 3:
					type = PlatType::PlatDownToLowestCeiling;
					break;
				case 4:
					type = PlatType::PlatPerpetualRaise;
					break;
				default:
					type = PlatType::PlatUpByValue;
					break;
			}

			buttonSuccess = EV_DoZDoomPlat(args[0], line, type, args[4] * 8,
				P_ArgToSpeed(args[1]), args[2] * TICRATE / 8, 0, 0);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::LineSetBlocking):
			if(args[0])
			{
				const int* id_p;
				static constexpr std::array flags =
				{
					LineFlag::Blocking,
					LineFlag::BlockMonsters,
					LineFlag::BlockPlayers,
					LineFlag::BlockFloaters,
					LineFlag::BlockProjectiles,
					LineFlag::BlockEverything,
					LineFlag::JumpOver,
					LineFlag::BlockUse,
					LineFlag::BlockSight,
					LineFlag::BlockHitscan,
					LineFlag::SoundBlock,
					LineFlag::BlockLandMonsters,
				};

				LineFlag setflags{};
				LineFlag clearflags{};

				for(const LineFlag flag : flags)
				{
					if(args[1] & 1) setflags |= flag;
					if(args[2] & 1) clearflags |= flag;
					args[1] >>= 1;
					args[2] >>= 1;
				}

				FIND_LINES(id_p, args[0])
				{
					lines[*id_p].flags = (lines[*id_p].flags - clearflags) | setflags;
				}

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorChangeFlags):
			if(args[0])
			{
				int i;
				const int* id_p;
				// The flag of each bit in args[1] (set) and args[2] (clear).
				static constexpr std::array<SectorFlag, 12> flags =
				{
					SectorFlag::Silent,
					SectorFlag{},
					SectorFlag{},
					SectorFlag{},
					SectorFlag::Friction,
					SectorFlag::Push,
					SectorFlag{},
					SectorFlag{},
					SectorFlag::EndGodMode,
					SectorFlag::EndLevel,
					SectorFlag::Hazard,
					SectorFlag::NoAttack,
				};

				SectorFlag setflags = {};
				SectorFlag clearflags = {};

				for(i = 0; i < std::ssize(flags); i++, args[1] >>= 1, args[2] >>= 1)
				{
					if(args[1] & 1) setflags |= flags[i];
					if(args[2] & 1) clearflags |= flags[i];
				}

				FIND_SECTORS(id_p, args[0])
					sectors[*id_p].flags = (sectors[*id_p].flags - clearflags) | setflags;

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::LineSetAutomapFlags):
			if(args[0])
			{
				const int* id_p;
				static constexpr std::array flags =
				{
					LineFlag::Secret,
					LineFlag::DontDraw,
					LineFlag::Mapped,
					LineFlag::Revealed,
				};

				LineFlag setflags{};
				LineFlag clearflags{};

				for(const LineFlag flag : flags)
				{
					if(args[1] & 1) setflags |= flag;
					if(args[2] & 1) clearflags |= flag;
					args[1] >>= 1;
					args[2] >>= 1;
				}

				FIND_LINES(id_p, args[0])
				{
					lines[*id_p].flags = (lines[*id_p].flags - clearflags) | setflags;
				}

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::LineSetAutomapStyle):
			if(args[1] >= 0 && args[1] <= std::to_underlying(AutomapStyle::Portal))
			{
				const int* id_p;

				FIND_LINES(id_p, args[0])
				{
					lines[*id_p].automap_style = static_cast<AutomapStyle>(args[1]);
				}

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::ScrollWall):
			if(args[0])
			{
				const int* id_p;
				int side = !!args[3];

				FIND_LINES(id_p, args[0])
				{
					dsda_AddSideScroller(args[1], args[2], lines[*id_p].sidenum[side], args[4]);
				}

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::LineSetTextureOffset):
			if(args[0])
			{
				const int* id_p;
				const int NO_CHANGE = 32767 << FRACBITS;
				int sidenum = !!args[3];

				FIND_LINES(id_p, args[0])
				{
					side_t* side = &sides[lines[*id_p].sidenum[sidenum]];

					if(args[4] & 8)
					{
						if(args[1] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->textureoffset_top += args[1];

							if(args[4] & 2)
								side->textureoffset_mid += args[1];

							if(args[4] & 4)
								side->textureoffset_bottom += args[1];
						}

						if(args[2] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->rowoffset_top += args[2];

							if(args[4] & 2)
								side->rowoffset_mid += args[2];

							if(args[4] & 4)
								side->rowoffset_bottom += args[2];
						}
					}
					else
					{
						if(args[1] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->textureoffset_top = args[1];

							if(args[4] & 2)
								side->textureoffset_mid = args[1];

							if(args[4] & 4)
								side->textureoffset_bottom = args[1];
						}

						if(args[2] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->rowoffset_top = args[2];

							if(args[4] & 2)
								side->rowoffset_mid = args[2];

							if(args[4] & 4)
								side->rowoffset_bottom = args[2];
						}
					}
				}

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::LineSetTexturescale):
			if(args[0])
			{
				const int* id_p;
				const int NO_CHANGE = 32767 << FRACBITS;
				int sidenum = !!args[3];

				if(!args[1])
					args[1] = FRACUNIT;

				if(!args[2])
					args[2] = FRACUNIT;

				FIND_LINES(id_p, args[0])
				{
					side_t* side = &sides[lines[*id_p].sidenum[sidenum]];

					if(args[4] & 8)
					{
						if(args[1] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->scalex_top = FixedMul(side->scalex_top, args[1]);

							if(args[4] & 2)
								side->scalex_mid = FixedMul(side->scalex_mid, args[1]);

							if(args[4] & 4)
								side->scalex_bottom = FixedMul(side->scalex_bottom, args[1]);
						}

						if(args[2] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->scaley_top = FixedMul(side->scaley_top, args[2]);

							if(args[4] & 2)
								side->scaley_mid = FixedMul(side->scaley_mid, args[2]);

							if(args[4] & 4)
								side->scaley_bottom = FixedMul(side->scaley_bottom, args[2]);
						}
					}
					else
					{
						if(args[1] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->scalex_top = args[1];

							if(args[4] & 2)
								side->scalex_mid = args[1];

							if(args[4] & 4)
								side->scalex_bottom = args[1];
						}

						if(args[2] != NO_CHANGE)
						{
							if(args[4] & 1)
								side->scaley_top = args[2];

							if(args[4] & 2)
								side->scaley_mid = args[2];

							if(args[4] & 4)
								side->scaley_bottom = args[2];
						}
					}
				}

				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::NoiseAlert):
		{

			mobj_t *target, *emitter;

			if(!args[0])
			{
				target = mo;
			}
			else
			{
				// not supported yet
				target = nullptr;
			}

			if(!args[1])
			{
				emitter = mo;
			}
			else
			{
				// not supported yet
				emitter = nullptr;
			}

			if(emitter)
			{
				P_NoiseAlert(target, emitter);
			}

			buttonSuccess = 1;
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetGravity):
		{
			fixed_t gravity;
			const int* id_p;

			if(args[2] > 99)
				args[2] = 99;

			gravity = P_ArgsToFixed(args[1], args[2]);

			FIND_SECTORS(id_p, args[0])
				sectors[*id_p].gravity = gravity;
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetDamage):
		{
			const int* id_p;
			dboolean unblockable = false;

			if(args[3] == 0)
			{
				if(args[1] < 20)
				{
					args[4] = 0;
					args[3] = 32;
				}
				else if(args[1] < 50)
				{
					args[4] = 5;
					args[3] = 32;
				}
				else
				{
					unblockable = true;
					args[4] = 0;
					args[3] = 1;
				}
			}

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].damage.amount = args[1];
				sectors[*id_p].damage.interval = args[3];
				sectors[*id_p].damage.leakrate = args[4];
				if(unblockable)
					sectors[*id_p].flags |= SectorFlag::DamageUnblockable;
				else
					sectors[*id_p].flags -= SectorFlag::DamageUnblockable;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorTransferNumeric):
			buttonSuccess = EV_DoChange(line, ChangeKind::NumericOnly, args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::FloorTransferTrigger):
			buttonSuccess = EV_DoChange(line, ChangeKind::TriggerOnly, args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetFloorPanning):
		{
			const int* id_p;
			fixed_t xoffs, yoffs;

			xoffs = P_ArgsToFixed(args[1], args[2]);
			yoffs = P_ArgsToFixed(args[3], args[4]);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].floor_xoffs = xoffs;
				sectors[*id_p].floor_yoffs = yoffs;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetCeilingPanning):
		{
			const int* id_p;
			fixed_t xoffs, yoffs;

			xoffs = P_ArgsToFixed(args[1], args[2]);
			yoffs = P_ArgsToFixed(args[3], args[4]);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].ceiling_xoffs = xoffs;
				sectors[*id_p].ceiling_yoffs = yoffs;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetRotation):
		{
			const int* id_p;
			angle_t floor, ceiling;

			floor = dsda_DegreesToAngle(args[1]);
			ceiling = dsda_DegreesToAngle(args[2]);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].floor_rotation = floor;
				sectors[*id_p].ceiling_rotation = ceiling;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetFloorScale):
		{
			const int* id_p;
			fixed_t xscale, yscale;

			xscale = (args[1] << FRACBITS) + (args[2] << FRACBITS) / 100;
			yscale = (args[3] << FRACBITS) + (args[4] << FRACBITS) / 100;

			if(xscale)
				xscale = FixedDiv(FRACUNIT, xscale);

			if(yscale)
				yscale = FixedDiv(FRACUNIT, yscale);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].floor_xscale = xscale;
				sectors[*id_p].floor_yscale = yscale;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetFloorScale2):
		{
			const int* id_p;
			fixed_t xscale, yscale;

			xscale = args[1];
			yscale = args[2];

			if(xscale)
				xscale = FixedDiv(FRACUNIT, xscale);

			if(yscale)
				yscale = FixedDiv(FRACUNIT, yscale);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].floor_xscale = xscale;
				sectors[*id_p].floor_yscale = yscale;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetCeilingScale):
		{
			const int* id_p;
			fixed_t xscale, yscale;

			xscale = (args[1] << FRACBITS) + (args[2] << FRACBITS) / 100;
			yscale = (args[3] << FRACBITS) + (args[4] << FRACBITS) / 100;

			if(xscale)
				xscale = FixedDiv(FRACUNIT, xscale);

			if(yscale)
				yscale = FixedDiv(FRACUNIT, yscale);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].ceiling_xscale = xscale;
				sectors[*id_p].ceiling_yscale = yscale;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetCeilingScale2):
		{
			const int* id_p;
			fixed_t xscale, yscale;

			xscale = args[1];
			yscale = args[2];

			if(xscale)
				xscale = FixedDiv(FRACUNIT, xscale);

			if(yscale)
				yscale = FixedDiv(FRACUNIT, yscale);

			FIND_SECTORS(id_p, args[0])
			{
				sectors[*id_p].ceiling_xscale = xscale;
				sectors[*id_p].ceiling_yscale = yscale;
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::HealThing):
			if(mo)
			{
				int max = args[1];

				buttonSuccess = 1;

				if(!max || !mo->player)
				{
					P_HealMobj(mo, args[0]);
					break;
				}
				else if(max == 1)
				{
					max = max_soul;
				}

				if(mo->health < max)
				{
					mo->health += P_PlayerHealthIncrease(args[0]);
					if(mo->health > max && max > 0)
					{
						mo->health = max;
					}
					mo->player->health = mo->health;
				}
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::ForceField):
			if(mo)
			{
				P_DamageMobj(mo, nullptr, nullptr, 16);
				P_ThrustMobj(mo, ANG180 + mo->angle, 2048 * 250);
			}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ClearForceField):
		{
			const int* id_p;

			FIND_SECTORS(id_p, args[0])
			{
				int i;

				buttonSuccess = 1;

				for(i = 0; i < sectors[*id_p].linecount; i++)
				{
					line_t* line = sectors[*id_p].lines[i];

					if(line->backsector && line->special == std::to_underlying(ZDoomLineSpecial::ForceField))
					{
						line->flags -= (LineFlag::Blocking | LineFlag::BlockEverything);
						line->special = 0;
						sides[line->sidenum[0]].midtexture = NO_TEXTURE;
						sides[line->sidenum[1]].midtexture = NO_TEXTURE;
					}
				}
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ExitNormal):
			G_ExitLevel(args[0]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ExitSecret):
			G_SecretExitLevel(args[0]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::TeleportNewMap):
			if(!side)
			{
				int flags;

				flags = args[2] ? LF_SET_ANGLE : 0;

				// TODO: this crashes if the map doesn't exist (gzdoom does a no-op)
				G_Completed(args[0], args[1], flags, mo->angle);
				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::TeleportEndGame):
			if(!side)
			{
				G_Completed(LEAVE_VICTORY, LEAVE_VICTORY, 0, 0);
				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjRotateLeft):
			buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], 1, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjOrRotateLeft):
			buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], 1, true);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjRotateRight):
			buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], -1, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjOrRotateRight):
			buttonSuccess = EV_RotateZDoomPoly(line, args[0], args[1], args[2], -1, true);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjMove):
			buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
				args[2], args[3], false, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjOrMove):
			buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
				args[2], args[3], false, true);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjMoveTimes8):
			buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
				args[2], args[3], true, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjOrMoveTimes8):
			buttonSuccess = EV_MoveZDoomPoly(line, args[0], args[1],
				args[2], args[3], true, true);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjDoorSwing):
			buttonSuccess = EV_OpenZDoomPolyDoor(line, args[0], args[1],
				args[2], args[3], args[4], PolyDoorType::Swing);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjDoorSlide):
			buttonSuccess = EV_OpenZDoomPolyDoor(line, args[0], args[1],
				args[2], args[3], args[4], PolyDoorType::Slide);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjMoveTo):
			buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
				args[2] << FRACBITS, args[3] << FRACBITS, false);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjOrMoveTo):
			buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
				args[2] << FRACBITS, args[3] << FRACBITS, true);
			break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjMoveToSpot):
		{
			mobj_t* dest;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			dest = dsda_FindMobjFromThingID(args[2], &search);

			if(!dest)
			{
				break;
			}

			buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
				dest->x, dest->y, false);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjOrMoveToSpot):
		{
			mobj_t* dest;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			dest = dsda_FindMobjFromThingID(args[2], &search);

			if(!dest)
			{
				break;
			}

			buttonSuccess = EV_MovePolyTo(line, args[0], P_ArgToSpeed(args[1]),
				dest->x, dest->y, true);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::PolyobjStop):
			buttonSuccess = EV_StopPoly(args[0]);
			break;
		case std::to_underlying(ZDoomLineSpecial::RadiusQuake):
		{
			mobj_t* spawn_location;
			thing_id_search_t search;

			args[0] = BETWEEN(1, 9, args[0]);

			dsda_ResetThingIDSearch(&search);
			while((spawn_location = dsda_FindMobjFromThingIDOrMobj(args[4], mo, &search)))
			{
				dsda_SpawnQuake(spawn_location, args[0], args[1], args[2], args[3]);
				buttonSuccess = 1;
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThingMove):
		{
			mobj_t* target;
			mobj_t* dest;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search);
			dsda_ResetThingIDSearch(&search);
			dest = dsda_FindMobjFromThingID(args[1], &search);

			if(target && dest)
			{
				buttonSuccess = P_MoveThing(target, dest->x, dest->y, dest->z, args[2] ? false : true);
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::TeleportOther):
			if(args[0] && args[1])
			{
				mobj_t* target;
				thing_id_search_t search;

				dsda_ResetThingIDSearch(&search);
				while((target = dsda_FindMobjFromThingID(args[0], &search)))
				{
					buttonSuccess |= map_format.ev_teleport(args[1], 0, nullptr, 0, target,
						args[2] ? (TeleportFlag::DestFog | TeleportFlag::SourceFog) : TeleportFlag::KeepOrientation);
				}
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::TeleportGroup):
			buttonSuccess = EV_TeleportGroup(args[0], mo, args[1], args[2], args[3], args[4]);
			break;
		case std::to_underlying(ZDoomLineSpecial::TeleportInSector):
			buttonSuccess = EV_TeleportInSector(args[0], args[1], args[2], args[3], args[4]);
			break;
		case std::to_underlying(ZDoomLineSpecial::Teleport):
		{
			TeleportFlag flags = TeleportFlag::DestFog;

			if(!args[2])
				flags |= TeleportFlag::SourceFog;

			buttonSuccess = map_format.ev_teleport(args[0], args[1], line, side, mo, flags);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::TeleportNoFog):
		{
			TeleportFlag flags = {};

			switch(args[1])
			{
				case 0:
					flags |= TeleportFlag::KeepOrientation;
					break;

				case 2:
					if(line)
						flags |= TeleportFlag::KeepOrientation | TeleportFlag::RotateBoom;
					break;

				case 3:
					if(line)
						flags |= TeleportFlag::KeepOrientation | TeleportFlag::RotateBoomInverse;
					break;

				default:
					break;
			}

			if(args[3])
				flags |= TeleportFlag::KeepHeight;

			buttonSuccess = map_format.ev_teleport(args[0], args[2], line, side, mo, flags);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::TeleportNoStop):
		{
			TeleportFlag flags = TeleportFlag::DestFog | TeleportFlag::KeepVelocity;

			if(!args[2])
				flags |= TeleportFlag::SourceFog;

			buttonSuccess = map_format.ev_teleport(args[0], args[1], line, side, mo, flags);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::TeleportZombieChanger):
			if(mo)
			{
				map_format.ev_teleport(args[0], args[1], line, side, mo, TeleportFlag{});
				if(mo->health >= 0 && mo->info->painstate != StateId::Null)
				{
					P_SetMobjState(mo, mo->info->painstate);
				}
				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::TeleportLine):
			buttonSuccess = EV_SilentLineTeleport(line, side, mo, args[1], args[2]);
			break;
		case std::to_underlying(ZDoomLineSpecial::LightRaiseByValue):
			EV_LightChange(args[0], args[1]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightLowerByValue):
			EV_LightChange(args[0], -(short)args[1]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightChangeToValue):
			EV_LightSet(args[0], args[1]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightMinNeighbor):
			EV_LightSetMinNeighbor(args[0]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightMaxNeighbor):
			EV_LightSetMaxNeighbor(args[0]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightFade):
			EV_StartLightFading(args[0], args[1], args[2]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightGlow):
			EV_StartLightGlowing(args[0], args[1], args[2], args[3]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightFlicker):
			EV_StartLightFlickering(args[0], args[1], args[2]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightStrobe):
			EV_StartZDoomLightStrobing(args[0], args[1], args[2], args[3], args[4]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightStrobeDoom):
			EV_StartZDoomLightStrobingDoom(args[0], args[1], args[2]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::LightStop):
			EV_StopLightEffect(args[0]);
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingSetSpecial):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				target->special = args[1];
				target->special_args[0] = args[2];
				target->special_args[1] = args[3];
				target->special_args[2] = args[4];
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingSpawn):
			buttonSuccess =
				P_SpawnThing(args[0], mo, args[1], P_ArgToAngle(args[2]), true, args[3]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingSpawnNoFog):
			buttonSuccess =
				P_SpawnThing(args[0], mo, args[1], P_ArgToAngle(args[2]), false, args[3]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingSpawnFacing):
			buttonSuccess =
				P_SpawnThing(args[0], mo, args[1], ANGLE_MAX, args[2] ? false : true, args[3]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingProjectile):
			buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], P_ArgToAngle(args[2]),
				P_ArgToSpeed(args[3]), P_ArgToSpeed(args[4]),
				0, nullptr, 0, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingProjectileGravity):
			buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], P_ArgToAngle(args[2]),
				P_ArgToSpeed(args[3]), P_ArgToSpeed(args[4]),
				0, nullptr, 1, 0);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingProjectileAimed):
			buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], 0,
				P_ArgToSpeed(args[2]), 0,
				args[3], mo, 0, args[4]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingProjectileIntercept):
			// ZDoom's implementation relies on a bunch of trigonometry
			// I tried converting this to fixed points,
			//   but the calculations easily go out of bounds (dot products).
			// Needs a different implementation, or 64 bit fixed point conversions
			// Falling back on the default aimed behaviour for now
			buttonSuccess = P_SpawnProjectile(args[0], mo, args[1], 0,
				P_ArgToSpeed(args[2]), 0,
				args[3], mo, 0, args[4]);
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingStop):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				buttonSuccess = 1;

				target->momx = 0;
				target->momy = 0;
				target->momz = 0;

				if(target->player)
				{
					target->player->momx = 0;
					target->player->momy = 0;
				}
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThingChangeTid):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				dsda_RemoveMobjThingID(target);
				target->tid = args[1];
				if(target->tid)
					dsda_AddMobjThingID(target, args[1]);
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingHate):
		{
			mobj_t* hater;
			mobj_t* target;
			thing_id_search_t search;
			thing_id_search_t target_search;

			// Currently no support for this arg
			if(args[2])
			{
				break;
			}

			if(!args[0] && mo && mo->player)
			{
				break;
			}

			buttonSuccess = 1;

			dsda_ResetThingIDSearch(&target_search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[1], mo, &target_search)))
			{
				if(
					(target->flags & MobjFlag::Shootable) != MobjFlag{} &&
					target->health > 0 &&
					(target->flags2 & MobjFlag2::Dormant) == MobjFlag2{}
				)
				{
					break;
				}
			}

			if(!target)
			{
				break;
			}

			dsda_ResetThingIDSearch(&search);
			while((hater = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				if(
					hater->health > 0 &&
					(hater->flags & MobjFlag::Shootable) != MobjFlag{} &&
					hater->info->seestate != StateId::Null
				)
				{
					while((target = dsda_FindMobjFromThingIDOrMobj(args[1], mo, &target_search)))
					{
						if(
							(target->flags & MobjFlag::Shootable) != MobjFlag{} &&
							target->health > 0 &&
							(target->flags2 & MobjFlag2::Dormant) == MobjFlag2{} &&
							target != hater
						)
						{
							break;
						}
					}

					// Restart from beginning of list
					if(!target)
					{
						dsda_ResetThingIDSearch(&target_search);
						while((target = dsda_FindMobjFromThingIDOrMobj(args[1], mo, &target_search)))
						{
							if(
								(target->flags & MobjFlag::Shootable) != MobjFlag{} &&
								target->health > 0 &&
								(target->flags2 & MobjFlag2::Dormant) == MobjFlag2{} &&
								target != hater
							)
							{
								break;
							}
						}
					}

					// We might have no target if the hater is the only possible target
					if(target)
					{
						P_SetTarget(&hater->lastenemy, hater->target);
						P_SetTarget(&hater->target, target);

						if((hater->flags2 & MobjFlag2::Dormant) == MobjFlag2{})
						{
							P_SetMobjState(hater, hater->info->seestate);
						}
					}
				}
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThingRemove):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				if(!target->player)
				{
					if((target->flags & MobjFlag::CountKill) != MobjFlag{})
						dsda_WatchKill(&players[consoleplayer], target);

					P_RemoveMobj(target);
				}
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingActivate):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				if((target->flags2 & MobjFlag2::Dormant) != MobjFlag2{})
				{
					target->flags2 -= MobjFlag2::Dormant;
					target->tics = 1;
				}

				buttonSuccess = 1;
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThingDeactivate):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				if((target->flags2 & MobjFlag2::Dormant) == MobjFlag2{})
				{
					target->flags2 |= MobjFlag2::Dormant;
					target->tics = -1;
				}

				buttonSuccess = 1;
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThrustThing):
		{
			fixed_t thrust;
			mobj_t* target;
			thing_id_search_t search;

			thrust = args[1] * FRACUNIT;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[3], mo, &search)))
			{
				P_ThrustMobj(target, P_ArgToAngle(args[0]), thrust);
			}

			buttonSuccess = (args[3] != 0 || mo);
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThrustThingZ):
		{
			fixed_t thrust;
			mobj_t* target;
			thing_id_search_t search;

			thrust = args[1] * FRACUNIT / 4;

			if(args[2])
				thrust = -thrust;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				if(!args[3])
					target->momz = thrust;
				else
					target->momz += thrust;

				buttonSuccess = 1;
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::ThingRaise):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				buttonSuccess |= P_RaiseThing(target, nullptr);
			}
		}
		break;
		case std::to_underlying(ZDoomLineSpecial::DamageThing):
			if(mo)
			{
				if(args[0] < 0)
				{
					P_HealMobj(mo, -args[0]);
				}
				else
				{
					P_DamageMobj(mo, nullptr, nullptr, args[0] ? args[0] : 10000);
				}
				buttonSuccess = 1;
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingDamage):
		{
			mobj_t* target;
			thing_id_search_t search;

			dsda_ResetThingIDSearch(&search);
			while((target = dsda_FindMobjFromThingIDOrMobj(args[0], mo, &search)))
			{
				if((target->flags & MobjFlag::Shootable) != MobjFlag{})
				{
					if(args[1] > 0)
					{
						P_DamageMobj(target, nullptr, mo, args[1]);
					}
					else
					{
						P_HealMobj(mo, -args[1]);
					}
				}
			}
		}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::ThingDestroy):
			if(!args[0] && !args[2])
			{
				P_Massacre();
			}
			else if(!args[0])
			{
				const int* id_p;

				FIND_SECTORS(id_p, args[2])
				{
					msecnode_t* n;
					sector_t* sec;

					sec = &sectors[*id_p];
					for(n = sec->touching_thinglist; n;)
					{
						mobj_t* target = n->m_thing;

						// Not sure if n might be freed when an enemy dies,
						//   so let's get the next node before applying the damage
						n = n->m_snext;

						if((target->flags & MobjFlag::Shootable) != MobjFlag{})
							P_DamageMobj(target, nullptr, mo, args[1] ? 10000 : target->health);
					}
				}
			}
			else
			{
				mobj_t* target;
				thing_id_search_t search;

				dsda_ResetThingIDSearch(&search);
				while((target = dsda_FindMobjFromThingID(args[0], &search)))
				{
					if(
						(target->flags & MobjFlag::Shootable) != MobjFlag{} &&
						(!args[2] || target->subsector->sector->tag == args[2])
					)
						P_DamageMobj(target, nullptr, mo, args[1] ? 10000 : target->health);
				}
			}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::MapSetColormap):
			if(args[0] >= 0)
			{
				map_colormap = args[0];
			}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::SectorSetColormap):
			if(args[0] >= 0 && args[1])
			{
				const int* id_p;

				FIND_SECTORS(id_p, args[1])
					sectors[*id_p].colormap = args[0];
			}
			buttonSuccess = 1;
			break;
		case std::to_underlying(ZDoomLineSpecial::MusicChangeSong):
			if(args[0] != LUMP_NOT_FOUND)
			{
				if(!args[1] || (mo->player && mo->player->mo == mo))
				{
					S_ChangeMusInfoMusic(args[0], args[2]);
					buttonSuccess = 1;
				}
			}
			break;
		case std::to_underlying(ZDoomLineSpecial::MusicStop):
			if(!args[0] || (mo->player && mo->player->mo == mo))
			{
				S_StopMusic();
				buttonSuccess = 1;
			}
			break;
		default:
			break;
	}

	return buttonSuccess;
}

extern "C" dboolean P_ExecuteHexenLineSpecial(int special, int* special_args, line_t* line, int side, mobj_t* mo)
{
	byte args[5];
	dboolean buttonSuccess = false;

	COLLAPSE_SPECIAL_ARGS(args, special_args);

	switch(special)
	{
		case 1: // Poly Start Line
			break;
		case 2: // Poly Rotate Left
			buttonSuccess = EV_RotatePoly(line, args, 1, false);
			break;
		case 3: // Poly Rotate Right
			buttonSuccess = EV_RotatePoly(line, args, -1, false);
			break;
		case 4: // Poly Move
			buttonSuccess = EV_MovePoly(line, args, false, false);
			break;
		case 6: // Poly Move Times 8
			buttonSuccess = EV_MovePoly(line, args, true, false);
			break;
		case 7: // Poly Door Swing
			buttonSuccess = EV_OpenPolyDoor(line, args, PolyDoorType::Swing);
			break;
		case 8: // Poly Door Slide
			buttonSuccess = EV_OpenPolyDoor(line, args, PolyDoorType::Slide);
			break;
		case 10: // Door Close
			buttonSuccess = Hexen_EV_DoDoor(line, args, VerticalDoorType::DrevClose);
			break;
		case 11: // Door Open
			if(!args[0])
			{
				buttonSuccess = Hexen_EV_VerticalDoor(line, mo);
			}
			else
			{
				buttonSuccess = Hexen_EV_DoDoor(line, args, VerticalDoorType::DrevOpen);
			}
			break;
		case 12: // Door Raise
			if(!args[0])
			{
				buttonSuccess = Hexen_EV_VerticalDoor(line, mo);
			}
			else
			{
				buttonSuccess = Hexen_EV_DoDoor(line, args, VerticalDoorType::DrevNormal);
			}
			break;
		case 13: // Door Locked_Raise
			if(CheckedLockedDoor(mo, args[3]))
			{
				if(!args[0])
				{
					buttonSuccess = Hexen_EV_VerticalDoor(line, mo);
				}
				else
				{
					buttonSuccess = Hexen_EV_DoDoor(line, args, VerticalDoorType::DrevNormal);
				}
			}
			break;
		case 20: // Floor Lower by Value
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevLowerfloorbyvalue);
			break;
		case 21: // Floor Lower to Lowest
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevLowerfloortolowest);
			break;
		case 22: // Floor Lower to Nearest
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevLowerfloor);
			break;
		case 23: // Floor Raise by Value
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevRaisefloorbyvalue);
			break;
		case 24: // Floor Raise to Highest
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevRaisefloor);
			break;
		case 25: // Floor Raise to Nearest
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevRaisefloortonearest);
			break;
		case 26: // Stairs Build Down Normal
			buttonSuccess = Hexen_EV_BuildStairs(line, args, -1, StairsMode::Normal);
			break;
		case 27: // Build Stairs Up Normal
			buttonSuccess = Hexen_EV_BuildStairs(line, args, 1, StairsMode::Normal);
			break;
		case 28: // Floor Raise and Crush
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevRaisefloorcrush);
			break;
		case 29: // Build Pillar (no crushing)
			buttonSuccess = EV_BuildPillar(line, args, false);
			break;
		case 30: // Open Pillar
			buttonSuccess = EV_OpenPillar(line, args);
			break;
		case 31: // Stairs Build Down Sync
			buttonSuccess = Hexen_EV_BuildStairs(line, args, -1, StairsMode::Sync);
			break;
		case 32: // Build Stairs Up Sync
			buttonSuccess = Hexen_EV_BuildStairs(line, args, 1, StairsMode::Sync);
			break;
		case 35: // Raise Floor by Value Times 8
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevRaisebyvaluetimes8);
			break;
		case 36: // Lower Floor by Value Times 8
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevLowerbyvaluetimes8);
			break;
		case 40: // Ceiling Lower by Value
			buttonSuccess = Hexen_EV_DoCeiling(line, args, CeilingKind::ClevLowerbyvalue);
			break;
		case 41: // Ceiling Raise by Value
			buttonSuccess = Hexen_EV_DoCeiling(line, args, CeilingKind::ClevRaisebyvalue);
			break;
		case 42: // Ceiling Crush and Raise
			buttonSuccess = Hexen_EV_DoCeiling(line, args, CeilingKind::ClevCrushandraise);
			break;
		case 43: // Ceiling Lower and Crush
			buttonSuccess = Hexen_EV_DoCeiling(line, args, CeilingKind::ClevLowerandcrush);
			break;
		case 44: // Ceiling Crush Stop
			buttonSuccess = Hexen_EV_CeilingCrushStop(line, args);
			break;
		case 45: // Ceiling Crush Raise and Stay
			buttonSuccess = Hexen_EV_DoCeiling(line, args, CeilingKind::ClevCrushraiseandstay);
			break;
		case 46: // Floor Crush Stop
			buttonSuccess = EV_FloorCrushStop(line, args);
			break;
		case 60: // Plat Perpetual Raise
			buttonSuccess = EV_DoHexenPlat(line, args, PlatType::PlatPerpetualraise, 0);
			break;
		case 61: // Plat Stop
			Hexen_EV_StopPlat(line, args);
			break;
		case 62: // Plat Down-Wait-Up-Stay
			buttonSuccess = EV_DoHexenPlat(line, args, PlatType::PlatDownwaitupstay, 0);
			break;
		case 63: // Plat Down-by-Value*8-Wait-Up-Stay
			buttonSuccess = EV_DoHexenPlat(line, args, PlatType::PlatDownbyvaluewaitupstay,
				0);
			break;
		case 64: // Plat Up-Wait-Down-Stay
			buttonSuccess = EV_DoHexenPlat(line, args, PlatType::PlatUpwaitdownstay, 0);
			break;
		case 65: // Plat Up-by-Value*8-Wait-Down-Stay
			buttonSuccess = EV_DoHexenPlat(line, args, PlatType::PlatUpbyvaluewaitdownstay,
				0);
			break;
		case 66: // Floor Lower Instant * 8
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevLowertimes8instant);
			break;
		case 67: // Floor Raise Instant * 8
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevRaisetimes8instant);
			break;
		case 68: // Floor Move to Value * 8
			buttonSuccess = Hexen_EV_DoFloor(line, args, FloorKind::FlevMovetovaluetimes8);
			break;
		case 69: // Ceiling Move to Value * 8
			buttonSuccess = Hexen_EV_DoCeiling(line, args, CeilingKind::ClevMovetovaluetimes8);
			break;
		case 70: // Teleport
			if(side == 0)
			{
				// Only teleport when crossing the front side of a line
				buttonSuccess = EV_HexenTeleport(args[0], mo, true);
			}
			break;
		case 71: // Teleport, no fog
			if(side == 0)
			{
				// Only teleport when crossing the front side of a line
				buttonSuccess = EV_HexenTeleport(args[0], mo, false);
			}
			break;
		case 72:      // Thrust Mobj
			if(!side) // Only thrust on side 0
			{
				P_ThrustMobj(mo, args[0] * (ANG90 / 64),
					args[1] << FRACBITS);
				buttonSuccess = 1;
			}
			break;
		case 73: // Damage Mobj
			if(args[0])
			{
				P_DamageMobj(mo, nullptr, nullptr, args[0]);
			}
			else
			{
				// If arg1 is zero, then guarantee a kill
				P_DamageMobj(mo, nullptr, nullptr, 10000);
			}
			buttonSuccess = 1;
			break;
		case 74: // Teleport_NewMap
			if(side == 0)
			{
				// Only teleport when crossing the front side of a line
				if(!(mo && mo->player && mo->player->playerstate == PlayerState::Dead)) // Players must be alive to teleport
				{
					G_Completed(args[0], args[1], 0, 0);
					buttonSuccess = true;
				}
			}
			break;
		case 75: // Teleport_EndGame
			if(side == 0)
			{
				// Only teleport when crossing the front side of a line
				if(!(mo && mo->player && mo->player->playerstate == PlayerState::Dead)) // Players must be alive to teleport
				{
					buttonSuccess = true;
					if(deathmatch)
					{
						// Winning in deathmatch just goes back to map 1
						G_Completed(1, 0, 0, 0);
					}
					else
					{
						// Starts the Finale
						G_Completed(LEAVE_VICTORY, LEAVE_VICTORY, 0, 0);
					}
				}
			}
			break;
		case 80: // ACS_Execute
			buttonSuccess =
				P_StartACS(args[0], args[1], &args[2], mo, line, side);
			break;
		case 81: // ACS_Suspend
			buttonSuccess = P_SuspendACS(args[0], args[1]);
			break;
		case 82: // ACS_Terminate
			buttonSuccess = P_TerminateACS(args[0], args[1]);
			break;
		case 83: // ACS_LockedExecute
			buttonSuccess = P_StartLockedACS(line, args, mo, side);
			break;
		case 90: // Poly Rotate Left Override
			buttonSuccess = EV_RotatePoly(line, args, 1, true);
			break;
		case 91: // Poly Rotate Right Override
			buttonSuccess = EV_RotatePoly(line, args, -1, true);
			break;
		case 92: // Poly Move Override
			buttonSuccess = EV_MovePoly(line, args, false, true);
			break;
		case 93: // Poly Move Times 8 Override
			buttonSuccess = EV_MovePoly(line, args, true, true);
			break;
		case 94: // Build Pillar Crush
			buttonSuccess = EV_BuildPillar(line, args, true);
			break;
		case 95: // Lower Floor and Ceiling
			buttonSuccess = EV_DoFloorAndCeiling(line, args, false);
			break;
		case 96: // Raise Floor and Ceiling
			buttonSuccess = EV_DoFloorAndCeiling(line, args, true);
			break;
		case 109: // Force Lightning
			buttonSuccess = true;
			P_ForceLightning();
			break;
		case 110: // Light Raise by Value
			buttonSuccess = EV_SpawnLight(line, args, LightType::RaiseByValue);
			break;
		case 111: // Light Lower by Value
			buttonSuccess = EV_SpawnLight(line, args, LightType::LowerByValue);
			break;
		case 112: // Light Change to Value
			buttonSuccess = EV_SpawnLight(line, args, LightType::ChangeToValue);
			break;
		case 113: // Light Fade
			buttonSuccess = EV_SpawnLight(line, args, LightType::Fade);
			break;
		case 114: // Light Glow
			buttonSuccess = EV_SpawnLight(line, args, LightType::Glow);
			break;
		case 115: // Light Flicker
			buttonSuccess = EV_SpawnLight(line, args, LightType::Flicker);
			break;
		case 116: // Light Strobe
			buttonSuccess = EV_SpawnLight(line, args, LightType::Strobe);
			break;
		case 120: // Quake Tremor
			buttonSuccess = A_LocalQuake(args, mo);
			break;
		case 129: // UsePuzzleItem
			buttonSuccess = EV_LineSearchForPuzzleItem(line, args, mo);
			break;
		case 130: // Thing_Activate
			buttonSuccess = EV_ThingActivate(args[0]);
			break;
		case 131: // Thing_Deactivate
			buttonSuccess = EV_ThingDeactivate(args[0]);
			break;
		case 132: // Thing_Remove
			buttonSuccess = EV_ThingRemove(args[0]);
			break;
		case 133: // Thing_Destroy
			buttonSuccess = EV_ThingDestroy(args[0]);
			break;
		case 134: // Thing_Projectile
			buttonSuccess = EV_ThingProjectile(args, 0);
			break;
		case 135: // Thing_Spawn
			buttonSuccess = EV_ThingSpawn(args, 1);
			break;
		case 136: // Thing_ProjectileGravity
			buttonSuccess = EV_ThingProjectile(args, 1);
			break;
		case 137: // Thing_SpawnNoFog
			buttonSuccess = EV_ThingSpawn(args, 0);
			break;
		case 138: // Floor_Waggle
			buttonSuccess = EV_StartFloorWaggle(args[0], args[1],
				args[2], args[3], args[4]);
			break;
		case 140: // Sector_SoundChange
			buttonSuccess = EV_SectorSoundChange(args);
			break;
		default:
			break;
	}
	return buttonSuccess;
}

static void Hexen_P_SpawnSpecials()
{
	sector_t* sector;
	int i;

	//
	//      Init special SECTORs
	//
	sector = sectors;
	for(i = 0; i < numsectors; i++, sector++)
	{
		if(!sector->special)
			continue;
		switch(sector->special)
		{
			case 1: // Phased light
				// Hardcoded base, use sector->lightlevel as the index
				P_SpawnPhasedLight(sector, 80, -1);
				break;
			case 2: // Phased light sequence start
				P_SpawnLightSequence(sector, 1);
				break;
				// Specials 3 & 4 are used by the phased light sequences
		}
	}


	//
	//      Init line EFFECTs
	//
	numlinespecials = 0;
	TaggedLineCount = 0;
	for(i = 0; i < numlines; i++)
	{
		switch(lines[i].special)
		{
			case 100: // Scroll_Texture_Left
			case 101: // Scroll_Texture_Right
			case 102: // Scroll_Texture_Up
			case 103: // Scroll_Texture_Down
				linespeciallist[numlinespecials] = &lines[i];
				numlinespecials++;
				break;
			case 121: // Line_SetIdentification
				if(lines[i].special_args[0])
				{
					if(TaggedLineCount == MAX_TAGGED_LINES)
					{
						Log::Fatal("P_SpawnSpecials: MAX_TAGGED_LINES "
							"({}) exceeded.", MAX_TAGGED_LINES);
					}
					TaggedLines[TaggedLineCount].line = &lines[i];
					TaggedLines[TaggedLineCount++].lineTag = lines[i].special_args[0];
				}
				lines[i].special = 0;
				break;
		}
	}

	//
	//      Init other misc stuff
	//
	P_RemoveAllActiveCeilings();
	P_RemoveAllActivePlats();
	for(i = 0; i < MAXBUTTONS; i++)
		memset(&buttonlist[i], 0, sizeof(button_t));

	// Initialize flat and texture animations
	P_InitFTAnims();
}
