// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

#include <stdint.h>

#include "doomstat.hpp"
#include "p_tick.hpp"
#include "p_setup.hpp"
#include "p_spec.hpp"
#include "d_player.hpp"
#include "p_mobj.hpp"
#include "p_map.hpp"
#include "g_game.hpp"
#include "r_state.hpp"
#include "r_main.hpp"
#include "p_maputl.hpp"
#include "p_enemy.hpp"
#include "p_saveg.hpp"
#include "hu_stuff.hpp"
#include "lprintf.hpp"

#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"

#include "hexen/a_action.hpp"
#include "hexen/p_acs.hpp"
#include "hexen/po_man.hpp"
#include "hexen/sn_sonix.hpp"

#include "sv_save.hpp"

#define MAX_TARGET_PLAYERS 512
#define MOBJ_NULL -1
#define MOBJ_XX_PLAYER -2
#define MAX_MAPS 99

enum struct GameArchiveSegment : int32_t
{
	GameHeader = 101,
	MapHeader,
	World,
	Polyobjs,
	Mobjs,
	Thinkers,
	Scripts,
	Players,
	Sounds,
	Misc,
	End
};

enum struct ThinkClass : int32_t
{
	Null,
	MoveCeiling,
	VerticalDoor,
	MoveFloor,
	PlatRaise,
	InterpretAcs,
	FloorWaggle,
	Light,
	Phase,
	BuildPillar,
	RotatePoly,
	MovePoly,
	PolyDoor
};

typedef struct
{
	ThinkClass tClass;
	think_t thinkerFunc;
	void (*writeFunc)();
	void (*readFunc)();
	void (*restoreFunc)();
	size_t size;
} thinkInfo_t;

static int MobjCount;
static mobj_t** MobjList;
static mobj_t*** TargetPlayerAddrs;
static int TargetPlayerCount;

extern int inv_ptr;
extern int curpos;

typedef struct
{
	size_t size;
	byte* buffer;
} map_archive_t;

static map_archive_t map_archive[MAX_MAPS];
static map_archive_t* ma_p;
static byte* buffer_p;

static dboolean MapArchiveExists(int map)
{
	return (map_archive[map].buffer != nullptr);
}

static void FreeMapArchive()
{
	int map;

	for(map = 0; map < MAX_MAPS; ++map)
		if(map_archive[map].buffer)
		{
			Z_Free(map_archive[map].buffer);
			map_archive[map].buffer = nullptr;
			map_archive[map].size = 0;
		}
}

void SV_StoreMapArchive()
{
	int i;

	for(i = 0; i < MAX_MAPS; ++i)
	{
		P_SAVE_X(map_archive[i].size);

		if(map_archive[i].size)
		{
			P_SAVE_SIZE(map_archive[i].buffer, map_archive[i].size);
		}
	}
}

void SV_RestoreMapArchive()
{
	int i;

	FreeMapArchive();

	for(i = 0; i < MAX_MAPS; ++i)
	{
		P_LOAD_X(map_archive[i].size);

		if(map_archive[i].size)
		{
			map_archive[i].buffer = static_cast<decltype(map_archive[i].buffer)>(Z_Malloc(map_archive[i].size));
			P_LOAD_SIZE(map_archive[i].buffer, map_archive[i].size);
		}
	}
}

static dboolean SV_IsMobjThinker(thinker_t* th)
{
	return th->function == reinterpret_cast<think_t>(P_MobjThinker) ||
		(th->function == reinterpret_cast<think_t>(P_RemoveThinkerDelayed) && th->references);
}

static void CheckBuffer(size_t size)
{
	size_t delta = buffer_p - ma_p->buffer;

	while(delta + size > ma_p->size)
	{
		ma_p->size += 1024;
		ma_p->buffer = static_cast<decltype(ma_p->buffer)>(Z_Realloc(ma_p->buffer, ma_p->size));
		buffer_p = ma_p->buffer + delta;
	}
}

static void SV_Read(void* buffer, size_t size)
{
	if(buffer_p - ma_p->buffer + size > ma_p->size)
	{
		I_Error("Invalid map archive in SV_Read");
	}

	memcpy(buffer, buffer_p, size);
	buffer_p += size;
}

static byte SV_ReadByte()
{
	byte result;
	SV_Read(&result, sizeof(byte));
	return result;
}

static unsigned short SV_ReadWord()
{
	unsigned short result;
	SV_Read(&result, sizeof(unsigned short));
	return result;
}

static int SV_ReadLong()
{
	int result;
	SV_Read(&result, sizeof(int));
	return result;
}

static uint64_t SV_ReadFlags()
{
	uint64_t result;
	SV_Read(&result, sizeof(uint64_t));
	return result;
}

static void SV_Write(const void* buffer, size_t size)
{
	CheckBuffer(size);
	memcpy(buffer_p, buffer, size);
	buffer_p += size;
}

static void SV_WriteByte(byte val)
{
	SV_Write(&val, sizeof(byte));
}

static void SV_WriteWord(unsigned short val)
{
	SV_Write(&val, sizeof(unsigned short));
}

static void SV_WriteLong(unsigned int val)
{
	SV_Write(&val, sizeof(unsigned int));
}

static void SV_WriteFlags(uint64_t val)
{
	SV_Write(&val, sizeof(uint64_t));
}

static void SV_OpenRead(int map)
{
	ma_p = &map_archive[map];
	buffer_p = ma_p->buffer;
}

static void SV_OpenWrite(int map)
{
	ma_p = &map_archive[map];
	if(ma_p->buffer)
	{
		Z_Free(ma_p->buffer);
	}
	ma_p->size = 1024;
	ma_p->buffer = static_cast<decltype(ma_p->buffer)>(Z_Malloc(ma_p->size));
	buffer_p = ma_p->buffer;
}

static int GetMobjNum(mobj_t* mobj)
{
	if(mobj == nullptr)
	{
		return MOBJ_NULL;
	}
	if(mobj->player)
	{
		return MOBJ_XX_PLAYER;
	}
	return mobj->archiveNum;
}

static void SetMobjPtr(mobj_t** ptr, unsigned int archiveNum)
{
	if(archiveNum == MOBJ_NULL)
	{
		*ptr = nullptr;
	}
	else if(archiveNum == MOBJ_XX_PLAYER)
	{
		if(TargetPlayerCount == MAX_TARGET_PLAYERS)
		{
			I_Error("RestoreMobj: exceeded MAX_TARGET_PLAYERS");
		}
		TargetPlayerAddrs[TargetPlayerCount++] = ptr;
		*ptr = nullptr;
	}
	else
	{
		P_SetTarget(ptr, MobjList[archiveNum]);
	}
}

static void StreamInMobjSpecials(mobj_t* mobj)
{
	mobj->special1.i = SV_ReadLong();
	SetMobjPtr(&mobj->special1.m, SV_ReadLong());
	mobj->special2.i = SV_ReadLong();
	SetMobjPtr(&mobj->special2.m, SV_ReadLong());
}

