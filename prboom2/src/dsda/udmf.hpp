// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA UDMF

#pragma once

#include <inttypes.h>

#include <functional>
#include <string_view>

#include "cpp/Util.hpp"
#include "r_defs.hpp"

// udmf_thing_t::flags
enum struct UdmfThingFlag : uint32_t
{
	Skill1 = Bit<uint32_t>(0u),
	Skill2 = Bit<uint32_t>(1u),
	Skill3 = Bit<uint32_t>(2u),
	Skill4 = Bit<uint32_t>(3u),
	Skill5 = Bit<uint32_t>(4u),
	Ambush = Bit<uint32_t>(5u),
	Single = Bit<uint32_t>(6u),
	Dm = Bit<uint32_t>(7u),
	Coop = Bit<uint32_t>(8u),
	Friend = Bit<uint32_t>(9u),
	Dormant = Bit<uint32_t>(10u),
	Class1 = Bit<uint32_t>(11u),
	Class2 = Bit<uint32_t>(12u),
	Class3 = Bit<uint32_t>(13u),
	Standing = Bit<uint32_t>(14u),
	StrifeAlly = Bit<uint32_t>(15u),
	Translucent = Bit<uint32_t>(16u),
	Invisible = Bit<uint32_t>(17u),
	CountSecret = Bit<uint32_t>(18u),
};
ENUM_FLAGS_FUNC(UdmfThingFlag)

// udmf_line_t::flags
enum struct UdmfLineFlag : uint64_t
{
	Blocking = Bit<uint64_t>(0u),
	BlockMonsters = Bit<uint64_t>(1u),
	TwoSided = Bit<uint64_t>(2u),
	DontPegTop = Bit<uint64_t>(3u),
	DontPegBottom = Bit<uint64_t>(4u),
	Secret = Bit<uint64_t>(5u),
	SoundBlock = Bit<uint64_t>(6u),
	DontDraw = Bit<uint64_t>(7u),
	Mapped = Bit<uint64_t>(8u),
	PassUse = Bit<uint64_t>(9u),
	Translucent = Bit<uint64_t>(10u),
	JumpOver = Bit<uint64_t>(11u),
	BlockFloaters = Bit<uint64_t>(12u),
	PlayerCross = Bit<uint64_t>(13u),
	PlayerUse = Bit<uint64_t>(14u),
	MonsterCross = Bit<uint64_t>(15u),
	MonsterUse = Bit<uint64_t>(16u),
	Impact = Bit<uint64_t>(17u),
	PlayerPush = Bit<uint64_t>(18u),
	MonsterPush = Bit<uint64_t>(19u),
	MissileCross = Bit<uint64_t>(20u),
	RepeatSpecial = Bit<uint64_t>(21u),
	PlayerUseBack = Bit<uint64_t>(22u),
	AnyCross = Bit<uint64_t>(23u),
	MonsterActivate = Bit<uint64_t>(24u),
	BlockPlayers = Bit<uint64_t>(25u),
	BlockEverything = Bit<uint64_t>(26u),
	FirstSideOnly = Bit<uint64_t>(27u),
	ZoneBoundary = Bit<uint64_t>(28u),
	ClipMidTex = Bit<uint64_t>(29u),
	WrapMidTex = Bit<uint64_t>(30u),
	MidTex3D = Bit<uint64_t>(31u),
	MidTex3DImpassible = Bit<uint64_t>(32u),
	CheckSwitchRange = Bit<uint64_t>(33u),
	BlockProjectiles = Bit<uint64_t>(34u),
	BlockUse = Bit<uint64_t>(35u),
	BlockSight = Bit<uint64_t>(36u),
	BlockHitscan = Bit<uint64_t>(37u),
	Transparent = Bit<uint64_t>(38u),
	Revealed = Bit<uint64_t>(39u),
	NoSkyWalls = Bit<uint64_t>(40u),
	DrawFullHeight = Bit<uint64_t>(41u),
	DamageSpecial = Bit<uint64_t>(42u),
	DeathSpecial = Bit<uint64_t>(43u),
	BlockLandMonsters = Bit<uint64_t>(44u),
};
ENUM_FLAGS_FUNC(UdmfLineFlag)

// udmf_sector_t::flags
enum struct UdmfSectorFlag : uint16_t
{
	LightFloorAbsolute = Bit<uint16_t>(0u),
	LightCeilingAbsolute = Bit<uint16_t>(1u),
	Silent = Bit<uint16_t>(2u),
	NoFallingDamage = Bit<uint16_t>(3u),
	DropActors = Bit<uint16_t>(4u),
	NoRespawn = Bit<uint16_t>(5u),
	Hidden = Bit<uint16_t>(6u),
	WaterZone = Bit<uint16_t>(7u),
	DamageTerrainEffect = Bit<uint16_t>(8u),
	DamageHazard = Bit<uint16_t>(9u),
	NoAttack = Bit<uint16_t>(10u),
};
ENUM_FLAGS_FUNC(UdmfSectorFlag)

