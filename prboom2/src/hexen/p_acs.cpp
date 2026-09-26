// SPDX-License-Identifier: GPL-2.0-or-later

#include "doomdef.hpp"
#include "doomstat.hpp"
#include "m_misc.hpp"
#include "m_random.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "w_wad.hpp"
#include "lprintf.hpp"
#include "r_main.hpp"
#include "p_tick.hpp"
#include "p_spec.hpp"
#include "p_inter.hpp"

#include "hexen/p_things.hpp"
#include "hexen/po_man.hpp"
#include "hexen/sn_sonix.hpp"

#include "dsda/id_list.hpp"
#include "dsda/map_format.hpp"

#include "p_acs.hpp"

#define MAX_SCRIPT_ARGS 3
#define SCRIPT_CONTINUE 0
#define SCRIPT_STOP 1
#define SCRIPT_TERMINATE 2
#define OPEN_SCRIPTS_BASE 1000
#define PRINT_BUFFER_SIZE 256
#define GAME_SINGLE_PLAYER 0
#define GAME_NET_COOPERATIVE 1
#define GAME_NET_DEATHMATCH 2
#define TEXTURE_TOP 0
#define TEXTURE_MIDDLE 1
#define TEXTURE_BOTTOM 2

#ifdef _MSC_VER // proff: This is the same as __attribute__ ((packed)) in GNUC
#pragma pack(push)
#pragma pack(1)
#endif //_MSC_VER

typedef struct
{
	int marker;
	int infoOffset;
	int code;
}
	PACKEDATTR acsHeader_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif //_MSC_VER

static void StartOpenACS(int number, int infoIndex, int offset);
static void ScriptFinished(int number);
static dboolean TagBusy(int tag);
static dboolean AddToACSStore(int map, int number, byte* args);
static int GetACSIndex(int number);
static void Push(int value);
static int Pop();
static int Top();
static void Drop();

static int CmdNOP();
static int CmdTerminate();
static int CmdSuspend();
static int CmdPushNumber();
static int CmdLSpec1();
static int CmdLSpec2();
static int CmdLSpec3();
static int CmdLSpec4();
static int CmdLSpec5();
static int CmdLSpec1Direct();
static int CmdLSpec2Direct();
static int CmdLSpec3Direct();
static int CmdLSpec4Direct();
static int CmdLSpec5Direct();
static int CmdAdd();
static int CmdSubtract();
static int CmdMultiply();
static int CmdDivide();
static int CmdModulus();
static int CmdEQ();
static int CmdNE();
static int CmdLT();
static int CmdGT();
static int CmdLE();
static int CmdGE();
static int CmdAssignScriptVar();
static int CmdAssignMapVar();
static int CmdAssignWorldVar();
static int CmdPushScriptVar();
static int CmdPushMapVar();
static int CmdPushWorldVar();
static int CmdAddScriptVar();
static int CmdAddMapVar();
static int CmdAddWorldVar();
static int CmdSubScriptVar();
static int CmdSubMapVar();
static int CmdSubWorldVar();
static int CmdMulScriptVar();
static int CmdMulMapVar();
static int CmdMulWorldVar();
static int CmdDivScriptVar();
static int CmdDivMapVar();
static int CmdDivWorldVar();
static int CmdModScriptVar();
static int CmdModMapVar();
static int CmdModWorldVar();
static int CmdIncScriptVar();
static int CmdIncMapVar();
static int CmdIncWorldVar();
static int CmdDecScriptVar();
static int CmdDecMapVar();
static int CmdDecWorldVar();
static int CmdGoto();
static int CmdIfGoto();
static int CmdDrop();
static int CmdDelay();
static int CmdDelayDirect();
static int CmdRandom();
static int CmdRandomDirect();
static int CmdThingCount();
static int CmdThingCountDirect();
static int CmdTagWait();
static int CmdTagWaitDirect();
static int CmdPolyWait();
static int CmdPolyWaitDirect();
static int CmdChangeFloor();
static int CmdChangeFloorDirect();
static int CmdChangeCeiling();
static int CmdChangeCeilingDirect();
static int CmdRestart();
static int CmdAndLogical();
static int CmdOrLogical();
static int CmdAndBitwise();
static int CmdOrBitwise();
static int CmdEorBitwise();
static int CmdNegateLogical();
static int CmdLShift();
static int CmdRShift();
static int CmdUnaryMinus();
static int CmdIfNotGoto();
static int CmdLineSide();
static int CmdScriptWait();
static int CmdScriptWaitDirect();
static int CmdClearLineSpecial();
static int CmdCaseGoto();
static int CmdBeginPrint();
static int CmdEndPrint();
static int CmdPrintString();
static int CmdPrintNumber();
static int CmdPrintCharacter();
static int CmdPlayerCount();
static int CmdGameType();
static int CmdGameSkill();
static int CmdTimer();
static int CmdSectorSound();
static int CmdAmbientSound();
static int CmdSoundSequence();
static int CmdSetLineTexture();
static int CmdSetLineBlocking();
static int CmdSetLineSpecial();
static int CmdThingSound();
static int CmdEndPrintBold();