static void StreamIn_mobj_t(mobj_t* str)
{
	unsigned int i;

	// "is the mobj marked for deletion?"
	str->index = SV_ReadByte();

	// fixed_t x, y, z;
	str->x = SV_ReadLong();
	str->y = SV_ReadLong();
	str->z = SV_ReadLong();

	// struct mobj_s *snext, *sprev;
	// Pointer values are discarded:
	str->snext = nullptr;
	str->sprev = nullptr;

	// angle_t angle;
	str->angle = SV_ReadLong();

	// SpriteId sprite;
	str->sprite = static_cast<SpriteId>(SV_ReadLong());

	// int frame;
	str->frame = SV_ReadLong();

	// struct mobj_s *bnext, *bprev;
	// Values are read but discarded; this will be restored when the thing's
	// position is set.
	str->bnext = nullptr;
	str->bprev = nullptr;

	// struct subsector_s *subsector;
	// Read but discard: pointer will be restored when thing position is set.
	str->subsector = nullptr;

	// fixed_t floorz, ceilingz;
	str->floorz = SV_ReadLong();
	str->ceilingz = SV_ReadLong();

	// fixed_t floorpic;
	str->floorpic = SV_ReadLong();

	// fixed_t radius, height;
	str->radius = SV_ReadLong();
	str->height = SV_ReadLong();

	// fixed_t momx, momy, momz;
	str->momx = SV_ReadLong();
	str->momy = SV_ReadLong();
	str->momz = SV_ReadLong();

	// int validcount;
	str->validcount = SV_ReadLong();

	// MobjType type;
	str->type = static_cast<MobjType>(SV_ReadLong());

	// mobjinfo_t *info;
	// Pointer value is read but discarded.
	str->info = nullptr;

	// int tics;
	str->tics = SV_ReadLong();

	// state_t *state;
	// Restore as index into states table.
	i = SV_ReadLong();
	str->state = &states[i];

	// int damage;
	str->damage = SV_ReadLong();

	// int flags;
	str->flags = SV_ReadFlags();

	// int flags2;
	str->flags2 = SV_ReadFlags();

	// specialval_t special1;
	// specialval_t special2;
	// Read in special values: there are special cases to deal with with
	// mobj pointers.
	StreamInMobjSpecials(str);

	// int health;
	str->health = SV_ReadLong();

	// short movedir;
	str->movedir = SV_ReadWord();

	// short movecount;
	str->movecount = SV_ReadWord();

	// struct mobj_s *target;
	i = SV_ReadLong();
	SetMobjPtr(&str->target, i);

	// int reactiontime;
	str->reactiontime = SV_ReadLong();

	// int threshold;
	str->threshold = SV_ReadLong();

	// struct player_s *player;
	// Saved as player number.
	i = SV_ReadLong();
	if(i == 0)
	{
		str->player = nullptr;
	}
	else
	{
		str->player = &players[i - 1];
		str->player->mo = str;
	}

	// int lastlook;
	str->lastlook = SV_ReadLong();

	// fixed_t floorclip;
	str->floorclip = SV_ReadLong();

	// int archiveNum;
	str->archiveNum = SV_ReadLong();

	// short tid;
	str->tid = SV_ReadWord();

	// int special;
	str->special = SV_ReadLong();

	// int args[5];
	for(i = 0; i < 5; ++i)
	{
		str->special_args[i] = SV_ReadLong();
	}

	str->friction = ORIG_FRICTION;
	str->gravity = FRACUNIT;
	str->alpha = 1.f;
}

static void StreamOutMobjSpecials(mobj_t* mobj)
{
	dboolean corpse;

	corpse = (mobj->flags & MF_CORPSE) != 0;

	SV_WriteLong(mobj->type == MobjType::HexenKorax ? 0 : mobj->special1.i);
	SV_WriteLong(corpse ? MOBJ_NULL : GetMobjNum(mobj->special1.m));
	SV_WriteLong(mobj->special2.i);
	SV_WriteLong(corpse ? MOBJ_NULL : GetMobjNum(mobj->special2.m));
}

static void StreamOut_mobj_t(mobj_t* str)
{
	int i;

	// store "marked for deletion" flag
	SV_WriteByte(str->thinker.function == reinterpret_cast<think_t>(P_RemoveThinkerDelayed));

	// fixed_t x, y, z;
	SV_WriteLong(str->x);
	SV_WriteLong(str->y);
	SV_WriteLong(str->z);

	// angle_t angle;
	SV_WriteLong(str->angle);

	// SpriteId sprite;
	SV_WriteLong(std::to_underlying(str->sprite));

	// int frame;
	SV_WriteLong(str->frame);

	// fixed_t floorz, ceilingz;
	SV_WriteLong(str->floorz);
	SV_WriteLong(str->ceilingz);

	// fixed_t floorpic;
	SV_WriteLong(str->floorpic);

	// fixed_t radius, height;
	SV_WriteLong(str->radius);
	SV_WriteLong(str->height);

	// fixed_t momx, momy, momz;
	SV_WriteLong(str->momx);
	SV_WriteLong(str->momy);
	SV_WriteLong(str->momz);

	// int validcount;
	SV_WriteLong(str->validcount);

	// MobjType type;
	SV_WriteLong(std::to_underlying(str->type));

	// int tics;
	SV_WriteLong(str->tics);

	// state_t *state;
	// Save as index into the states table.
	SV_WriteLong(str->state - states);

	// int damage;
	SV_WriteLong(str->damage);

	// int flags;
	SV_WriteFlags(str->flags);

	// int flags2;
	SV_WriteFlags(str->flags2);

	// specialval_t special1;
	// specialval_t special2;
	// There are lots of special cases for the special values:
	StreamOutMobjSpecials(str);

	// int health;
	SV_WriteLong(str->health);

	// int movedir;
	SV_WriteWord(str->movedir);

	// int movecount;
	SV_WriteWord(str->movecount);

	// struct mobj_s *target;
	if((str->flags & MF_CORPSE) != 0)
	{
		SV_WriteLong(MOBJ_NULL);
	}
	else
	{
		SV_WriteLong(GetMobjNum(str->target));
	}

	// int reactiontime;
	SV_WriteLong(str->reactiontime);

	// int threshold;
	SV_WriteLong(str->threshold);

	// struct player_s *player;
	// Stored as index into players[] array, if there is a player pointer.
	if(str->player != nullptr)
	{
		SV_WriteLong(str->player - players + 1);
	}
	else
	{
		SV_WriteLong(0);
	}

	// int lastlook;
	SV_WriteLong(str->lastlook);

	// fixed_t floorclip;
	SV_WriteLong(str->floorclip);

	// int archiveNum;
	SV_WriteLong(str->archiveNum);

	// short tid;
	SV_WriteWord(str->tid);

	// int special;
	SV_WriteLong(str->special);

	// int args[5];
	for(i = 0; i < 5; ++i)
	{
		SV_WriteLong(str->special_args[i]);
	}
}

