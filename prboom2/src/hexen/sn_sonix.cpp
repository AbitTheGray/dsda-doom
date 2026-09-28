// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

#include <string.h>

#include "doomdef.hpp"
#include "m_random.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "p_mobj.hpp"
#include "lprintf.hpp"
#include "sc_man.hpp"

#include "dsda/pause.hpp"

#include "sn_sonix.hpp"

#define SS_MAX_SCRIPTS 64
#define SS_TEMPBUFFER_SIZE 1024
#define SS_SEQUENCE_NAME_LENGTH 32

#define SS_SCRIPT_NAME          "SNDSEQ"
#define SS_STRING_PLAY          "play"
#define SS_STRING_PLAYUNTILDONE "playuntildone"
#define SS_STRING_PLAYTIME      "playtime"
#define SS_STRING_PLAYREPEAT    "playrepeat"
#define SS_STRING_DELAY         "delay"
#define SS_STRING_DELAYRAND     "delayrand"
#define SS_STRING_VOLUME        "volume"
#define SS_STRING_END           "end"
#define SS_STRING_STOPSOUND     "stopsound"

enum struct SoundSeqCmd : int32_t
{
	None,
	Play,
	WaitUntilDone, // used by PLAYUNTILDONE
	PlayTime,
	PlayRepeat,
	Delay,
	DelayRand,
	Volume,
	StopSound,
	End
};

static void VerifySequencePtr(int* base, int* ptr);
static SfxId GetSoundOffset(char* name);

static struct
{
	char name[SS_SEQUENCE_NAME_LENGTH];
	int scriptNum;
	SfxId stopSound;
} SequenceTranslate[std::to_underlying(SoundSequence::Numseq)] =
{
	{
		"Platform", 0, SfxId::None
	},
	{
		"Platform", 0, SfxId::None
	}, // a 'heavy' platform is just a platform
	{
		"PlatformMetal", 0, SfxId::None
	},
	{
		"Platform", 0, SfxId::None
	}, // same with a 'creak' platform
	{
		"Silence", 0, SfxId::None
	},
	{
		"Lava", 0, SfxId::None
	},
	{
		"Water", 0, SfxId::None
	},
	{
		"Ice", 0, SfxId::None
	},
	{
		"Earth", 0, SfxId::None
	},
	{
		"PlatformMetal2", 0, SfxId::None
	},
	{
		"DoorNormal", 0, SfxId::None
	},
	{
		"DoorHeavy", 0, SfxId::None
	},
	{
		"DoorMetal", 0, SfxId::None
	},
	{
		"DoorCreak", 0, SfxId::None
	},
	{
		"Silence", 0, SfxId::None
	},
	{
		"Lava", 0, SfxId::None
	},
	{
		"Water", 0, SfxId::None
	},
	{
		"Ice", 0, SfxId::None
	},
	{
		"Earth", 0, SfxId::None
	},
	{
		"DoorMetal2", 0, SfxId::None
	},
	{
		"Wind", 0, SfxId::None
	}
};

static int* SequenceData[SS_MAX_SCRIPTS];

int ActiveSequences;
seqnode_t* SequenceListHead;

static void VerifySequencePtr(int* base, int* ptr)
{
	if(ptr - base > SS_TEMPBUFFER_SIZE)
	{
		Log::Fatal("VerifySequencePtr:  tempPtr >= {}\n", SS_TEMPBUFFER_SIZE);
	}
}

static SfxId GetSoundOffset(char* name)
{
	for(int32_t i = 0; i < num_sfx; i++)
	{
		if(!strcasecmp(name, S_sfx[i].tagname))
		{
			return static_cast<SfxId>(i);
		}
	}
	SC_ScriptError("GetSoundOffset:  Unknown sound name\n");
	return SfxId::None;
}

