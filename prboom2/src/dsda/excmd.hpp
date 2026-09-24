// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Cmd

#pragma once

#include "d_ticcmd.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define XC_JUMP   0x01
#define XC_SAVE   0x02
#define XC_LOAD   0x04
#define XC_GOD    0x08
#define XC_NOCLIP 0x10
#define XC_LOOK   0x20

#define XC_LOOK_RESET -32768

void dsda_EnableExCmd();
void dsda_DisableExCmd();
dboolean dsda_AllowExCmd();
dboolean dsda_ExCmdDemo();
void dsda_EnableCasualExCmdFeatures();
dboolean dsda_AllowCasualExCmdFeatures();
dboolean dsda_AllowJumping();
dboolean dsda_FreeAim();
void dsda_ReadExCmd(ticcmd_t* cmd, const byte** p);
void dsda_WriteExCmd(char** p, ticcmd_t* cmd);
void dsda_ResetExCmdQueue();
void dsda_PopExCmdQueue(ticcmd_t* cmd);
void dsda_QueueExCmdJump();
void dsda_QueueExCmdLook(short look);
void dsda_QueueExCmdSave(int slot);
void dsda_QueueExCmdLoad(int slot);
void dsda_QueueExCmdGod();
void dsda_QueueExCmdNoClip();

#ifdef __cplusplus
}
#endif