static void StreamIn_floormove_t(floormove_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = sectors + i;

	// floor_e type;
	str->type = static_cast<FloorKind>(SV_ReadLong());

	// int crush;
	str->crush = SV_ReadLong();

	// int direction;
	str->direction = SV_ReadLong();

	// newspecial_t newspecial;
	str->newspecial.special = SV_ReadWord();
	str->newspecial.flags = SV_ReadLong();
	str->newspecial.damage.amount = SV_ReadWord();
	str->newspecial.damage.leakrate = SV_ReadByte();
	str->newspecial.damage.interval = SV_ReadByte();

	// short texture;
	str->texture = SV_ReadWord();

	// fixed_t floordestheight;
	str->floordestheight = SV_ReadLong();

	// fixed_t speed;
	str->speed = SV_ReadLong();

	// int delayCount;
	str->delayCount = SV_ReadLong();

	// int delayTotal;
	str->delayTotal = SV_ReadLong();

	// fixed_t stairsDelayHeight;
	str->stairsDelayHeight = SV_ReadLong();

	// fixed_t stairsDelayHeightDelta;
	str->stairsDelayHeightDelta = SV_ReadLong();

	// fixed_t resetHeight;
	str->resetHeight = SV_ReadLong();

	// short resetDelay;
	str->resetDelay = SV_ReadWord();

	// short resetDelayCount;
	str->resetDelayCount = SV_ReadWord();

	// byte textureChange;
	str->textureChange = SV_ReadByte();
}

static void StreamOut_floormove_t(floormove_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// floor_e type;
	SV_WriteLong(std::to_underlying(str->type));

	// int crush;
	SV_WriteLong(str->crush);

	// int direction;
	SV_WriteLong(str->direction);

	// newspecial_t newspecial;
	SV_WriteWord(str->newspecial.special);
	SV_WriteLong(str->newspecial.flags);
	SV_WriteWord(str->newspecial.damage.amount);
	SV_WriteByte(str->newspecial.damage.leakrate);
	SV_WriteByte(str->newspecial.damage.interval);

	// short texture;
	SV_WriteWord(str->texture);

	// fixed_t floordestheight;
	SV_WriteLong(str->floordestheight);

	// fixed_t speed;
	SV_WriteLong(str->speed);

	// int delayCount;
	SV_WriteLong(str->delayCount);

	// int delayTotal;
	SV_WriteLong(str->delayTotal);

	// fixed_t stairsDelayHeight;
	SV_WriteLong(str->stairsDelayHeight);

	// fixed_t stairsDelayHeightDelta;
	SV_WriteLong(str->stairsDelayHeightDelta);

	// fixed_t resetHeight;
	SV_WriteLong(str->resetHeight);

	// short resetDelay;
	SV_WriteWord(str->resetDelay);

	// short resetDelayCount;
	SV_WriteWord(str->resetDelayCount);

	// byte textureChange;
	SV_WriteByte(str->textureChange);
}

static void StreamIn_plat_t(plat_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = sectors + i;

	// fixed_t speed;
	str->speed = SV_ReadLong();

	// fixed_t low;
	str->low = SV_ReadLong();

	// fixed_t high;
	str->high = SV_ReadLong();

	// int wait;
	str->wait = SV_ReadLong();

	// int count;
	str->count = SV_ReadLong();

	// plat_e status;
	str->status = static_cast<PlatState>(SV_ReadLong());

	// plat_e oldstatus;
	str->oldstatus = static_cast<PlatState>(SV_ReadLong());

	// int crush;
	str->crush = SV_ReadLong();

	// int tag;
	str->tag = SV_ReadLong();

	// plattype_e type;
	str->type = static_cast<PlatType>(SV_ReadLong());
}

static void StreamOut_plat_t(plat_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// fixed_t speed;
	SV_WriteLong(str->speed);

	// fixed_t low;
	SV_WriteLong(str->low);

	// fixed_t high;
	SV_WriteLong(str->high);

	// int wait;
	SV_WriteLong(str->wait);

	// int count;
	SV_WriteLong(str->count);

	// PlatState status;
	SV_WriteLong(std::to_underlying(str->status));

	// PlatState oldstatus;
	SV_WriteLong(std::to_underlying(str->oldstatus));

	// int crush;
	SV_WriteLong(str->crush);

	// int tag;
	SV_WriteLong(str->tag);

	// plattype_e type;
	SV_WriteLong(std::to_underlying(str->type));
}

static void StreamIn_ceiling_t(ceiling_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = sectors + i;

	// ceiling_e type;
	str->type = static_cast<CeilingKind>(SV_ReadLong());

	// fixed_t bottomheight, topheight;
	str->bottomheight = SV_ReadLong();
	str->topheight = SV_ReadLong();

	// fixed_t speed;
	str->speed = SV_ReadLong();

	// int crush;
	str->crush = SV_ReadLong();

	// int direction;
	str->direction = SV_ReadLong();

	// int tag;
	str->tag = SV_ReadLong();

	// int olddirection;
	str->olddirection = SV_ReadLong();
}

static void StreamOut_ceiling_t(ceiling_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// ceiling_e type;
	SV_WriteLong(std::to_underlying(str->type));

	// fixed_t bottomheight, topheight;
	SV_WriteLong(str->bottomheight);
	SV_WriteLong(str->topheight);

	// fixed_t speed;
	SV_WriteLong(str->speed);

	// int crush;
	SV_WriteLong(str->crush);

	// int direction;
	SV_WriteLong(str->direction);

	// int tag;
	SV_WriteLong(str->tag);

	// int olddirection;
	SV_WriteLong(str->olddirection);
}

static void StreamIn_light_t(light_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = sectors + i;

	// lighttype_t type;
	str->type = static_cast<LightType>(SV_ReadLong());

	// int value1;
	str->value1 = SV_ReadLong();

	// int value2;
	str->value2 = SV_ReadLong();

	// int tics1;
	str->tics1 = SV_ReadLong();

	// int tics2;
	str->tics2 = SV_ReadLong();

	// int count;
	str->count = SV_ReadLong();
}

static void StreamOut_light_t(light_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// lighttype_t type;
	SV_WriteLong(std::to_underlying(str->type));

	// int value1;
	SV_WriteLong(str->value1);

	// int value2;
	SV_WriteLong(str->value2);

	// int tics1;
	SV_WriteLong(str->tics1);

	// int tics2;
	SV_WriteLong(str->tics2);

	// int count;
	SV_WriteLong(str->count);
}