void SN_InitSequenceScript()
{
	int i, j;
	int inSequence;
	int* tempDataStart = nullptr;
	int* tempDataPtr = nullptr;

	inSequence = -1;
	ActiveSequences = 0;
	for(i = 0; i < SS_MAX_SCRIPTS; i++)
	{
		SequenceData[i] = nullptr;
	}
	SC_OpenLump(SS_SCRIPT_NAME);
	while(SC_GetString())
	{
		if(*sc_String == ':')
		{
			if(inSequence != -1)
			{
				SC_ScriptError("SN_InitSequenceScript:  Nested Script Error");
			}
			tempDataStart = (int*)Z_Malloc(SS_TEMPBUFFER_SIZE);
			memset(tempDataStart, 0, SS_TEMPBUFFER_SIZE);
			tempDataPtr = tempDataStart;
			for(i = 0; i < SS_MAX_SCRIPTS; i++)
			{
				if(SequenceData[i] == nullptr)
				{
					break;
				}
			}
			if(i == SS_MAX_SCRIPTS)
			{
				Log::Fatal("Number of SS Scripts >= SS_MAX_SCRIPTS");
			}
			for(j = 0; j < std::to_underlying(SoundSequence::Numseq); j++)
			{
				if(!strcasecmp(SequenceTranslate[j].name, sc_String + 1))
				{
					SequenceTranslate[j].scriptNum = i;
					inSequence = j;
					break;
				}
			}
			continue; // parse the next command
		}
		if(inSequence == -1)
		{
			continue;
		}
		if(SC_Compare(SS_STRING_PLAYUNTILDONE))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::Play);
			*tempDataPtr++ = std::to_underlying(GetSoundOffset(sc_String));
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::WaitUntilDone);
		}
		else if(SC_Compare(SS_STRING_PLAY))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::Play);
			*tempDataPtr++ = std::to_underlying(GetSoundOffset(sc_String));
		}
		else if(SC_Compare(SS_STRING_PLAYTIME))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::Play);
			*tempDataPtr++ = std::to_underlying(GetSoundOffset(sc_String));
			SC_MustGetNumber();
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::Delay);
			*tempDataPtr++ = sc_Number;
		}
		else if(SC_Compare(SS_STRING_PLAYREPEAT))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			SC_MustGetString();
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::PlayRepeat);
			*tempDataPtr++ = std::to_underlying(GetSoundOffset(sc_String));
		}
		else if(SC_Compare(SS_STRING_DELAY))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::Delay);
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
		}
		else if(SC_Compare(SS_STRING_DELAYRAND))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::DelayRand);
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
		}
		else if(SC_Compare(SS_STRING_VOLUME))
		{
			VerifySequencePtr(tempDataStart, tempDataPtr);
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::Volume);
			SC_MustGetNumber();
			*tempDataPtr++ = sc_Number;
		}
		else if(SC_Compare(SS_STRING_END))
		{
			int dataSize;

			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::End);
			dataSize = (tempDataPtr - tempDataStart) * sizeof(int);
			SequenceData[i] = (int*)Z_Malloc(dataSize);
			memcpy(SequenceData[i], tempDataStart, dataSize);
			Z_Free(tempDataStart);
			inSequence = -1;
		}
		else if(SC_Compare(SS_STRING_STOPSOUND))
		{
			SC_MustGetString();
			SequenceTranslate[inSequence].stopSound =
				GetSoundOffset(sc_String);
			*tempDataPtr++ = std::to_underlying(SoundSeqCmd::StopSound);
		}
		else
		{
			SC_ScriptError("SN_InitSequenceScript:  Unknown command.\n");
		}
	}
}

void SN_StartSequence(mobj_t* mobj, int sequence)
{
	seqnode_t* node;

	SN_StopSequence(mobj); // Stop any previous sequence
	node = (seqnode_t*)Z_Malloc(sizeof(seqnode_t));
	node->sequencePtr = SequenceData[SequenceTranslate[sequence].scriptNum];
	node->sequence = sequence;
	node->mobj = mobj;
	node->delayTics = 0;
	node->stopSound = SequenceTranslate[sequence].stopSound;
	node->volume = 127; // Start at max volume

	if(!SequenceListHead)
	{
		SequenceListHead = node;
		node->next = node->prev = nullptr;
	}
	else
	{
		SequenceListHead->prev = node;
		node->next = SequenceListHead;
		node->prev = nullptr;
		SequenceListHead = node;
	}
	ActiveSequences++;
	return;
}