static void ThingCount(int type, int tid);

int ACScriptCount;
const byte* ActionCodeBase;
static int ActionCodeSize;
acsInfo_t* ACSInfo;
int MapVars[MAX_ACS_MAP_VARS];
int WorldVars[MAX_ACS_WORLD_VARS];
acsstore_t ACSStore[MAX_ACS_STORE + 1]; // +1 for termination marker

static char EvalContext[64];
static acs_t* ACScript;
static unsigned int PCodeOffset;
static int SpecArgs[8];
static int ACStringCount;
static const char** ACStrings;
static char PrintBuffer[PRINT_BUFFER_SIZE];
static acs_t* NewScript;

static int (*PCodeCmds[])() =
{
	CmdNOP,
	CmdTerminate,
	CmdSuspend,
	CmdPushNumber,
	CmdLSpec1,
	CmdLSpec2,
	CmdLSpec3,
	CmdLSpec4,
	CmdLSpec5,
	CmdLSpec1Direct,
	CmdLSpec2Direct,
	CmdLSpec3Direct,
	CmdLSpec4Direct,
	CmdLSpec5Direct,
	CmdAdd,
	CmdSubtract,
	CmdMultiply,
	CmdDivide,
	CmdModulus,
	CmdEQ,
	CmdNE,
	CmdLT,
	CmdGT,
	CmdLE,
	CmdGE,
	CmdAssignScriptVar,
	CmdAssignMapVar,
	CmdAssignWorldVar,
	CmdPushScriptVar,
	CmdPushMapVar,
	CmdPushWorldVar,
	CmdAddScriptVar,
	CmdAddMapVar,
	CmdAddWorldVar,
	CmdSubScriptVar,
	CmdSubMapVar,
	CmdSubWorldVar,
	CmdMulScriptVar,
	CmdMulMapVar,
	CmdMulWorldVar,
	CmdDivScriptVar,
	CmdDivMapVar,
	CmdDivWorldVar,
	CmdModScriptVar,
	CmdModMapVar,
	CmdModWorldVar,
	CmdIncScriptVar,
	CmdIncMapVar,
	CmdIncWorldVar,
	CmdDecScriptVar,
	CmdDecMapVar,
	CmdDecWorldVar,
	CmdGoto,
	CmdIfGoto,
	CmdDrop,
	CmdDelay,
	CmdDelayDirect,
	CmdRandom,
	CmdRandomDirect,
	CmdThingCount,
	CmdThingCountDirect,
	CmdTagWait,
	CmdTagWaitDirect,
	CmdPolyWait,
	CmdPolyWaitDirect,
	CmdChangeFloor,
	CmdChangeFloorDirect,
	CmdChangeCeiling,
	CmdChangeCeilingDirect,
	CmdRestart,
	CmdAndLogical,
	CmdOrLogical,
	CmdAndBitwise,
	CmdOrBitwise,
	CmdEorBitwise,
	CmdNegateLogical,
	CmdLShift,
	CmdRShift,
	CmdUnaryMinus,
	CmdIfNotGoto,
	CmdLineSide,
	CmdScriptWait,
	CmdScriptWaitDirect,
	CmdClearLineSpecial,
	CmdCaseGoto,
	CmdBeginPrint,
	CmdEndPrint,
	CmdPrintString,
	CmdPrintNumber,
	CmdPrintCharacter,
	CmdPlayerCount,
	CmdGameType,
	CmdGameSkill,
	CmdTimer,
	CmdSectorSound,
	CmdAmbientSound,
	CmdSoundSequence,
	CmdSetLineTexture,
	CmdSetLineBlocking,
	CmdSetLineSpecial,
	CmdThingSound,
	CmdEndPrintBold,
};

static void ACSAssert(int condition, const char* fmt, ...)
{
	char buf[128];
	va_list args;

	if(condition)
	{
		return;
	}

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	I_Error("ACS assertion failure: in %s: %s", EvalContext, buf);
}

static int ReadCodeInt()
{
	int result;
	const int* ptr;

	ACSAssert(PCodeOffset + 3 < ActionCodeSize,
		"unexpectedly reached end of ACS lump");

	ptr = (const int*)(ActionCodeBase + PCodeOffset);
	result = LittleLong(*ptr);
	PCodeOffset += 4;

	return result;
}

static int ReadScriptVar()
{
	int var = ReadCodeInt();
	ACSAssert(var >= 0, "negative script variable: %d < 0", var);
	ACSAssert(var < MAX_ACS_SCRIPT_VARS,
		"invalid script variable: %d >= %d", var, MAX_ACS_SCRIPT_VARS);
	return var;
}

static int ReadMapVar()
{
	int var = ReadCodeInt();
	ACSAssert(var >= 0, "negative map variable: %d < 0", var);
	ACSAssert(var < MAX_ACS_MAP_VARS,
		"invalid map variable: %d >= %d", var, MAX_ACS_MAP_VARS);
	return var;
}

static int ReadWorldVar()
{
	int var = ReadCodeInt();
	ACSAssert(var >= 0, "negative world variable: %d < 0", var);
	ACSAssert(var < MAX_ACS_WORLD_VARS,
		"invalid world variable: %d >= %d", var, MAX_ACS_WORLD_VARS);
	return var;
}