static void StreamIn_vldoor_t(vldoor_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = &sectors[i];

	// vldoor_e type;
	str->type = static_cast<VerticalDoorType>(SV_ReadLong());

	// fixed_t topheight;
	str->topheight = SV_ReadLong();

	// fixed_t speed;
	str->speed = SV_ReadLong();

	// int direction;
	str->direction = SV_ReadLong();

	// int topwait;
	str->topwait = SV_ReadLong();

	// int topcountdown;
	str->topcountdown = SV_ReadLong();
}

static void StreamOut_vldoor_t(vldoor_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// vldoor_e type;
	SV_WriteLong(std::to_underlying(str->type));

	// fixed_t topheight;
	SV_WriteLong(str->topheight);

	// fixed_t speed;
	SV_WriteLong(str->speed);

	// int direction;
	SV_WriteLong(str->direction);

	// int topwait;
	SV_WriteLong(str->topwait);

	// int topcountdown;
	SV_WriteLong(str->topcountdown);
}

static void StreamIn_phase_t(phase_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = &sectors[i];

	// int index;
	str->index = SV_ReadLong();

	// int base;
	str->base = SV_ReadLong();
}

static void StreamOut_phase_t(phase_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// int index;
	SV_WriteLong(str->index);

	// int base;
	SV_WriteLong(str->base);
}

static void StreamIn_acs_t(acs_t* str)
{
	int i;

	// mobj_t *activator;
	i = SV_ReadLong();
	SetMobjPtr(&str->activator, i);

	// line_t *line;
	i = SV_ReadLong();
	if(i != -1)
	{
		str->line = &lines[i];
	}
	else
	{
		str->line = nullptr;
	}

	// int side;
	str->side = SV_ReadLong();

	// int number;
	str->number = SV_ReadLong();

	// int infoIndex;
	str->infoIndex = SV_ReadLong();

	// int delayCount;
	str->delayCount = SV_ReadLong();

	// int stack[ACS_STACK_DEPTH];
	for(i = 0; i < ACS_STACK_DEPTH; ++i)
	{
		str->stack[i] = SV_ReadLong();
	}

	// int stackPtr;
	str->stackPtr = SV_ReadLong();

	// int vars[MAX_ACS_SCRIPT_VARS];
	for(i = 0; i < MAX_ACS_SCRIPT_VARS; ++i)
	{
		str->vars[i] = SV_ReadLong();
	}

	// int *ip;
	str->ip = SV_ReadLong();
}

static void StreamOut_acs_t(acs_t* str)
{
	int i;

	// mobj_t *activator;
	SV_WriteLong(GetMobjNum(str->activator));

	// line_t *line;
	if(str->line != nullptr)
	{
		SV_WriteLong(str->line - lines);
	}
	else
	{
		SV_WriteLong(-1);
	}

	// int side;
	SV_WriteLong(str->side);

	// int number;
	SV_WriteLong(str->number);

	// int infoIndex;
	SV_WriteLong(str->infoIndex);

	// int delayCount;
	SV_WriteLong(str->delayCount);

	// int stack[ACS_STACK_DEPTH];
	for(i = 0; i < ACS_STACK_DEPTH; ++i)
	{
		SV_WriteLong(str->stack[i]);
	}

	// int stackPtr;
	SV_WriteLong(str->stackPtr);

	// int vars[MAX_ACS_SCRIPT_VARS];
	for(i = 0; i < MAX_ACS_SCRIPT_VARS; ++i)
	{
		SV_WriteLong(str->vars[i]);
	}

	// int *ip;
	SV_WriteLong(str->ip);
}

static void StreamIn_polyevent_t(polyevent_t* str)
{
	// int polyobj;
	str->polyobj = SV_ReadLong();

	// int speed;
	str->speed = SV_ReadLong();

	// unsigned int dist;
	str->dist = SV_ReadLong();

	// int angle;
	str->angle = SV_ReadLong();

	// fixed_t xSpeed;
	str->xSpeed = SV_ReadLong();

	// fixed_t ySpeed;
	str->ySpeed = SV_ReadLong();
}

static void StreamOut_polyevent_t(polyevent_t* str)
{
	// int polyobj;
	SV_WriteLong(str->polyobj);

	// int speed;
	SV_WriteLong(str->speed);

	// unsigned int dist;
	SV_WriteLong(str->dist);

	// int angle;
	SV_WriteLong(str->angle);

	// fixed_t xSpeed;
	SV_WriteLong(str->xSpeed);

	// fixed_t ySpeed;
	SV_WriteLong(str->ySpeed);
}

static void StreamIn_pillar_t(pillar_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = &sectors[i];

	// int ceilingSpeed;
	str->ceilingSpeed = SV_ReadLong();

	// int floorSpeed;
	str->floorSpeed = SV_ReadLong();

	// int floordest;
	str->floordest = SV_ReadLong();

	// int ceilingdest;
	str->ceilingdest = SV_ReadLong();

	// int direction;
	str->direction = SV_ReadLong();

	// int crush;
	str->crush = SV_ReadLong();
}

static void StreamOut_pillar_t(pillar_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// int ceilingSpeed;
	SV_WriteLong(str->ceilingSpeed);

	// int floorSpeed;
	SV_WriteLong(str->floorSpeed);

	// int floordest;
	SV_WriteLong(str->floordest);

	// int ceilingdest;
	SV_WriteLong(str->ceilingdest);

	// int direction;
	SV_WriteLong(str->direction);

	// int crush;
	SV_WriteLong(str->crush);
}

static void StreamIn_polydoor_t(polydoor_t* str)
{
	// int polyobj;
	str->polyobj = SV_ReadLong();

	// int speed;
	str->speed = SV_ReadLong();

	// int dist;
	str->dist = SV_ReadLong();

	// int totalDist;
	str->totalDist = SV_ReadLong();

	// int direction;
	str->direction = SV_ReadLong();

	// fixed_t xSpeed, ySpeed;
	str->xSpeed = SV_ReadLong();
	str->ySpeed = SV_ReadLong();

	// int tics;
	str->tics = SV_ReadLong();

	// int waitTics;
	str->waitTics = SV_ReadLong();

	// podoortype_t type;
	str->type = static_cast<PolyDoorType>(SV_ReadLong());

	// dboolean close;
	str->close = SV_ReadLong();
}

static void StreamOut_polydoor_t(polydoor_t* str)
{
	// int polyobj;
	SV_WriteLong(str->polyobj);

	// int speed;
	SV_WriteLong(str->speed);

	// int dist;
	SV_WriteLong(str->dist);

	// int totalDist;
	SV_WriteLong(str->totalDist);

	// int direction;
	SV_WriteLong(str->direction);

	// fixed_t xSpeed, ySpeed;
	SV_WriteLong(str->xSpeed);
	SV_WriteLong(str->ySpeed);

	// int tics;
	SV_WriteLong(str->tics);

	// int waitTics;
	SV_WriteLong(str->waitTics);

	// podoortype_t type;
	SV_WriteLong(std::to_underlying(str->type));

	// dboolean close;
	SV_WriteLong(str->close);
}

