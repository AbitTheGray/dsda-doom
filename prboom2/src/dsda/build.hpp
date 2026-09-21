// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Build Mode

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "d_event.hpp"
#include "d_ticcmd.hpp"
#include "tables.hpp"

dboolean dsda_AllowBuilding();
dboolean dsda_BuildMode();
void dsda_QueueBuildCommands(ticcmd_t* cmds, int depth);
dboolean dsda_BuildPlayback();
void dsda_CopyBuildCmd(ticcmd_t* cmd);
void dsda_ReadBuildCmd(ticcmd_t* cmd);
void dsda_EnterBuildMode();
void dsda_RefreshBuildMode();
dboolean dsda_BuildResponder(event_t* ev);
void dsda_ToggleBuildTurbo();
dboolean dsda_AdvanceFrame();
dboolean dsda_BuildMF(int x);
dboolean dsda_BuildMB(int x);
dboolean dsda_BuildSR(int x);
dboolean dsda_BuildSL(int x);
dboolean dsda_BuildTR(int x);
dboolean dsda_BuildTL(int x);
dboolean dsda_BuildFU(int x);
dboolean dsda_BuildFD(int x);
dboolean dsda_BuildFC();
dboolean dsda_BuildLU(int x);
dboolean dsda_BuildLD(int x);
dboolean dsda_BuildLC();
dboolean dsda_BuildUA(int x);

#ifdef __cplusplus
}
#endif