static const char* StringLookup(int string_index)
{
	ACSAssert(string_index >= 0,
		"negative string index: %d < 0", string_index);
	ACSAssert(string_index < ACStringCount,
		"invalid string index: %d >= %d", string_index, ACStringCount);
	return ACStrings[string_index];
}

static int ReadOffset()
{
	int offset = ReadCodeInt();
	ACSAssert(offset >= 0, "negative lump offset %d", offset);
	ACSAssert(offset < ActionCodeSize, "invalid lump offset: %d >= %d",
		offset, ActionCodeSize);
	return offset;
}

void P_LoadACScripts(int lump)
{
	int i, offset;
	const acsHeader_t* header;
	acsInfo_t* info;

	ActionCodeBase = static_cast<decltype(ActionCodeBase)>(W_LumpByNum(lump));
	ActionCodeSize = W_LumpLength(lump);

	snprintf(EvalContext, sizeof(EvalContext), "header parsing of lump #%d", lump);

	header = (const acsHeader_t*)ActionCodeBase;
	PCodeOffset = LittleLong(header->infoOffset);

	ACScriptCount = ReadCodeInt();

	if(ACScriptCount == 0)
	{
		// Empty behavior lump
		return;
	}

	ACSInfo = static_cast<acsInfo_t*>(Z_MallocLevel(ACScriptCount * sizeof(acsInfo_t)));
	memset(ACSInfo, 0, ACScriptCount * sizeof(acsInfo_t));
	for(i = 0, info = ACSInfo; i < ACScriptCount; i++, info++)
	{
		info->number = ReadCodeInt();
		info->offset = ReadOffset();
		info->argCount = ReadCodeInt();

		if(info->argCount > MAX_SCRIPT_ARGS)
		{
			fprintf(stderr, "Warning: ACS script #%i has %i arguments, more "
				"than the maximum of %i. Enforcing limit.\n"
				"If you are seeing this message, please report "
				"the name of the WAD where you saw it.\n",
				i, info->argCount, MAX_SCRIPT_ARGS);
			info->argCount = MAX_SCRIPT_ARGS;
		}

		if(info->number >= OPEN_SCRIPTS_BASE)
		{
			// Auto-activate
			info->number -= OPEN_SCRIPTS_BASE;
			StartOpenACS(info->number, i, info->offset);
			info->state = AcsState::Running;
		}
		else
		{
			info->state = AcsState::Inactive;
		}
	}

	ACStringCount = ReadCodeInt();
	ACSAssert(ACStringCount >= 0, "negative string count %d", ACStringCount);
	ACStrings = static_cast<const char**>(Z_MallocLevel(ACStringCount * sizeof(char*)));

	for(i = 0; i < ACStringCount; ++i)
	{
		offset = ReadOffset();
		ACStrings[i] = (const char*)ActionCodeBase + offset;
		ACSAssert(memchr(ACStrings[i], '\0', ActionCodeSize - offset) != nullptr,
			"string %d missing terminating NUL", i);
	}

	memset(MapVars, 0, sizeof(MapVars));
}

static void StartOpenACS(int number, int infoIndex, int offset)
{
	acs_t* script;

	script = static_cast<acs_t*>(Z_MallocLevel(sizeof(acs_t)));
	memset(script, 0, sizeof(acs_t));
	script->number = number;

	// World objects are allotted 1 second for initialization
	script->delayCount = TICRATE;

	script->infoIndex = infoIndex;
	script->ip = offset;
	script->thinker.function = reinterpret_cast<think_t>(T_InterpretACS);
	P_AddThinker(&script->thinker);
}

void P_CheckACSStore()
{
	acsstore_t* store;

	for(store = ACSStore; store->map != 0; store++)
	{
		if(store->map == gamemap)
		{
			P_StartACS(store->script, 0, store->args, nullptr, nullptr, 0);
			if(NewScript)
			{
				NewScript->delayCount = TICRATE;
			}
			store->map = -1;
		}
	}
}

static char ErrorMsg[128];

dboolean P_StartACS(int number, int map, byte* args, mobj_t* activator,
	line_t* line, int side)
{
	int i;
	acs_t* script;
	int infoIndex;
	AcsState* statePtr;

	NewScript = nullptr;
	if(map && map != gamemap)
	{
		// Add to the script store
		return AddToACSStore(map, number, args);
	}
	infoIndex = GetACSIndex(number);
	if(infoIndex == -1)
	{
		// Script not found
		//I_Error("P_StartACS: Unknown script number %d", number);
		snprintf(ErrorMsg, sizeof(ErrorMsg), "P_STARTACS ERROR: UNKNOWN SCRIPT %d", number);
		P_SetMessage(&players[consoleplayer], ErrorMsg, true);
	}
	statePtr = &ACSInfo[infoIndex].state;
	if(*statePtr == AcsState::Suspended)
	{
		// Resume a suspended script
		*statePtr = AcsState::Running;
		return true;
	}
	if(*statePtr != AcsState::Inactive)
	{
		// Script is already executing
		return false;
	}
	script = static_cast<acs_t*>(Z_MallocLevel(sizeof(acs_t)));
	memset(script, 0, sizeof(acs_t));
	script->number = number;
	script->infoIndex = infoIndex;
	P_SetTarget(&script->activator, activator);
	script->line = line;
	script->side = side;
	script->ip = ACSInfo[infoIndex].offset;
	script->thinker.function = reinterpret_cast<think_t>(T_InterpretACS);
	for(i = 0; i < MAX_SCRIPT_ARGS && i < ACSInfo[infoIndex].argCount; i++)
	{
		script->vars[i] = args[i];
	}
	*statePtr = AcsState::Running;
	P_AddThinker(&script->thinker);
	NewScript = script;
	return true;
}

