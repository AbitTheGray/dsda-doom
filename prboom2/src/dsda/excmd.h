// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Cmd

#pragma once

#include "d_ticcmd.h"

#define XC_JUMP   0x01
#define XC_SAVE   0x02
#define XC_LOAD   0x04
#define XC_GOD    0x08
#define XC_NOCLIP 0x10
#define XC_LOOK   0x20

#define XC_LOOK_RESET -32768

void dsda_EnableExCmd(void);
void dsda_DisableExCmd(void);
dboolean dsda_AllowExCmd(void);
dboolean dsda_ExCmdDemo(void);
void dsda_EnableCasualExCmdFeatures(void);
dboolean dsda_AllowCasualExCmdFeatures(void);
dboolean dsda_AllowJumping(void);
dboolean dsda_FreeAim(void);
void dsda_ReadExCmd(ticcmd_t* cmd, const byte** p);
void dsda_WriteExCmd(char** p, ticcmd_t* cmd);
void dsda_ResetExCmdQueue(void);
void dsda_PopExCmdQueue(ticcmd_t* cmd);
void dsda_QueueExCmdJump(void);
void dsda_QueueExCmdLook(short look);
void dsda_QueueExCmdSave(int slot);
void dsda_QueueExCmdLoad(int slot);
void dsda_QueueExCmdGod(void);
void dsda_QueueExCmdNoClip(void);