static void StreamIn_planeWaggle_t(planeWaggle_t* str)
{
	int i;

	// sector_t *sector;
	i = SV_ReadLong();
	str->sector = &sectors[i];

	// fixed_t originalHeight;
	str->originalHeight = SV_ReadLong();

	// fixed_t accumulator;
	str->accumulator = SV_ReadLong();

	// fixed_t accDelta;
	str->accDelta = SV_ReadLong();

	// fixed_t targetScale;
	str->targetScale = SV_ReadLong();

	// fixed_t scale;
	str->scale = SV_ReadLong();

	// fixed_t scaleDelta;
	str->scaleDelta = SV_ReadLong();

	// int ticker;
	str->ticker = SV_ReadLong();

	// int state;
	str->state = SV_ReadLong();
}

static void StreamOut_planeWaggle_t(planeWaggle_t* str)
{
	// sector_t *sector;
	SV_WriteLong(str->sector - sectors);

	// fixed_t originalHeight;
	SV_WriteLong(str->originalHeight);

	// fixed_t accumulator;
	SV_WriteLong(str->accumulator);

	// fixed_t accDelta;
	SV_WriteLong(str->accDelta);

	// fixed_t targetScale;
	SV_WriteLong(str->targetScale);

	// fixed_t scale;
	SV_WriteLong(str->scale);

	// fixed_t scaleDelta;
	SV_WriteLong(str->scaleDelta);

	// int ticker;
	SV_WriteLong(str->ticker);

	// int state;
	SV_WriteLong(str->state);
}

void SV_Init()
{
	FreeMapArchive();
}

static void AssertSegment(GameArchiveSegment segType)
{
	if(SV_ReadLong() != std::to_underlying(segType))
	{
		I_Error("Corrupt save game: Segment [%d] failed alignment check",
			std::to_underlying(segType));
	}
}

static void ArchiveWorld()
{
	int i;
	int j;
	sector_t* sec;
	line_t* li;
	side_t* si;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::World));
	for(i = 0, sec = sectors; i < numsectors; i++, sec++)
	{
		SV_WriteWord(sec->floorheight >> FRACBITS);
		SV_WriteWord(sec->ceilingheight >> FRACBITS);
		SV_WriteWord(sec->floorpic);
		SV_WriteWord(sec->ceilingpic);
		SV_WriteWord(sec->lightlevel);
		SV_WriteWord(sec->special);
		SV_WriteWord(sec->tag);
		SV_WriteWord(std::to_underlying(sec->seqType));
	}
	for(i = 0, li = lines; i < numlines; i++, li++)
	{
		SV_WriteLong(li->flags);
		// TODO: how does this work? it's a short
		SV_WriteByte(li->special);
		SV_WriteLong(li->special_args[0]);
		SV_WriteLong(li->special_args[1]);
		SV_WriteLong(li->special_args[2]);
		SV_WriteLong(li->special_args[3]);
		SV_WriteLong(li->special_args[4]);
		for(j = 0; j < 2; j++)
		{
			if(li->sidenum[j] == NO_INDEX)
			{
				continue;
			}
			si = &sides[li->sidenum[j]];
			SV_WriteWord(si->textureoffset >> FRACBITS);
			SV_WriteWord(si->rowoffset >> FRACBITS);
			SV_WriteWord(si->toptexture);
			SV_WriteWord(si->bottomtexture);
			SV_WriteWord(si->midtexture);
		}
	}
}

static void UnarchiveWorld()
{
	int i;
	int j;
	sector_t* sec;
	line_t* li;
	side_t* si;

	AssertSegment(GameArchiveSegment::World);
	for(i = 0, sec = sectors; i < numsectors; i++, sec++)
	{
		sec->floorheight = SV_ReadWord() << FRACBITS;
		sec->ceilingheight = SV_ReadWord() << FRACBITS;
		sec->floorpic = SV_ReadWord();
		sec->ceilingpic = SV_ReadWord();
		sec->lightlevel = SV_ReadWord();
		sec->special = SV_ReadWord();
		sec->tag = SV_ReadWord();
		sec->seqType = static_cast<SeqType>(SV_ReadWord());
		sec->ceilingdata = nullptr;
		sec->floordata = nullptr;
		sec->lightingdata = nullptr;
		sec->soundtarget = nullptr;
	}
	for(i = 0, li = lines; i < numlines; i++, li++)
	{
		li->flags = SV_ReadLong();
		li->special = SV_ReadByte();
		li->special_args[0] = SV_ReadLong();
		li->special_args[1] = SV_ReadLong();
		li->special_args[2] = SV_ReadLong();
		li->special_args[3] = SV_ReadLong();
		li->special_args[4] = SV_ReadLong();
		for(j = 0; j < 2; j++)
		{
			if(li->sidenum[j] == NO_INDEX)
			{
				continue;
			}
			si = &sides[li->sidenum[j]];
			si->textureoffset = SV_ReadWord() << FRACBITS;
			si->rowoffset = SV_ReadWord() << FRACBITS;
			si->toptexture = SV_ReadWord();
			si->bottomtexture = SV_ReadWord();
			si->midtexture = SV_ReadWord();
		}
	}
}

static void SetMobjArchiveNums()
{
	mobj_t* mobj;
	thinker_t* thinker;

	MobjCount = 0;
	for(thinker = thinkercap.next; thinker != &thinkercap;
		thinker = thinker->next)
	{
		if(SV_IsMobjThinker(thinker))
		{
			mobj = (mobj_t*)thinker;
			if(mobj->player)
			{
				// Skipping player mobjs
				continue;
			}
			mobj->archiveNum = MobjCount++;
		}
	}
}

static void ArchiveMobjs()
{
	int count;
	thinker_t* thinker;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::Mobjs));
	SV_WriteLong(MobjCount);
	count = 0;
	for(thinker = thinkercap.next; thinker != &thinkercap;
		thinker = thinker->next)
	{
		if(!SV_IsMobjThinker(thinker))
		{
			// Not a mobj thinker
			continue;
		}
		if(((mobj_t*)thinker)->player)
		{
			// Skipping player mobjs
			continue;
		}
		count++;
		StreamOut_mobj_t((mobj_t*)thinker);
	}
	if(count != MobjCount)
	{
		I_Error("ArchiveMobjs: bad mobj count");
	}
}