static dboolean AddToACSStore(int map, int number, byte* args)
{
	int i;
	int index;

	index = -1;
	for(i = 0; ACSStore[i].map != 0; i++)
	{
		if(ACSStore[i].script == number && ACSStore[i].map == map)
		{
			// Don't allow duplicates
			return false;
		}
		if(index == -1 && ACSStore[i].map == -1)
		{
			// Remember first empty slot
			index = i;
		}
	}
	if(index == -1)
	{
		// Append required
		if(i == MAX_ACS_STORE)
		{
			I_Error("AddToACSStore: MAX_ACS_STORE (%d) exceeded.",
				MAX_ACS_STORE);
		}
		index = i;
		ACSStore[index + 1].map = 0;
	}
	ACSStore[index].map = map;
	ACSStore[index].script = number;
	memcpy(ACSStore[index].args, args, MAX_SCRIPT_ARGS);
	return true;
}

dboolean P_StartLockedACS(line_t* line, byte* args, mobj_t* mo, int side)
{
	int i;
	int lock;
	byte newArgs[5];

	extern char* TextKeyMessages[11];
	extern char LockedBuffer[80];

	lock = args[4];
	if(!mo->player)
	{
		return false;
	}
	if(lock)
	{
		if(!mo->player->cards[lock - 1])
		{
			snprintf(LockedBuffer, sizeof(LockedBuffer),
				"YOU NEED THE %s\n", TextKeyMessages[lock - 1]);
			P_SetMessage(mo->player, LockedBuffer, true);
			S_StartMobjSound(mo, SfxId::HexenDoorLocked);
			return false;
		}
	}
	for(i = 0; i < 4; i++)
	{
		newArgs[i] = args[i];
	}
	newArgs[4] = 0;
	return P_StartACS(newArgs[0], newArgs[1], &newArgs[2], mo, line, side);
}

dboolean P_TerminateACS(int number, int map)
{
	int infoIndex;

	infoIndex = GetACSIndex(number);
	if(infoIndex == -1)
	{
		// Script not found
		return false;
	}
	if(ACSInfo[infoIndex].state == AcsState::Inactive
		|| ACSInfo[infoIndex].state == AcsState::Terminating)
	{
		// States that disallow termination
		return false;
	}
	ACSInfo[infoIndex].state = AcsState::Terminating;
	return true;
}

dboolean P_SuspendACS(int number, int map)
{
	int infoIndex;

	infoIndex = GetACSIndex(number);
	if(infoIndex == -1)
	{
		// Script not found
		return false;
	}
	if(ACSInfo[infoIndex].state == AcsState::Inactive
		|| ACSInfo[infoIndex].state == AcsState::Suspended
		|| ACSInfo[infoIndex].state == AcsState::Terminating)
	{
		// States that disallow suspension
		return false;
	}
	ACSInfo[infoIndex].state = AcsState::Suspended;
	return true;
}

void P_ACSInitNewGame()
{
	memset(WorldVars, 0, sizeof(WorldVars));
	memset(ACSStore, 0, sizeof(ACSStore));
}

void T_InterpretACS(acs_t* script)
{
	int cmd;
	int action;

	if(ACSInfo[script->infoIndex].state == AcsState::Terminating)
	{
		ACSInfo[script->infoIndex].state = AcsState::Inactive;
		ScriptFinished(ACScript->number);
		P_RemoveThinker(&ACScript->thinker);
		return;
	}
	if(ACSInfo[script->infoIndex].state != AcsState::Running)
	{
		return;
	}
	if(script->delayCount)
	{
		script->delayCount--;
		return;
	}
	ACScript = script;
	PCodeOffset = ACScript->ip;

	do
	{
		snprintf(EvalContext, sizeof(EvalContext), "script %d @0x%x",
			ACSInfo[script->infoIndex].number, PCodeOffset);
		cmd = ReadCodeInt();
		snprintf(EvalContext, sizeof(EvalContext), "script %d @0x%x, cmd=%d",
			ACSInfo[script->infoIndex].number, PCodeOffset, cmd);
		ACSAssert(cmd >= 0, "negative ACS instruction %d", cmd);
		ACSAssert(cmd < arrlen(PCodeCmds),
			"invalid ACS instruction %d (maybe this WAD is designed "
			"for an advanced source port and is not vanilla "
			"compatible)", cmd);
		action = PCodeCmds[cmd]();
	}
	while(action == SCRIPT_CONTINUE);

	ACScript->ip = PCodeOffset;

	if(action == SCRIPT_TERMINATE)
	{
		ACSInfo[script->infoIndex].state = AcsState::Inactive;
		ScriptFinished(ACScript->number);
		P_RemoveThinker(&ACScript->thinker);
	}
}

