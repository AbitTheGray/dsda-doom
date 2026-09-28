// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  all external data is defined here
 *  most of the data is loaded into different structures at run time
 *  some internal structures shared by many modules are here
 */

#pragma once

#include "config.h"
#include "doomtype.hpp"
#include "m_fixed.hpp"

// mapthing_t::options - difficulty/skill settings/filters.
// Doom and Hexen give bits 4 to 8 different meanings, so some values repeat.
enum struct MapThingFlag : uint32_t
{
	// Skill flags.
	Easy = Bit<uint32_t>(0u),
	Normal = Bit<uint32_t>(1u),
	Hard = Bit<uint32_t>(2u),
	// Deaf monsters/do not react to sound.
	Ambush = Bit<uint32_t>(3u),

	/* killough 11/98 */
	NotSingle = Bit<uint32_t>(4u),
	NotDm = Bit<uint32_t>(5u),
	NotCoop = Bit<uint32_t>(6u),
	Friend = Bit<uint32_t>(7u),
	Reserved = Bit<uint32_t>(8u),

	// hexen
	Dormant = Bit<uint32_t>(4u),
	Fighter = Bit<uint32_t>(5u),
	Cleric = Bit<uint32_t>(6u),
	Mage = Bit<uint32_t>(7u),
	GSingle = Bit<uint32_t>(8u),
	GCoop = Bit<uint32_t>(9u),
	GDeathmatch = Bit<uint32_t>(10u),

	// zdoom
	Translucent = Bit<uint32_t>(11u),
	Invisible = Bit<uint32_t>(12u),
	Friendly = Bit<uint32_t>(13u),
	StandStill = Bit<uint32_t>(14u),
	CountSecret = Bit<uint32_t>(15u),
	Skill1 = Bit<uint32_t>(16u),
	Skill2 = Bit<uint32_t>(17u),
	Skill3 = Bit<uint32_t>(18u),
	Skill4 = Bit<uint32_t>(19u),
	Skill5 = Bit<uint32_t>(20u),
};
ENUM_FLAGS_FUNC(MapThingFlag)

// line_t::flags - LineDef attributes.
enum struct LineFlag : uint32_t
{
	// Solid, is an obstacle.
	Blocking = Bit<uint32_t>(0u),
	// Blocks monsters only.
	BlockMonsters = Bit<uint32_t>(1u),
	// Backside will not be drawn if not two sided.
	TwoSided = Bit<uint32_t>(2u),

	// If a texture is pegged, the texture will have
	// the end exposed to air held constant at the
	// top or bottom of the texture (stairs or pulled
	// down things) and will move with a height change
	// of one of the neighbor sectors.
	// Unpegged textures always have the first row of
	// the texture at the top pixel of the line for both
	// top and bottom textures (use next to windows).

	// upper texture unpegged
	DontPegTop = Bit<uint32_t>(3u),
	// lower texture unpegged
	DontPegBottom = Bit<uint32_t>(4u),
	// In AutoMap: don't map as two sided: IT'S A SECRET!
	Secret = Bit<uint32_t>(5u),
	// Sound rendering: don't let sound cross two of these.
	SoundBlock = Bit<uint32_t>(6u),
	// Don't draw on the automap at all.
	DontDraw = Bit<uint32_t>(7u),
	// Set if already seen, thus drawn in automap.
	Mapped = Bit<uint32_t>(8u),
	//jff 3/21/98 Set if line absorbs use by player
	//allow multiple push/switch triggers to be used on one push
	PassUse = Bit<uint32_t>(9u),

	// Reserved by EE
	// SoM 9/02/02: 3D Middletexture flag!
	Eternity = Bit<uint32_t>(10u),

	// haleyjd 05/02/06: Although it was believed until now that a reserved line
	// flag was unnecessary, a problem with Ultimate DOOM E2M7 has disproven this
	// theory. It has roughly 1000 linedefs with 0xFE00 masked into the flags, so
	// making the next line flag reserved and using it to toggle off ALL extended
	// flags will preserve compatibility for such maps. I have been told this map
	// is one of the first ever created, so it may have something to do with that.
	Reserved = Bit<uint32_t>(11u),

	// mbf21
	BlockLandMonsters = Bit<uint32_t>(12u),
	BlockPlayers = Bit<uint32_t>(13u),

	// extensions
	MonstersCanActivate = Bit<uint32_t>(14u), // zdoom
	BlockEverything = Bit<uint32_t>(15u), // zdoom

	RepeatSpecial = Bit<uint32_t>(16u), // hexen