static void UnarchiveMobjs()
{
	int i;
	mobj_t* mobj;

	AssertSegment(GameArchiveSegment::Mobjs);
	TargetPlayerAddrs = static_cast<mobj_t***>(Z_Malloc(MAX_TARGET_PLAYERS * sizeof(mobj_t**)));
	TargetPlayerCount = 0;
	MobjCount = SV_ReadLong();
	MobjList = static_cast<mobj_t**>(Z_Malloc(MobjCount * sizeof(mobj_t*)));
	for(i = 0; i < MobjCount; i++)
	{
		MobjList[i] = static_cast<mobj_t*>(Z_MallocLevel(sizeof(mobj_t)));
		memset(MobjList[i], 0, sizeof(mobj_t));
	}
	for(i = 0; i < MobjCount; i++)
	{
		int references;

		mobj = MobjList[i];
		StreamIn_mobj_t(mobj);

		// Restore broken pointers.
		mobj->info = &mobjinfo[std::to_underlying(mobj->type)];

		// "marked for deletion"
		if(mobj->index)
		{
			mobj->index = -1;
			mobj->thinker.function = reinterpret_cast<think_t>(P_RemoveThinkerDelayed);

			references = mobj->thinker.references;
			P_AddThinker(&mobj->thinker);
			mobj->thinker.references = references;

			continue;
		}

		P_SetThingPosition(mobj);
		mobj->floorz = mobj->subsector->sector->floorheight;
		mobj->ceilingz = mobj->subsector->sector->ceilingheight;

		mobj->thinker.function = reinterpret_cast<think_t>(P_MobjThinker);

		references = mobj->thinker.references;
		P_AddThinker(&mobj->thinker);
		mobj->thinker.references = references;
	}
	map_format.build_mobj_thing_id_list();
	P_InitCreatureCorpseQueue(true); // true = scan for corpses
}

static void RestoreFloorWaggle(planeWaggle_t* th)
{
	th->sector->floordata = reinterpret_cast<void*>(th->thinker.function);
}

static void RestoreBuildPillar(pillar_t* th)
{
	th->sector->floordata = reinterpret_cast<void*>(th->thinker.function);
}

static void RestoreVerticalDoor(vldoor_t* th)
{
	th->sector->ceilingdata = reinterpret_cast<void*>(th->thinker.function);
}

static void RestoreMoveFloor(floormove_t* th)
{
	th->sector->floordata = reinterpret_cast<void*>(th->thinker.function);
}

static void RestorePlatRaise(plat_t* plat)
{
	plat->sector->floordata = reinterpret_cast<void*>(T_PlatRaise);
	P_AddActivePlat(plat);
}

static void RestoreMoveCeiling(ceiling_t* ceiling)
{
	ceiling->sector->ceilingdata = reinterpret_cast<void*>(T_MoveCeiling);
	P_AddActiveCeiling(ceiling);
}


// The thinker functions do not share a parameter list, so each entry needs a cast.
#define THINKER_FUNC(a_function) reinterpret_cast<void (*)()>(a_function)

static thinkInfo_t ThinkerInfo[] = {
	{
		ThinkClass::MoveFloor,
		THINKER_FUNC(T_MoveFloor),
		THINKER_FUNC(StreamOut_floormove_t),
		THINKER_FUNC(StreamIn_floormove_t),
		THINKER_FUNC(RestoreMoveFloor),
		sizeof(floormove_t)
	},
	{
		ThinkClass::PlatRaise,
		THINKER_FUNC(T_PlatRaise),
		THINKER_FUNC(StreamOut_plat_t),
		THINKER_FUNC(StreamIn_plat_t),
		THINKER_FUNC(RestorePlatRaise),
		sizeof(plat_t)
	},
	{
		ThinkClass::MoveCeiling,
		THINKER_FUNC(T_MoveCeiling),
		THINKER_FUNC(StreamOut_ceiling_t),
		THINKER_FUNC(StreamIn_ceiling_t),
		THINKER_FUNC(RestoreMoveCeiling),
		sizeof(ceiling_t)
	},
	{
		ThinkClass::Light,
		THINKER_FUNC(T_Light),
		THINKER_FUNC(StreamOut_light_t),
		THINKER_FUNC(StreamIn_light_t),
		nullptr,
		sizeof(light_t)
	},
	{
		ThinkClass::VerticalDoor,
		THINKER_FUNC(T_VerticalDoor),
		THINKER_FUNC(StreamOut_vldoor_t),
		THINKER_FUNC(StreamIn_vldoor_t),
		THINKER_FUNC(RestoreVerticalDoor),
		sizeof(vldoor_t)
	},
	{
		ThinkClass::Phase,
		THINKER_FUNC(T_Phase),
		THINKER_FUNC(StreamOut_phase_t),
		THINKER_FUNC(StreamIn_phase_t),
		nullptr,
		sizeof(phase_t)
	},
	{
		ThinkClass::InterpretAcs,
		THINKER_FUNC(T_InterpretACS),
		THINKER_FUNC(StreamOut_acs_t),
		THINKER_FUNC(StreamIn_acs_t),
		nullptr,
		sizeof(acs_t)
	},
	{
		ThinkClass::RotatePoly,
		THINKER_FUNC(T_RotatePoly),
		THINKER_FUNC(StreamOut_polyevent_t),
		THINKER_FUNC(StreamIn_polyevent_t),
		nullptr,
		sizeof(polyevent_t)
	},
	{
		ThinkClass::BuildPillar,
		THINKER_FUNC(T_BuildPillar),
		THINKER_FUNC(StreamOut_pillar_t),
		THINKER_FUNC(StreamIn_pillar_t),
		THINKER_FUNC(RestoreBuildPillar),
		sizeof(pillar_t)
	},
	{
		ThinkClass::MovePoly,
		THINKER_FUNC(T_MovePoly),
		THINKER_FUNC(StreamOut_polyevent_t),
		THINKER_FUNC(StreamIn_polyevent_t),
		nullptr,
		sizeof(polyevent_t)
	},
	{
		ThinkClass::PolyDoor,
		THINKER_FUNC(T_PolyDoor),
		THINKER_FUNC(StreamOut_polydoor_t),
		THINKER_FUNC(StreamIn_polydoor_t),
		nullptr,
		sizeof(polydoor_t)
	},
	{
		ThinkClass::FloorWaggle,
		THINKER_FUNC(T_FloorWaggle),
		THINKER_FUNC(StreamOut_planeWaggle_t),
		THINKER_FUNC(StreamIn_planeWaggle_t),
		THINKER_FUNC(RestoreFloorWaggle),
		sizeof(planeWaggle_t)
	},
	{ThinkClass::Null, nullptr, nullptr, nullptr, nullptr, 0},
};

#undef THINKER_FUNC

static void ArchiveThinkers()
{
	thinker_t* thinker;
	thinkInfo_t* info;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::Thinkers));
	for(thinker = thinkercap.next; thinker != &thinkercap;
		thinker = thinker->next)
	{
		for(info = ThinkerInfo; info->tClass != ThinkClass::Null; info++)
		{
			if(thinker->function == info->thinkerFunc)
			{
				SV_WriteByte(std::to_underlying(info->tClass));
				reinterpret_cast<void (*)(thinker_t*)>(info->writeFunc)(thinker);
				break;
			}
		}
	}
	// Add a termination marker
	SV_WriteByte(std::to_underlying(ThinkClass::Null));
}