void P_TagFinished(int tag)
{
	int i;

	if(!map_format.acs) return;

	if(TagBusy(tag) == true)
	{
		return;
	}
	for(i = 0; i < ACScriptCount; i++)
	{
		if(ACSInfo[i].state == AcsState::WaitingForTag
			&& ACSInfo[i].waitValue == tag)
		{
			ACSInfo[i].state = AcsState::Running;
		}
	}
}

void P_PolyobjFinished(int po)
{
	int i;

	if(PO_Busy(po) == true)
	{
		return;
	}
	for(i = 0; i < ACScriptCount; i++)
	{
		if(ACSInfo[i].state == AcsState::WaitingForPoly
			&& ACSInfo[i].waitValue == po)
		{
			ACSInfo[i].state = AcsState::Running;
		}
	}
}

static void ScriptFinished(int number)
{
	int i;

	for(i = 0; i < ACScriptCount; i++)
	{
		if(ACSInfo[i].state == AcsState::WaitingForScript
			&& ACSInfo[i].waitValue == number)
		{
			ACSInfo[i].state = AcsState::Running;
		}
	}
}

static dboolean TagBusy(int tag)
{
	const int* id_p;

	FIND_SECTORS(id_p, tag)
	{
		if(sectors[*id_p].floordata || sectors[*id_p].ceilingdata)
		{
			return true;
		}
	}
	return false;
}

static int GetACSIndex(int number)
{
	int i;

	for(i = 0; i < ACScriptCount; i++)
	{
		if(ACSInfo[i].number == number)
		{
			return i;
		}
	}
	return -1;
}

void CheckACSPresent(int number)
{
	if(GetACSIndex(number) == -1)
	{
		I_Error("Required ACS script %d not initialized", number);
	}
}

static void Push(int value)
{
	ACSAssert(ACScript->stackPtr < ACS_STACK_DEPTH,
		"maximum stack depth exceeded: %d >= %d",
		ACScript->stackPtr, ACS_STACK_DEPTH);
	ACScript->stack[ACScript->stackPtr++] = value;
}

static int Pop()
{
	ACSAssert(ACScript->stackPtr > 0, "pop of empty stack");
	return ACScript->stack[--ACScript->stackPtr];
}

static int Top()
{
	ACSAssert(ACScript->stackPtr > 0, "read from top of empty stack");
	return ACScript->stack[ACScript->stackPtr - 1];
}

static void Drop()
{
	ACSAssert(ACScript->stackPtr > 0, "drop on empty stack");
	ACScript->stackPtr--;
}

static int CmdNOP()
{
	return SCRIPT_CONTINUE;
}

static int CmdTerminate()
{
	return SCRIPT_TERMINATE;
}

static int CmdSuspend()
{
	ACSInfo[ACScript->infoIndex].state = AcsState::Suspended;
	return SCRIPT_STOP;
}

static int CmdPushNumber()
{
	Push(ReadCodeInt());
	return SCRIPT_CONTINUE;
}

static int CmdLSpec1()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec2()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec3()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec4()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[3] = Pop();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec5()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[4] = Pop();
	SpecArgs[3] = Pop();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec1Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec2Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec3Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	SpecArgs[2] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec4Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	SpecArgs[2] = ReadCodeInt();
	SpecArgs[3] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdLSpec5Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	SpecArgs[2] = ReadCodeInt();
	SpecArgs[3] = ReadCodeInt();
	SpecArgs[4] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return SCRIPT_CONTINUE;
}

static int CmdAdd()
{
	Push(Pop() + Pop());
	return SCRIPT_CONTINUE;
}

static int CmdSubtract()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() - operand2);
	return SCRIPT_CONTINUE;
}

static int CmdMultiply()
{
	Push(Pop() * Pop());
	return SCRIPT_CONTINUE;
}

static int CmdDivide()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() / operand2);
	return SCRIPT_CONTINUE;
}

static int CmdModulus()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() % operand2);
	return SCRIPT_CONTINUE;
}

static int CmdEQ()
{
	Push(Pop() == Pop());
	return SCRIPT_CONTINUE;
}

static int CmdNE()
{
	Push(Pop() != Pop());
	return SCRIPT_CONTINUE;
}

static int CmdLT()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() < operand2);
	return SCRIPT_CONTINUE;
}

static int CmdGT()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() > operand2);
	return SCRIPT_CONTINUE;
}

static int CmdLE()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() <= operand2);
	return SCRIPT_CONTINUE;
}

static int CmdGE()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() >= operand2);
	return SCRIPT_CONTINUE;
}

