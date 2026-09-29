// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

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
#define OPEN_SCRIPTS_BASE 1000
#define PRINT_BUFFER_SIZE 256

// What the interpreter does after a p-code command.
enum struct AcsScriptAction : uint8_t
{
	Continue,
	Stop,
	Terminate,
};

// The game type ACS's GameType() returns to the script.
enum struct AcsGameType : int32_t
{
	SinglePlayer = 0,
	NetCooperative = 1,
	NetDeathmatch = 2,
};

// Which texture of a side ACS's SetLineTexture() replaces.
enum struct AcsLineTexture : int32_t
{
	Top = 0,
	Middle = 1,
	Bottom = 2,
};

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

static AcsScriptAction CmdNOP();
static AcsScriptAction CmdTerminate();
static AcsScriptAction CmdSuspend();
static AcsScriptAction CmdPushNumber();
static AcsScriptAction CmdLSpec1();
static AcsScriptAction CmdLSpec2();
static AcsScriptAction CmdLSpec3();
static AcsScriptAction CmdLSpec4();
static AcsScriptAction CmdLSpec5();
static AcsScriptAction CmdLSpec1Direct();
static AcsScriptAction CmdLSpec2Direct();
static AcsScriptAction CmdLSpec3Direct();
static AcsScriptAction CmdLSpec4Direct();
static AcsScriptAction CmdLSpec5Direct();
static AcsScriptAction CmdAdd();
static AcsScriptAction CmdSubtract();
static AcsScriptAction CmdMultiply();
static AcsScriptAction CmdDivide();
static AcsScriptAction CmdModulus();
static AcsScriptAction CmdEQ();
static AcsScriptAction CmdNE();
static AcsScriptAction CmdLT();
static AcsScriptAction CmdGT();
static AcsScriptAction CmdLE();
static AcsScriptAction CmdGE();
static AcsScriptAction CmdAssignScriptVar();
static AcsScriptAction CmdAssignMapVar();
static AcsScriptAction CmdAssignWorldVar();
static AcsScriptAction CmdPushScriptVar();
static AcsScriptAction CmdPushMapVar();
static AcsScriptAction CmdPushWorldVar();
static AcsScriptAction CmdAddScriptVar();
static AcsScriptAction CmdAddMapVar();
static AcsScriptAction CmdAddWorldVar();
static AcsScriptAction CmdSubScriptVar();
static AcsScriptAction CmdSubMapVar();
static AcsScriptAction CmdSubWorldVar();
static AcsScriptAction CmdMulScriptVar();
static AcsScriptAction CmdMulMapVar();
static AcsScriptAction CmdMulWorldVar();
static AcsScriptAction CmdDivScriptVar();
static AcsScriptAction CmdDivMapVar();
static AcsScriptAction CmdDivWorldVar();
static AcsScriptAction CmdModScriptVar();
static AcsScriptAction CmdModMapVar();
static AcsScriptAction CmdModWorldVar();
static AcsScriptAction CmdIncScriptVar();
static AcsScriptAction CmdIncMapVar();
static AcsScriptAction CmdIncWorldVar();
static AcsScriptAction CmdDecScriptVar();
static AcsScriptAction CmdDecMapVar();
static AcsScriptAction CmdDecWorldVar();
static AcsScriptAction CmdGoto();
static AcsScriptAction CmdIfGoto();
static AcsScriptAction CmdDrop();
static AcsScriptAction CmdDelay();
static AcsScriptAction CmdDelayDirect();
static AcsScriptAction CmdRandom();
static AcsScriptAction CmdRandomDirect();
static AcsScriptAction CmdThingCount();
static AcsScriptAction CmdThingCountDirect();
static AcsScriptAction CmdTagWait();
static AcsScriptAction CmdTagWaitDirect();
static AcsScriptAction CmdPolyWait();
static AcsScriptAction CmdPolyWaitDirect();
static AcsScriptAction CmdChangeFloor();
static AcsScriptAction CmdChangeFloorDirect();
static AcsScriptAction CmdChangeCeiling();
static AcsScriptAction CmdChangeCeilingDirect();
static AcsScriptAction CmdRestart();
static AcsScriptAction CmdAndLogical();
static AcsScriptAction CmdOrLogical();
static AcsScriptAction CmdAndBitwise();
static AcsScriptAction CmdOrBitwise();
static AcsScriptAction CmdEorBitwise();
static AcsScriptAction CmdNegateLogical();
static AcsScriptAction CmdLShift();
static AcsScriptAction CmdRShift();
static AcsScriptAction CmdUnaryMinus();
static AcsScriptAction CmdIfNotGoto();
static AcsScriptAction CmdLineSide();
static AcsScriptAction CmdScriptWait();
static AcsScriptAction CmdScriptWaitDirect();
static AcsScriptAction CmdClearLineSpecial();
static AcsScriptAction CmdCaseGoto();
static AcsScriptAction CmdBeginPrint();
static AcsScriptAction CmdEndPrint();
static AcsScriptAction CmdPrintString();
static AcsScriptAction CmdPrintNumber();
static AcsScriptAction CmdPrintCharacter();
static AcsScriptAction CmdPlayerCount();
static AcsScriptAction CmdGameType();
static AcsScriptAction CmdGameSkill();
static AcsScriptAction CmdTimer();
static AcsScriptAction CmdSectorSound();
static AcsScriptAction CmdAmbientSound();
static AcsScriptAction CmdSoundSequence();
static AcsScriptAction CmdSetLineTexture();
static AcsScriptAction CmdSetLineBlocking();
static AcsScriptAction CmdSetLineSpecial();
static AcsScriptAction CmdThingSound();
static AcsScriptAction CmdEndPrintBold();

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