void SN_StartSequenceName(mobj_t* mobj, const char* name)
{
	int i;

	for(i = 0; i < std::to_underlying(SoundSequence::Numseq); i++)
	{
		if(!strcmp(name, SequenceTranslate[i].name))
		{
			SN_StartSequence(mobj, i);
			return;
		}
	}
}

void SN_StopSequence(mobj_t* mobj)
{
	seqnode_t* node;
	seqnode_t* next_node;

	for(node = SequenceListHead; node; node = next_node)
	{
		next_node = node->next;
		if(node->mobj == mobj)
		{
			S_StopSound(mobj);
			if(node->stopSound != SfxId::None)
			{
				S_StartSoundAtVolume(mobj, node->stopSound, node->volume, false, 0);
			}
			if(SequenceListHead == node)
			{
				SequenceListHead = node->next;
			}
			if(node->prev)
			{
				node->prev->next = node->next;
			}
			if(node->next)
			{
				node->next->prev = node->prev;
			}
			Z_Free(node);
			ActiveSequences--;
		}
	}
}

void SN_UpdateActiveSequences()
{
	seqnode_t* node;
	seqnode_t* next_node;
	dboolean sndPlaying;

	if(!ActiveSequences || dsda_Paused())
	{
		// No sequences currently playing/game is paused
		return;
	}
	for(node = SequenceListHead; node; node = next_node)
	{
		next_node = node->next;
		if(node->delayTics)
		{
			node->delayTics--;
			continue;
		}
		sndPlaying = S_GetSoundPlayingInfo(node->mobj, node->currentSoundID);
		switch(static_cast<SoundSeqCmd>(*node->sequencePtr))
		{
			case SoundSeqCmd::Play:
				if(!sndPlaying)
				{
					node->currentSoundID = static_cast<SfxId>(*(node->sequencePtr + 1));
					S_StartSoundAtVolume(node->mobj, node->currentSoundID,
						node->volume, false, 0);
				}
				node->sequencePtr += 2;
				break;
			case SoundSeqCmd::WaitUntilDone:
				if(!sndPlaying)
				{
					node->sequencePtr++;
					node->currentSoundID = SfxId::None;
				}
				break;
			case SoundSeqCmd::PlayRepeat:
				if(!sndPlaying)
				{
					node->currentSoundID = static_cast<SfxId>(*(node->sequencePtr + 1));
					S_StartSoundAtVolume(node->mobj, node->currentSoundID,
						node->volume, false, 0);
				}
				break;
			case SoundSeqCmd::Delay:
				node->delayTics = *(node->sequencePtr + 1);
				node->sequencePtr += 2;
				node->currentSoundID = SfxId::None;
				break;
			case SoundSeqCmd::DelayRand:
				node->delayTics = *(node->sequencePtr + 1) +
					M_Random() % (*(node->sequencePtr + 2) -
						*(node->sequencePtr + 1));
				node->sequencePtr += 2;
				node->currentSoundID = SfxId::None;
				break;
			case SoundSeqCmd::Volume:
				node->volume = (127 * (*(node->sequencePtr + 1))) / 100;
				node->sequencePtr += 2;
				break;
			case SoundSeqCmd::StopSound:
				// Wait until something else stops the sequence
				break;
			case SoundSeqCmd::End:
				SN_StopSequence(node->mobj);
				break;
			default:
				break;
		}
	}
}

void SN_StopAllSequences()
{
	seqnode_t* node;
	seqnode_t* next_node;

	for(node = SequenceListHead; node; node = next_node)
	{
		next_node = node->next;
		node->stopSound = SfxId::None; // don't play any stop sounds
		SN_StopSequence(node->mobj);
	}
}

int SN_GetSequenceOffset(int sequence, int* sequencePtr)
{
	return (sequencePtr -
		SequenceData[SequenceTranslate[sequence].scriptNum]);
}

void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume,
	SfxId currentSoundID)
{
	int i;
	seqnode_t* node;

	i = 0;
	node = SequenceListHead;
	while(node && i < nodeNum)
	{
		node = node->next;
		i++;
	}
	if(!node)
	{
		// reach the end of the list before finding the nodeNum-th node
		return;
	}
	node->delayTics = delayTics;
	node->volume = volume;
	node->sequencePtr += seqOffset;
	node->currentSoundID = currentSoundID;
}