	// udmf
	ClipMidTex = Bit<uint32_t>(17u),
	BlockSight = Bit<uint32_t>(18u),
	BlockHitscan = Bit<uint32_t>(19u),
	BlockProjectiles = Bit<uint32_t>(20u),
	BlockUse = Bit<uint32_t>(21u),
	BlockFloaters = Bit<uint32_t>(22u),
	JumpOver = Bit<uint32_t>(23u),
	MidTex3D = Bit<uint32_t>(24u),
	MidTex3DImpassible = Bit<uint32_t>(25u),
	FirstSideOnly = Bit<uint32_t>(26u),
	Revealed = Bit<uint32_t>(27u),
	CheckSwitchRange = Bit<uint32_t>(28u),
	WrapMidTex = Bit<uint32_t>(29u),

	// The flags each format knows about.
	Vanilla = 0x01ffu, // Blocking to Mapped
	Boom = 0x03ffu, // Vanilla plus PassUse
	MBF21 = 0x3fffu, // Boom plus BlockLandMonsters and BlockPlayers (and the reserved bits)
};
ENUM_FLAGS_FUNC(LineFlag)

// hexen_maplinedef_t::flags - the line flags as a Hexen-format map stores them.
// Bits 0 to 8 are the same as in LineFlag, bits 10 to 12 hold the activation type.
enum struct HexenLineFlag : uint32_t
{
	// hexen
	RepeatSpecial = Bit<uint32_t>(9u), // special is repeatable

	// zdoom
	// ZDoom extends the Hexen map format with bits Hexen leaves unused, in this same field.
	// They stay here so one type describes the field. P_TranslateHexenLineFlags ignores them.
	MonstersCanActivate = Bit<uint32_t>(13u), // Monsters and players can activate
	BlockPlayers = Bit<uint32_t>(14u), // Blocks players
	BlockEverything = Bit<uint32_t>(15u), // Blocks everything
};
ENUM_FLAGS_FUNC(HexenLineFlag)

// Bits 10 to 12 are not flags but a 3-bit number: the line's activation type.
// The translate functions map it to SPAC_ values, each format in its own way.
// Being a number, it has no enumerator and is read only through this function.
[[nodiscard]]
inline constexpr uint32_t HexenLineSpacIndex(const HexenLineFlag flags)
{
	return (std::to_underlying(flags) >> 10u) & Bits<uint32_t>(3u);
}

