// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "p_mobj.h"

typedef struct seqnode_s seqnode_t;

struct seqnode_s
{
	int* sequencePtr;
	int sequence;
	mobj_t* mobj;
	int currentSoundID;
	int delayTics;
	int volume;
	int stopSound;
	seqnode_t* prev;
	seqnode_t* next;
};

extern int ActiveSequences;
extern seqnode_t* SequenceListHead;

void SN_InitSequenceScript(void);
void SN_StartSequence(mobj_t* mobj, int sequence);
void SN_StartSequenceName(mobj_t* mobj, const char* name);
void SN_StopSequence(mobj_t* mobj);
void SN_UpdateActiveSequences(void);
void SN_StopAllSequences(void);
int SN_GetSequenceOffset(int sequence, int* sequencePtr);
void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume, int currentSoundID);