static int CmdAssignScriptVar()
{
	ACScript->vars[ReadScriptVar()] = Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAssignMapVar()
{
	MapVars[ReadMapVar()] = Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAssignWorldVar()
{
	WorldVars[ReadWorldVar()] = Pop();
	return SCRIPT_CONTINUE;
}

static int CmdPushScriptVar()
{
	Push(ACScript->vars[ReadScriptVar()]);
	return SCRIPT_CONTINUE;
}

static int CmdPushMapVar()
{
	Push(MapVars[ReadMapVar()]);
	return SCRIPT_CONTINUE;
}

static int CmdPushWorldVar()
{
	Push(WorldVars[ReadWorldVar()]);
	return SCRIPT_CONTINUE;
}

static int CmdAddScriptVar()
{
	ACScript->vars[ReadScriptVar()] += Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAddMapVar()
{
	MapVars[ReadMapVar()] += Pop();
	return SCRIPT_CONTINUE;
}

static int CmdAddWorldVar()
{
	WorldVars[ReadWorldVar()] += Pop();
	return SCRIPT_CONTINUE;
}

static int CmdSubScriptVar()
{
	ACScript->vars[ReadScriptVar()] -= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdSubMapVar()
{
	MapVars[ReadMapVar()] -= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdSubWorldVar()
{
	WorldVars[ReadWorldVar()] -= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdMulScriptVar()
{
	ACScript->vars[ReadScriptVar()] *= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdMulMapVar()
{
	MapVars[ReadMapVar()] *= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdMulWorldVar()
{
	WorldVars[ReadWorldVar()] *= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdDivScriptVar()
{
	ACScript->vars[ReadScriptVar()] /= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdDivMapVar()
{
	MapVars[ReadMapVar()] /= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdDivWorldVar()
{
	WorldVars[ReadWorldVar()] /= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdModScriptVar()
{
	ACScript->vars[ReadScriptVar()] %= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdModMapVar()
{
	MapVars[ReadMapVar()] %= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdModWorldVar()
{
	WorldVars[ReadWorldVar()] %= Pop();
	return SCRIPT_CONTINUE;
}

static int CmdIncScriptVar()
{
	++ACScript->vars[ReadScriptVar()];
	return SCRIPT_CONTINUE;
}

static int CmdIncMapVar()
{
	++MapVars[ReadMapVar()];
	return SCRIPT_CONTINUE;
}

static int CmdIncWorldVar()
{
	++WorldVars[ReadWorldVar()];
	return SCRIPT_CONTINUE;
}

static int CmdDecScriptVar()
{
	--ACScript->vars[ReadScriptVar()];
	return SCRIPT_CONTINUE;
}

static int CmdDecMapVar()
{
	--MapVars[ReadMapVar()];
	return SCRIPT_CONTINUE;
}

static int CmdDecWorldVar()
{
	--WorldVars[ReadWorldVar()];
	return SCRIPT_CONTINUE;
}

static int CmdGoto()
{
	PCodeOffset = ReadOffset();
	return SCRIPT_CONTINUE;
}

static int CmdIfGoto()
{
	int offset;

	offset = ReadOffset();

	if(Pop() != 0)
	{
		PCodeOffset = offset;
	}
	return SCRIPT_CONTINUE;
}

static int CmdDrop()
{
	Drop();
	return SCRIPT_CONTINUE;
}

static int CmdDelay()
{
	ACScript->delayCount = Pop();
	return SCRIPT_STOP;
}

static int CmdDelayDirect()
{
	ACScript->delayCount = ReadCodeInt();
	return SCRIPT_STOP;
}

static int CmdRandom()
{
	int low;
	int high;

	high = Pop();
	low = Pop();
	Push(low + (P_Random(RandomClass::Hexen) % (high - low + 1)));
	return SCRIPT_CONTINUE;
}

static int CmdRandomDirect()
{
	int low;
	int high;

	low = ReadCodeInt();
	high = ReadCodeInt();
	Push(low + (P_Random(RandomClass::Hexen) % (high - low + 1)));
	return SCRIPT_CONTINUE;
}

static int CmdThingCount()
{
	int tid;

	tid = Pop();
	ThingCount(Pop(), tid);
	return SCRIPT_CONTINUE;
}

static int CmdThingCountDirect()
{
	int type;

	type = ReadCodeInt();
	ThingCount(type, ReadCodeInt());
	return SCRIPT_CONTINUE;
}

static void ThingCount(int type, int tid)
{
	int count;
	int searcher;
	mobj_t* mobj;
	MobjType moType;
	thinker_t* think;

	if(!(type + tid))
	{
		// Nothing to count
		return;
	}
	moType = TranslateThingType[type];
	count = 0;
	searcher = -1;
	if(tid)
	{
		// Count TID things
		while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
		{
			if(type == 0)
			{
				// Just count TIDs
				count++;
			}
			else if(moType == mobj->type)
			{
				if((mobj->flags & MobjFlag::CountKill) != MobjFlag{} && mobj->health <= 0)
				{
					// Don't count dead monsters
					continue;
				}
				count++;
			}
		}
	}
	else
	{
		// Count only types
		for(think = thinkercap.next; think != &thinkercap;
			think = think->next)
		{
			if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
			{
				// Not a mobj thinker
				continue;
			}
			mobj = (mobj_t*)think;
			if(mobj->type != moType)
			{
				// Doesn't match
				continue;
			}
			if((mobj->flags & MobjFlag::CountKill) != MobjFlag{} && mobj->health <= 0)
			{
				// Don't count dead monsters
				continue;
			}
			count++;
		}
	}
	Push(count);
}

static int CmdTagWait()
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForTag;
	return SCRIPT_STOP;
}

static int CmdTagWaitDirect()
{
	ACSInfo[ACScript->infoIndex].waitValue = ReadCodeInt();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForTag;
	return SCRIPT_STOP;
}

static int CmdPolyWait()
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForPoly;
	return SCRIPT_STOP;
}

static int CmdPolyWaitDirect()
{
	ACSInfo[ACScript->infoIndex].waitValue = ReadCodeInt();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForPoly;
	return SCRIPT_STOP;
}

static int CmdChangeFloor()
{
	int tag;
	int flat;
	const int* id_p;

	flat = R_FlatNumForName(StringLookup(Pop()));
	tag = Pop();
	FIND_SECTORS(id_p, tag)
	{
		sectors[*id_p].floorpic = flat;
	}
	return SCRIPT_CONTINUE;
}

static int CmdChangeFloorDirect()
{
	int tag;
	int flat;
	const int* id_p;

	tag = ReadCodeInt();
	flat = R_FlatNumForName(StringLookup(ReadCodeInt()));
	FIND_SECTORS(id_p, tag)
	{
		sectors[*id_p].floorpic = flat;
	}
	return SCRIPT_CONTINUE;
}

static int CmdChangeCeiling()
{
	int tag;
	int flat;
	const int* id_p;

	flat = R_FlatNumForName(StringLookup(Pop()));
	tag = Pop();
	FIND_SECTORS(id_p, tag)
	{
		sectors[*id_p].ceilingpic = flat;
	}
	return SCRIPT_CONTINUE;
}

static int CmdChangeCeilingDirect()
{
	int tag;
	int flat;
	const int* id_p;

	tag = ReadCodeInt();
	flat = R_FlatNumForName(StringLookup(ReadCodeInt()));
	FIND_SECTORS(id_p, tag)
	{
		sectors[*id_p].ceilingpic = flat;
	}
	return SCRIPT_CONTINUE;
}

static int CmdRestart()
{
	PCodeOffset = ACSInfo[ACScript->infoIndex].offset;
	return SCRIPT_CONTINUE;
}

static int CmdAndLogical()
{
	Push(Pop() && Pop());
	return SCRIPT_CONTINUE;
}

static int CmdOrLogical()
{
	Push(Pop() || Pop());
	return SCRIPT_CONTINUE;
}

static int CmdAndBitwise()
{
	Push(Pop() & Pop());
	return SCRIPT_CONTINUE;
}

static int CmdOrBitwise()
{
	Push(Pop() | Pop());
	return SCRIPT_CONTINUE;
}

static int CmdEorBitwise()
{
	Push(Pop() ^ Pop());
	return SCRIPT_CONTINUE;
}

static int CmdNegateLogical()
{
	Push(!Pop());
	return SCRIPT_CONTINUE;
}

static int CmdLShift()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() << operand2);
	return SCRIPT_CONTINUE;
}

static int CmdRShift()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() >> operand2);
	return SCRIPT_CONTINUE;
}

static int CmdUnaryMinus()
{
	Push(-Pop());
	return SCRIPT_CONTINUE;
}

static int CmdIfNotGoto()
{
	int offset;

	offset = ReadOffset();

	if(Pop() == 0)
	{
		PCodeOffset = offset;
	}
	return SCRIPT_CONTINUE;
}

static int CmdLineSide()
{
	Push(ACScript->side);
	return SCRIPT_CONTINUE;
}

static int CmdScriptWait()
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForScript;
	return SCRIPT_STOP;
}

static int CmdScriptWaitDirect()
{
	ACSInfo[ACScript->infoIndex].waitValue = ReadCodeInt();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForScript;
	return SCRIPT_STOP;
}

static int CmdClearLineSpecial()
{
	if(ACScript->line)
	{
		ACScript->line->special = 0;
	}
	return SCRIPT_CONTINUE;
}

static int CmdCaseGoto()
{
	int value;
	int offset;

	value = ReadCodeInt();
	offset = ReadOffset();

	if(Top() == value)
	{
		PCodeOffset = offset;
		Drop();
	}

	return SCRIPT_CONTINUE;
}

static int CmdBeginPrint()
{
	*PrintBuffer = 0;
	return SCRIPT_CONTINUE;
}

static int CmdEndPrint()
{
	player_t* player;

	if(ACScript->activator && ACScript->activator->player)
	{
		player = ACScript->activator->player;
	}
	else
	{
		player = &players[consoleplayer];
	}
	P_SetMessage(player, PrintBuffer, true);
	return SCRIPT_CONTINUE;
}

static int CmdEndPrintBold()
{
	int i;

	for(i = 0; i < g_maxplayers; i++)
	{
		if(playeringame[i])
		{
			P_SetYellowMessage(&players[i], PrintBuffer, true);
		}
	}
	return SCRIPT_CONTINUE;
}

static int CmdPrintString()
{
	M_StringConcat(PrintBuffer, StringLookup(Pop()), sizeof(PrintBuffer));
	return SCRIPT_CONTINUE;
}

static int CmdPrintNumber()
{
	char tempStr[16];

	snprintf(tempStr, sizeof(tempStr), "%d", Pop());
	M_StringConcat(PrintBuffer, tempStr, sizeof(PrintBuffer));
	return SCRIPT_CONTINUE;
}

static int CmdPrintCharacter()
{
	char tempStr[2];

	tempStr[0] = Pop();
	tempStr[1] = '\0';
	M_StringConcat(PrintBuffer, tempStr, sizeof(PrintBuffer));

	return SCRIPT_CONTINUE;
}

static int CmdPlayerCount()
{
	int i;
	int count;

	count = 0;
	for(i = 0; i < g_maxplayers; i++)
	{
		count += playeringame[i];
	}
	Push(count);
	return SCRIPT_CONTINUE;
}

static int CmdGameType()
{
	int gametype;

	if(netgame == false)
	{
		gametype = GAME_SINGLE_PLAYER;
	}
	else if(deathmatch)
	{
		gametype = GAME_NET_DEATHMATCH;
	}
	else
	{
		gametype = GAME_NET_COOPERATIVE;
	}
	Push(gametype);
	return SCRIPT_CONTINUE;
}

static int CmdGameSkill()
{
	Push(gameskill);
	return SCRIPT_CONTINUE;
}

static int CmdTimer()
{
	Push(leveltime);
	return SCRIPT_CONTINUE;
}

static int CmdSectorSound()
{
	int volume;
	mobj_t* mobj;

	mobj = nullptr;
	if(ACScript->line)
	{
		mobj = (mobj_t*)&ACScript->line->frontsector->soundorg;
	}
	volume = Pop();
	S_StartSoundAtVolume(mobj, S_GetSoundID(StringLookup(Pop())), volume, false, 0);
	return SCRIPT_CONTINUE;
}

static int CmdThingSound()
{
	int tid;
	SfxId sound;
	int volume;
	mobj_t* mobj;
	int searcher;

	volume = Pop();
	sound = S_GetSoundID(StringLookup(Pop()));
	tid = Pop();
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != nullptr)
	{
		S_StartSoundAtVolume(mobj, sound, volume, false, 0);
	}
	return SCRIPT_CONTINUE;
}

static int CmdAmbientSound()
{
	int volume;

	volume = Pop();
	S_StartSoundAtVolume(nullptr, S_GetSoundID(StringLookup(Pop())), volume, false, 0);
	return SCRIPT_CONTINUE;
}

static int CmdSoundSequence()
{
	mobj_t* mobj;

	mobj = nullptr;
	if(ACScript->line)
	{
		mobj = (mobj_t*)&ACScript->line->frontsector->soundorg;
	}
	SN_StartSequenceName(mobj, StringLookup(Pop()));
	return SCRIPT_CONTINUE;
}

static int CmdSetLineTexture()
{
	line_t* line;
	int lineTag;
	int side;
	int position;
	int texture;
	int searcher;

	texture = R_TextureNumForName(StringLookup(Pop()));
	position = Pop();
	side = Pop();
	lineTag = Pop();
	searcher = -1;
	while((line = P_FindLine(lineTag, &searcher)) != nullptr)
	{
		if(position == TEXTURE_MIDDLE)
		{
			sides[line->sidenum[side]].midtexture = texture;
		}
		else if(position == TEXTURE_BOTTOM)
		{
			sides[line->sidenum[side]].bottomtexture = texture;
		}
		else
		{
			// TEXTURE_TOP
			sides[line->sidenum[side]].toptexture = texture;
		}
	}
	return SCRIPT_CONTINUE;
}

static int CmdSetLineBlocking()
{
	line_t* line;
	int lineTag;
	dboolean blocking;
	int searcher;

	blocking = Pop() ? ML_BLOCKING : 0;
	lineTag = Pop();
	searcher = -1;
	while((line = P_FindLine(lineTag, &searcher)) != nullptr)
	{
		line->flags = (line->flags & ~ML_BLOCKING) | blocking;
	}
	return SCRIPT_CONTINUE;
}

static int CmdSetLineSpecial()
{
	line_t* line;
	int lineTag;
	int special, arg1, arg2, arg3, arg4, arg5;
	int searcher;

	arg5 = Pop();
	arg4 = Pop();
	arg3 = Pop();
	arg2 = Pop();
	arg1 = Pop();
	special = Pop();
	lineTag = Pop();
	searcher = -1;
	while((line = P_FindLine(lineTag, &searcher)) != nullptr)
	{
		line->special = special;
		line->special_args[0] = arg1;
		line->special_args[1] = arg2;
		line->special_args[2] = arg3;
		line->special_args[3] = arg4;
		line->special_args[4] = arg5;
	}
	return SCRIPT_CONTINUE;
}