static void UnarchiveThinkers()
{
	ThinkClass tClass;
	thinker_t* thinker;
	thinkInfo_t* info;

	AssertSegment(GameArchiveSegment::Thinkers);
	while((tClass = static_cast<ThinkClass>(SV_ReadByte())) != ThinkClass::Null)
	{
		for(info = ThinkerInfo; info->tClass != ThinkClass::Null; info++)
		{
			if(tClass == info->tClass)
			{
				thinker = static_cast<thinker_t*>(Z_MallocLevel(info->size));
				memset(thinker, 0, info->size);
				reinterpret_cast<void (*)(thinker_t*)>(info->readFunc)(thinker);
				thinker->function = info->thinkerFunc;
				if(info->restoreFunc)
				{
					reinterpret_cast<void (*)(thinker_t*)>(info->restoreFunc)(thinker);
				}
				P_AddThinker(thinker);
				break;
			}
		}
		if(info->tClass == ThinkClass::Null)
		{
			I_Error("UnarchiveThinkers: Unknown tClass %d in "
				"savegame", tClass);
		}
	}
}

static void ArchiveScripts()
{
	int i;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::Scripts));
	for(i = 0; i < ACScriptCount; i++)
	{
		SV_WriteWord(std::to_underlying(ACSInfo[i].state));
		SV_WriteWord(ACSInfo[i].waitValue);
	}

	for(i = 0; i < MAX_ACS_MAP_VARS; ++i)
	{
		SV_WriteLong(MapVars[i]);
	}
}

static void UnarchiveScripts()
{
	int i;

	AssertSegment(GameArchiveSegment::Scripts);
	for(i = 0; i < ACScriptCount; i++)
	{
		ACSInfo[i].state = static_cast<AcsState>(SV_ReadWord());
		ACSInfo[i].waitValue = SV_ReadWord();
	}

	for(i = 0; i < MAX_ACS_MAP_VARS; ++i)
	{
		MapVars[i] = SV_ReadLong();
	}
}

static void ArchiveMisc()
{
	int ix;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::Misc));
	for(ix = 0; ix < g_maxplayers; ix++)
	{
		SV_WriteLong(localQuakeHappening[ix]);
	}
}

static void UnarchiveMisc()
{
	int ix;

	AssertSegment(GameArchiveSegment::Misc);
	for(ix = 0; ix < g_maxplayers; ix++)
	{
		localQuakeHappening[ix] = SV_ReadLong();
	}
}

static void RemoveAllThinkers()
{
	thinker_t* thinker;
	thinker_t* nextThinker;

	thinker = thinkercap.next;
	while(thinker != &thinkercap)
	{
		nextThinker = thinker->next;
		if(SV_IsMobjThinker(thinker))
		{
			P_RemoveMobj((mobj_t*)thinker);
			P_RemoveThinkerDelayed(thinker); // fix mobj leak
		}
		else
		{
			Z_Free(thinker);
		}
		thinker = nextThinker;
	}
	P_InitThinkers();
}

static void ArchiveSounds()
{
	seqnode_t* node;
	sector_t* sec;
	int difference;
	int i;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::Sounds));

	// Save the sound sequences
	SV_WriteLong(ActiveSequences);
	for(node = SequenceListHead; node; node = node->next)
	{
		SV_WriteLong(node->sequence);
		SV_WriteLong(node->delayTics);
		SV_WriteLong(node->volume);
		SV_WriteLong(SN_GetSequenceOffset(node->sequence,
			node->sequencePtr));
		SV_WriteLong(std::to_underlying(node->currentSoundID));
		for(i = 0; i < po_NumPolyobjs; i++)
		{
			if(node->mobj == (mobj_t*)&polyobjs[i].startSpot)
			{
				break;
			}
		}
		if(i == po_NumPolyobjs)
		{
			// Sound is attached to a sector, not a polyobj
			sec = R_PointInSector(node->mobj->x, node->mobj->y);
			difference = (int)((byte*)sec
				- (byte*)&sectors[0]) / sizeof(sector_t);
			SV_WriteLong(0); // 0 -- sector sound origin
		}
		else
		{
			SV_WriteLong(1); // 1 -- polyobj sound origin
			difference = i;
		}
		SV_WriteLong(difference);
	}
}

static void UnarchiveSounds()
{
	int i;
	int numSequences;
	int sequence;
	int delayTics;
	int volume;
	int seqOffset;
	int soundID;
	int polySnd;
	int secNum;
	mobj_t* sndMobj;

	AssertSegment(GameArchiveSegment::Sounds);

	// Reload and restart all sound sequences
	numSequences = SV_ReadLong();
	i = 0;
	while(i < numSequences)
	{
		sequence = SV_ReadLong();
		delayTics = SV_ReadLong();
		volume = SV_ReadLong();
		seqOffset = SV_ReadLong();

		soundID = SV_ReadLong();
		polySnd = SV_ReadLong();
		secNum = SV_ReadLong();
		if(!polySnd)
		{
			sndMobj = (mobj_t*)&sectors[secNum].soundorg;
		}
		else
		{
			sndMobj = (mobj_t*)&polyobjs[secNum].startSpot;
		}
		SN_StartSequence(sndMobj, sequence);
		SN_ChangeNodeData(i, seqOffset, delayTics, volume, static_cast<SfxId>(soundID));
		i++;
	}
}

static void ArchivePolyobjs()
{
	int i;

	SV_WriteLong(std::to_underlying(GameArchiveSegment::Polyobjs));
	SV_WriteLong(po_NumPolyobjs);
	for(i = 0; i < po_NumPolyobjs; i++)
	{
		SV_WriteLong(polyobjs[i].tag);
		SV_WriteLong(polyobjs[i].angle);
		SV_WriteLong(polyobjs[i].startSpot.x);
		SV_WriteLong(polyobjs[i].startSpot.y);
	}
}

static void UnarchivePolyobjs()
{
	int i;
	fixed_t deltaX;
	fixed_t deltaY;

	AssertSegment(GameArchiveSegment::Polyobjs);
	if(SV_ReadLong() != po_NumPolyobjs)
	{
		I_Error("UnarchivePolyobjs: Bad polyobj count");
	}
	for(i = 0; i < po_NumPolyobjs; i++)
	{
		if(SV_ReadLong() != polyobjs[i].tag)
		{
			I_Error("UnarchivePolyobjs: Invalid polyobj tag");
		}
		PO_RotatePolyobj(polyobjs[i].tag, (angle_t)SV_ReadLong());
		deltaX = SV_ReadLong() - polyobjs[i].startSpot.x;
		deltaY = SV_ReadLong() - polyobjs[i].startSpot.y;
		PO_MovePolyobj(polyobjs[i].tag, deltaX, deltaY);
	}
}

