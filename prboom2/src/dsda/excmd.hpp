// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Cmd

#pragma once

#include <cstdint>

#include "d_ticcmd.hpp"

// `excmd_t::look` value that resets the view pitch instead of changing it.
inline constexpr int16_t k_ExCmdLookReset = -32768;

#ifdef __cplusplus
extern "C"
{
#endif

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