#ifdef __cplusplus
extern "C"
{
#endif

// The most basic types we use, portability.

//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum struct MapLump : int32_t
{
	Label,    // A separator, name, ExMx or MAPxx
	Things,   // Monsters, items..
	Linedefs, // LineDefs, from editing
	Sidedefs, // SideDefs, from editing
	Vertexes, // Vertices, edited and BSP splits generated
	Segs,     // LineSegs, from LineDefs split by BSP
	Ssectors, // SubSectors, list of LineSegs
	Nodes,    // BSP nodes
	Sectors,  // Sectors, from editing
	Reject,   // LUT, sector-sector visibility
	Blockmap, // LUT, motion clipping, walls/grid element
	Behavior
};

#define ML_TEXTMAP 1

#ifdef _MSC_VER // proff: This is the same as __attribute__ ((packed)) in GNUC
#pragma pack(push)
#pragma pack(1)
#endif //_MSC_VER

// A single Vertex.
typedef struct
{
	short x, y;
}
	PACKEDATTR mapvertex_t;

// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef struct
{
	short textureoffset;
	short rowoffset;
	char toptexture[8];
	char bottomtexture[8];
	char midtexture[8];
	short sector; // Front sector, towards viewer.
}
	PACKEDATTR mapsidedef_t;

// A LineDef, as used for editing, and as input to the BSP builder.

typedef struct
{
	unsigned short v1;
	unsigned short v2;
	unsigned short flags;
	byte special;
	byte arg1;
	byte arg2;
	byte arg3;
	byte arg4;
	byte arg5;
	unsigned short sidenum[2];
}
	PACKEDATTR hexen_maplinedef_t;

typedef struct
{
	unsigned short v1;
	unsigned short v2;
	unsigned short flags;
	short special;
	short tag;
	// proff 07/23/2006 - support more than 32768 sidedefs
	// use the unsigned value and special case the -1
	// sidenum[1] will be -1 (NO_INDEX) if one sided
	unsigned short sidenum[2];
}
	PACKEDATTR doom_maplinedef_t;

// Updated to 32-bit
#define NO_INDEX ((unsigned int)-1)

// Sector definition, from editing.
typedef struct
{
	short floorheight;
	short ceilingheight;
	char floorpic[8];
	char ceilingpic[8];
	short lightlevel;
	short special;
	short tag;
}
	PACKEDATTR mapsector_t;

// SubSector, as generated by BSP.
typedef struct
{
	unsigned short numsegs;
	unsigned short firstseg; // Index of first one; segs are stored sequentially.
}
	PACKEDATTR mapsubsector_t;

typedef struct
{
	unsigned short numsegs;
	int firstseg;
}
	PACKEDATTR mapsubsector_v4_t;

typedef struct
{
	unsigned int numsegs;
}
	PACKEDATTR mapsubsector_znod_t;

// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
typedef struct
{
	unsigned short v1;
	unsigned short v2;
	short angle;
	unsigned short linedef;
	short side;
	short offset;
}
	PACKEDATTR mapseg_t;

typedef struct
{
	int v1;
	int v2;
	unsigned short angle;
	unsigned short linedef;
	short side;
	unsigned short offset;
}
	PACKEDATTR mapseg_v4_t;

typedef struct
{
	unsigned int v1, v2;
	unsigned short linedef;
	unsigned char side;
}
	PACKEDATTR mapseg_znod_t;

typedef struct
{
	unsigned int v1, v2;
	unsigned int linedef;
	unsigned char side;
}
	PACKEDATTR mapseg_znod2_t;

// BSP node structure.

// Indicate a leaf.
// e6y: support for extended nodes
#define NF_SUBSECTOR    0x80000000

typedef struct
{
	short x; // Partition line from (x,y) to x+dx,y+dy)
	short y;
	short dx;
	short dy;
	// Bounding box for each child, clip against view frustum.
	short bbox[2][4];
	// If NF_SUBSECTOR its a subsector, else it's a node of another subtree.
	unsigned short children[2];
}
	PACKEDATTR mapnode_t;

typedef struct
{
	short x; // Partition line from (x,y) to x+dx,y+dy)
	short y;
	short dx;
	short dy;
	// Bounding box for each child, clip against view frustum.
	short bbox[2][4];
	// If NF_SUBSECTOR its a subsector, else it's a node of another subtree.
	int children[2];
}
	PACKEDATTR mapnode_v4_t;

typedef struct
{
	short x; // Partition line from (x,y) to x+dx,y+dy)
	short y;
	short dx;
	short dy;
	// Bounding box for each child, clip against view frustum.
	short bbox[2][4];
	// If NF_SUBSECTOR its a subsector, else it's a node of another subtree.
	int children[2];
}
	PACKEDATTR mapnode_znod_t;

typedef struct
{
	int x; // Partition line from (x,y) to x+dx,y+dy)
	int y;
	int dx;
	int dy;
	// Bounding box for each child, clip against view frustum.
	short bbox[2][4];
	// If NF_SUBSECTOR its a subsector, else it's a node of another subtree.
	int children[2];
}
	PACKEDATTR mapnode_znod2_t;

// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.

typedef struct
{
	short tid;
	fixed_t x;
	fixed_t y;
	fixed_t height;
	short angle;
	short type;
	MapThingFlag options;
	int special;
	int special_args[5];
	fixed_t gravity;
	fixed_t health;
	float alpha;
} mapthing_t;

typedef struct
{
	short tid;
	short x;
	short y;
	short height;
	short angle;
	short type;
	short options;
	byte special;
	byte arg1;
	byte arg2;
	byte arg3;
	byte arg4;
	byte arg5;
}
	PACKEDATTR hexen_mapthing_t;

typedef struct
{
	short x;
	short y;
	short angle;
	short type;
	short options;
}
	PACKEDATTR doom_mapthing_t;

// line activation
#define SPAC_NONE     0x0000
#define SPAC_CROSS    0x0001
#define SPAC_USE      0x0002
#define SPAC_MCROSS   0x0004
#define SPAC_IMPACT   0x0008
#define SPAC_PUSH     0x0010
#define SPAC_PCROSS   0x0020
#define SPAC_USEBACK  0x0040
#define SPAC_MPUSH    0x0080
#define SPAC_MUSE     0x0100
#define SPAC_ANYCROSS 0x0200
#define SPAC_DAMAGE   0x0400
#define SPAC_DEATH    0x0800

#ifdef _MSC_VER
#pragma pack(pop)
#endif //_MSC_VER

#ifdef __cplusplus
}
#endif