#ifdef __cplusplus
extern "C"
{
#endif

	enum struct UdmfNamespace : int32_t
	{
		None,
		Doom,
		Heretic,
		Hexen,
		Dsda,
	};

	extern UdmfNamespace udmf_namespace;

	typedef struct
	{
		int id;
		char* moreids;
		int v1;
		int v2;
		int special;
		int arg0;
		int arg1;
		int arg2;
		int arg3;
		int arg4;
		char* arg0str;
		int sidefront;
		int sideback;
		float alpha;
		int locknumber;
		int automapstyle;
		int health;
		int healthgroup;
		UdmfLineFlag flags;
	} udmf_line_t;

	typedef struct
	{
		int offsetx;
		int offsety;
		char texturetop[9];
		char texturebottom[9];
		char texturemiddle[9];
		int sector;
		float scalex_top;
		float scaley_top;
		float scalex_mid;
		float scaley_mid;
		float scalex_bottom;
		float scaley_bottom;
		float offsetx_top;
		float offsety_top;
		float offsetx_mid;
		float offsety_mid;
		float offsetx_bottom;
		float offsety_bottom;
		int light;
		int light_top;
		int light_mid;
		int light_bottom;
		float xscroll;
		float yscroll;
		float xscrolltop;
		float yscrolltop;
		float xscrollmid;
		float yscrollmid;
		float xscrollbottom;
		float yscrollbottom;
		SideFlag flags;
	} udmf_side_t;

	typedef struct
	{
		const char* x;
		const char* y;
	} udmf_vertex_t;

#define UDMF_SCROLL_TEXTURE 0x01
#define UDMF_SCROLL_STATIC  0x02
#define UDMF_SCROLL_PLAYER  0x04
#define UDMF_SCROLL_MONSTER 0x08

#define UDMF_THRUST_STATIC     0x01
#define UDMF_THRUST_PLAYER     0x02
#define UDMF_THRUST_MONSTER    0x04
#define UDMF_THRUST_PROJECTILE 0x08
#define UDMF_THRUST_GROUNDED   0x10
#define UDMF_THRUST_AIRBORNE   0x20
#define UDMF_THRUST_CEILING    0x40
#define UDMF_THRUST_WINDTHRUST 0x80

	typedef struct
	{
		int heightfloor;
		int heightceiling;
		char texturefloor[9];
		char textureceiling[9];
		int lightlevel;
		int special;
		int id;
		char* skyfloor;
		char* skyceiling;
		char* colormap;
		char* moreids;
		float xpanningfloor;
		float ypanningfloor;
		float xpanningceiling;
		float ypanningceiling;
		float xscalefloor;
		float yscalefloor;
		float xscaleceiling;
		float yscaleceiling;
		float rotationfloor;
		float rotationceiling;
		int lightfloor;
		int lightceiling;
		const char* gravity;
		int damageamount;
		int damageinterval;
		int leakiness;
		float xscrollfloor;
		float yscrollfloor;
		int scrollfloormode;
		float xscrollceiling;
		float yscrollceiling;
		int scrollceilingmode;
		char* xthrust;
		char* ythrust;
		int thrustgroup;
		int thrustlocation;
		char* frictionfactor;
		char* movefactor;
		UdmfSectorFlag flags;
	} udmf_sector_t;

	typedef struct
	{
		int id;
		const char* x;
		const char* y;
		const char* height;
		int angle;
		int type;
		int special;
		int arg0;
		int arg1;
		int arg2;
		int arg3;
		int arg4;
		char* arg0str;
		const char* gravity;
		const char* health;
		float scalex;
		float scaley;
		float scale;
		float alpha;
		int floatbobphase;
		UdmfThingFlag flags;
	} udmf_thing_t;

	typedef struct
	{
		size_t num_lines;
		udmf_line_t* lines;

		size_t num_sides;
		udmf_side_t* sides;

		size_t num_vertices;
		udmf_vertex_t* vertices;

		size_t num_sectors;
		udmf_sector_t* sectors;

		size_t num_things;
		udmf_thing_t* things;
	} udmf_t;

	extern udmf_t udmf;


#ifdef __cplusplus
}
#endif

/// Called with the finished message of a parse error; must not return.
using udmf_errorfunc = std::function<void(std::string_view message)>;

void dsda_ParseUDMF(const unsigned char* buffer, size_t length, udmf_errorfunc err);