static AcsScriptAction (*PCodeCmds[])() =
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
	Log::Fatal("ACS assertion failure: in {}: {}", std::string_view(EvalContext), std::string_view(buf));
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
			Log::Fatal("AddToACSStore: MAX_ACS_STORE ({}) exceeded.",
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
	AcsScriptAction action;

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
	while(action == AcsScriptAction::Continue);

	ACScript->ip = PCodeOffset;

	if(action == AcsScriptAction::Terminate)
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
		Log::Fatal("Required ACS script {} not initialized", number);
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

static AcsScriptAction CmdNOP()
{
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdTerminate()
{
	return AcsScriptAction::Terminate;
}

static AcsScriptAction CmdSuspend()
{
	ACSInfo[ACScript->infoIndex].state = AcsState::Suspended;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdPushNumber()
{
	Push(ReadCodeInt());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec1()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec2()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec3()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec4()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[3] = Pop();
	SpecArgs[2] = Pop();
	SpecArgs[1] = Pop();
	SpecArgs[0] = Pop();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec5()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec1Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec2Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec3Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	SpecArgs[2] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec4Direct()
{
	int special;

	special = ReadCodeInt();
	SpecArgs[0] = ReadCodeInt();
	SpecArgs[1] = ReadCodeInt();
	SpecArgs[2] = ReadCodeInt();
	SpecArgs[3] = ReadCodeInt();
	map_format.execute_line_special(special, SpecArgs, ACScript->line,
		ACScript->side, ACScript->activator);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLSpec5Direct()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAdd()
{
	Push(Pop() + Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSubtract()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() - operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdMultiply()
{
	Push(Pop() * Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDivide()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() / operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdModulus()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() % operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdEQ()
{
	Push(Pop() == Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdNE()
{
	Push(Pop() != Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLT()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() < operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdGT()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() > operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLE()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() <= operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdGE()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() >= operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAssignScriptVar()
{
	ACScript->vars[ReadScriptVar()] = Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAssignMapVar()
{
	MapVars[ReadMapVar()] = Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAssignWorldVar()
{
	WorldVars[ReadWorldVar()] = Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPushScriptVar()
{
	Push(ACScript->vars[ReadScriptVar()]);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPushMapVar()
{
	Push(MapVars[ReadMapVar()]);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPushWorldVar()
{
	Push(WorldVars[ReadWorldVar()]);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAddScriptVar()
{
	ACScript->vars[ReadScriptVar()] += Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAddMapVar()
{
	MapVars[ReadMapVar()] += Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAddWorldVar()
{
	WorldVars[ReadWorldVar()] += Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSubScriptVar()
{
	ACScript->vars[ReadScriptVar()] -= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSubMapVar()
{
	MapVars[ReadMapVar()] -= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSubWorldVar()
{
	WorldVars[ReadWorldVar()] -= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdMulScriptVar()
{
	ACScript->vars[ReadScriptVar()] *= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdMulMapVar()
{
	MapVars[ReadMapVar()] *= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdMulWorldVar()
{
	WorldVars[ReadWorldVar()] *= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDivScriptVar()
{
	ACScript->vars[ReadScriptVar()] /= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDivMapVar()
{
	MapVars[ReadMapVar()] /= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDivWorldVar()
{
	WorldVars[ReadWorldVar()] /= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdModScriptVar()
{
	ACScript->vars[ReadScriptVar()] %= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdModMapVar()
{
	MapVars[ReadMapVar()] %= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdModWorldVar()
{
	WorldVars[ReadWorldVar()] %= Pop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdIncScriptVar()
{
	++ACScript->vars[ReadScriptVar()];
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdIncMapVar()
{
	++MapVars[ReadMapVar()];
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdIncWorldVar()
{
	++WorldVars[ReadWorldVar()];
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDecScriptVar()
{
	--ACScript->vars[ReadScriptVar()];
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDecMapVar()
{
	--MapVars[ReadMapVar()];
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDecWorldVar()
{
	--WorldVars[ReadWorldVar()];
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdGoto()
{
	PCodeOffset = ReadOffset();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdIfGoto()
{
	int offset;

	offset = ReadOffset();

	if(Pop() != 0)
	{
		PCodeOffset = offset;
	}
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDrop()
{
	Drop();
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdDelay()
{
	ACScript->delayCount = Pop();
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdDelayDirect()
{
	ACScript->delayCount = ReadCodeInt();
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdRandom()
{
	int low;
	int high;

	high = Pop();
	low = Pop();
	Push(low + (P_Random(RandomClass::Hexen) % (high - low + 1)));
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdRandomDirect()
{
	int low;
	int high;

	low = ReadCodeInt();
	high = ReadCodeInt();
	Push(low + (P_Random(RandomClass::Hexen) % (high - low + 1)));
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdThingCount()
{
	int tid;

	tid = Pop();
	ThingCount(Pop(), tid);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdThingCountDirect()
{
	int type;

	type = ReadCodeInt();
	ThingCount(type, ReadCodeInt());
	return AcsScriptAction::Continue;
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

static AcsScriptAction CmdTagWait()
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForTag;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdTagWaitDirect()
{
	ACSInfo[ACScript->infoIndex].waitValue = ReadCodeInt();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForTag;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdPolyWait()
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForPoly;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdPolyWaitDirect()
{
	ACSInfo[ACScript->infoIndex].waitValue = ReadCodeInt();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForPoly;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdChangeFloor()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdChangeFloorDirect()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdChangeCeiling()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdChangeCeilingDirect()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdRestart()
{
	PCodeOffset = ACSInfo[ACScript->infoIndex].offset;
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAndLogical()
{
	Push(Pop() && Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdOrLogical()
{
	Push(Pop() || Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAndBitwise()
{
	Push(Pop() & Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdOrBitwise()
{
	Push(Pop() | Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdEorBitwise()
{
	Push(Pop() ^ Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdNegateLogical()
{
	Push(!Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLShift()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() << operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdRShift()
{
	int operand2;

	operand2 = Pop();
	Push(Pop() >> operand2);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdUnaryMinus()
{
	Push(-Pop());
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdIfNotGoto()
{
	int offset;

	offset = ReadOffset();

	if(Pop() == 0)
	{
		PCodeOffset = offset;
	}
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdLineSide()
{
	Push(ACScript->side);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdScriptWait()
{
	ACSInfo[ACScript->infoIndex].waitValue = Pop();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForScript;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdScriptWaitDirect()
{
	ACSInfo[ACScript->infoIndex].waitValue = ReadCodeInt();
	ACSInfo[ACScript->infoIndex].state = AcsState::WaitingForScript;
	return AcsScriptAction::Stop;
}

static AcsScriptAction CmdClearLineSpecial()
{
	if(ACScript->line)
	{
		ACScript->line->special = 0;
	}
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdCaseGoto()
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

	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdBeginPrint()
{
	*PrintBuffer = 0;
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdEndPrint()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdEndPrintBold()
{
	int i;

	for(i = 0; i < g_maxplayers; i++)
	{
		if(playeringame[i])
		{
			P_SetYellowMessage(&players[i], PrintBuffer, true);
		}
	}
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPrintString()
{
	M_StringConcat(PrintBuffer, StringLookup(Pop()), sizeof(PrintBuffer));
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPrintNumber()
{
	char tempStr[16];

	snprintf(tempStr, sizeof(tempStr), "%d", Pop());
	M_StringConcat(PrintBuffer, tempStr, sizeof(PrintBuffer));
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPrintCharacter()
{
	char tempStr[2];

	tempStr[0] = Pop();
	tempStr[1] = '\0';
	M_StringConcat(PrintBuffer, tempStr, sizeof(PrintBuffer));

	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdPlayerCount()
{
	int i;
	int count;

	count = 0;
	for(i = 0; i < g_maxplayers; i++)
	{
		count += playeringame[i];
	}
	Push(count);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdGameType()
{
	AcsGameType gametype;

	if(netgame == false)
	{
		gametype = AcsGameType::SinglePlayer;
	}
	else if(deathmatch)
	{
		gametype = AcsGameType::NetDeathmatch;
	}
	else
	{
		gametype = AcsGameType::NetCooperative;
	}
	Push(std::to_underlying(gametype));
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdGameSkill()
{
	Push(gameskill);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdTimer()
{
	Push(leveltime);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSectorSound()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdThingSound()
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
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdAmbientSound()
{
	int volume;

	volume = Pop();
	S_StartSoundAtVolume(nullptr, S_GetSoundID(StringLookup(Pop())), volume, false, 0);
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSoundSequence()
{
	mobj_t* mobj;

	mobj = nullptr;
	if(ACScript->line)
	{
		mobj = (mobj_t*)&ACScript->line->frontsector->soundorg;
	}
	SN_StartSequenceName(mobj, StringLookup(Pop()));
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSetLineTexture()
{
	line_t* line;
	int lineTag;
	int side;
	AcsLineTexture position;
	int texture;
	int searcher;

	texture = R_TextureNumForName(StringLookup(Pop()));
	position = static_cast<AcsLineTexture>(Pop());
	side = Pop();
	lineTag = Pop();
	searcher = -1;
	while((line = P_FindLine(lineTag, &searcher)) != nullptr)
	{
		if(position == AcsLineTexture::Middle)
		{
			sides[line->sidenum[side]].midtexture = texture;
		}
		else if(position == AcsLineTexture::Bottom)
		{
			sides[line->sidenum[side]].bottomtexture = texture;
		}
		else
		{
			// AcsLineTexture::Top
			sides[line->sidenum[side]].toptexture = texture;
		}
	}
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSetLineBlocking()
{
	line_t* line;
	int lineTag;
	LineFlag blocking;
	int searcher;

	blocking = Pop() ? LineFlag::Blocking : LineFlag{};
	lineTag = Pop();
	searcher = -1;
	while((line = P_FindLine(lineTag, &searcher)) != nullptr)
	{
		line->flags = (line->flags - LineFlag::Blocking) | blocking;
	}
	return AcsScriptAction::Continue;
}

static AcsScriptAction CmdSetLineSpecial()
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
	return AcsScriptAction::Continue;
}