void SV_SaveMap()
{
	// Initialize the output buffer
	SV_OpenWrite(gamemap);

	// Place a header marker
	SV_WriteLong(std::to_underlying(GameArchiveSegment::MapHeader));

	// Write the level timer
	SV_WriteLong(leveltime);

	// Set the mobj archive numbers
	SetMobjArchiveNums();

	ArchiveWorld();
	ArchivePolyobjs();
	ArchiveMobjs();
	ArchiveThinkers();
	ArchiveScripts();
	ArchiveSounds();
	ArchiveMisc();

	// Place a termination marker
	SV_WriteLong(std::to_underlying(GameArchiveSegment::End));
}

void SV_LoadMap()
{
	// Load a base level
	G_InitNew(gameskill, gameepisode, gamemap, false);

	// Remove all thinkers
	RemoveAllThinkers();

	// Initialize the input buffer
	SV_OpenRead(gamemap);

	AssertSegment(GameArchiveSegment::MapHeader);

	// Read the level timer
	leveltime = SV_ReadLong();

	UnarchiveWorld();
	UnarchivePolyobjs();
	UnarchiveMobjs();
	UnarchiveThinkers();
	UnarchiveScripts();
	UnarchiveSounds();
	UnarchiveMisc();

	AssertSegment(GameArchiveSegment::End);

	// Free mobj list and save buffer
	Z_Free(MobjList);
}

void SV_MapTeleport(int map, int position)
{
	int i;
	int j;
	int key_i;
	player_t playerBackup[MAX_MAXPLAYERS];
	mobj_t* targetPlayerMobj;
	mobj_t* mobj;
	int inventoryPtr;
	int currentInvPos;
	dboolean rClass;
	dboolean playerWasReborn;
	dboolean oldWeaponowned[std::to_underlying(WeaponType::HexenCount)];
	int oldKeys[std::to_underlying(Card::Count)];
	int oldPieces = 0;
	int bestWeapon;

	memset(oldKeys, 0, sizeof(oldKeys));
	memset(oldWeaponowned, 0, sizeof(oldWeaponowned));

	if(!deathmatch)
	{
		if(dsda_MapCluster(gamemap) == dsda_MapCluster(map))
		{
			// Same cluster - save map without saving player mobjs
			SV_SaveMap();
		}
		else
		{
			// Entering new cluster - clear map archive
			SV_Init();
		}
	}

	// Store player structs for later
	rClass = randomclass;
	randomclass = false;
	for(i = 0; i < g_maxplayers; i++)
	{
		playerBackup[i] = players[i];
	}

	// Save some globals that get trashed during the load
	inventoryPtr = inv_ptr;
	currentInvPos = curpos;

	// Only SV_LoadMap() uses TargetPlayerAddrs, so it's NULLed here
	// for the following check (player mobj redirection)
	TargetPlayerAddrs = nullptr;

	dsda_UpdateGameMap(1, map);

	if(!deathmatch && MapArchiveExists(gamemap))
	{
		// Unarchive map
		SV_LoadMap();
		P_MapStart();
	}
	else
	{
		// New map
		G_InitNew(gameskill, gameepisode, gamemap, false);

		P_MapStart();

		// Destroy all freshly spawned players
		for(i = 0; i < g_maxplayers; i++)
		{
			if(playeringame[i])
			{
				P_RemoveMobj(players[i].mo);
			}
		}
	}

	// Restore player structs
	targetPlayerMobj = nullptr;
	for(i = 0; i < g_maxplayers; i++)
	{
		if(!playeringame[i])
		{
			continue;
		}
		players[i] = playerBackup[i];
		players[i].attacker = nullptr;
		players[i].poisoner = nullptr;

		if(netgame)
		{
			if(players[i].playerstate == PlayerState::Dead)
			{
				// In a network game, force all players to be alive
				players[i].playerstate = PlayerState::Reborn;
			}
			if(!deathmatch)
			{
				// Cooperative net-play, retain keys and weapons
				for(key_i = 0; key_i < std::to_underlying(Card::Count); ++key_i)
					oldKeys[key_i] = players[i].cards[key_i];
				oldPieces = players[i].pieces;
				for(j = 0; j < std::to_underlying(WeaponType::HexenCount); j++)
				{
					oldWeaponowned[j] = players[i].weaponowned[j];
				}
			}
		}
		playerWasReborn = (players[i].playerstate == PlayerState::Reborn);
		if(deathmatch)
		{
			memset(players[i].frags, 0, sizeof(players[i].frags));
			mobj = P_SpawnMobj(playerstarts[0][i].x,
				playerstarts[0][i].y, 0,
				MobjType::HexenPlayerFighter);
			players[i].mo = mobj;
			G_DeathMatchSpawnPlayer(i);
			P_RemoveMobj(mobj);
		}
		else
		{
			P_SpawnPlayer(i, &playerstarts[position][i]);
		}

		if(playerWasReborn && netgame && !deathmatch)
		{
			// Restore keys and weapons when reborn in co-op
			for(key_i = 0; key_i < std::to_underlying(Card::Count); ++key_i)
				players[i].cards[key_i] = oldKeys[key_i];
			players[i].pieces = oldPieces;
			for(bestWeapon = 0, j = 0; j < std::to_underlying(WeaponType::HexenCount); j++)
			{
				if(oldWeaponowned[j])
				{
					bestWeapon = j;
					players[i].weaponowned[j] = true;
				}
			}
			players[i].ammo[std::to_underlying(AmmoType::Mana1)] = 25;
			players[i].ammo[std::to_underlying(AmmoType::Mana2)] = 25;
			if(bestWeapon)
			{
				// Bring up the best weapon
				players[i].pendingweapon = static_cast<WeaponType>(bestWeapon);
			}
		}

		if(targetPlayerMobj == nullptr)
		{
			// The poor sap
			targetPlayerMobj = players[i].mo;
		}
	}
	randomclass = rClass;

	// Redirect anything targeting a player mobj
	if(TargetPlayerAddrs)
	{
		for(i = 0; i < TargetPlayerCount; i++)
		{
			*TargetPlayerAddrs[i] = targetPlayerMobj;
		}
		Z_Free(TargetPlayerAddrs);
	}

	// Destroy all things touching players
	for(i = 0; i < g_maxplayers; i++)
	{
		if(playeringame[i])
		{
			P_TeleportMove(players[i].mo, players[i].mo->x, players[i].mo->y, false);
		}
	}

	// Restore trashed globals
	inv_ptr = inventoryPtr;
	curpos = currentInvPos;

	// Launch waiting scripts
	if(!deathmatch)
	{
		P_CheckACSStore();
	}

	P_MapEnd();
}
