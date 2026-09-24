// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Build Mode

#include <utility>

#include "doomstat.hpp"
#include "g_game.hpp"

#include "dsda/args.hpp"
#include "dsda/brute_force.hpp"
#include "dsda/demo.hpp"
#include "dsda/exhud.hpp"
#include "dsda/features.hpp"
#include "dsda/input.hpp"
#include "dsda/key_frame.hpp"
#include "dsda/pause.hpp"
#include "dsda/playback.hpp"
#include "dsda/settings.hpp"
#include "dsda/skip.hpp"

#include "build.hpp"

typedef struct
{
	ticcmd_t* cmds;
	int depth;
	int original_depth;
} build_cmd_queue_t;

static dboolean allow_turbo;
static dboolean build_mode;
static dboolean advance_frame;
static ticcmd_t build_cmd;
static ticcmd_t overwritten_cmd;
static int overwritten_logictic;
static int build_cmd_tic = -1;
static dboolean replace_source = true;
static build_cmd_queue_t cmd_queue;

static signed char forward50()
{
	return dsda_Flag(ArgId::Stroller) ? pclass[std::to_underlying(players[consoleplayer].pclass)].forwardmove[0] : pclass[std::to_underlying(players[consoleplayer].pclass)].forwardmove[1];
}

static signed char strafe40()
{
	return pclass[std::to_underlying(players[consoleplayer].pclass)].sidemove[1];
}

static signed char strafe50()
{
	return dsda_Flag(ArgId::Stroller) ? 0 : forward50();
}

static signed short shortTic()
{
	return (1 << 8);
}

static signed char maxForward()
{
	return allow_turbo ? 127 : forward50();
}

static signed char minBackward()
{
	return allow_turbo ? -127 : -forward50();
}

static signed char maxStrafeRight()
{
	return allow_turbo ? 127 : strafe50();
}

static signed char minStrafeLeft()
{
	return allow_turbo ? -128 : -strafe50();
}

void dsda_ChangeBuildCommand()
{
	if(demoplayback)
		dsda_JoinDemo(nullptr);

	replace_source = true;
	build_cmd_tic = true_logictic - 1;
	dsda_JumpToLogicTicFrom(true_logictic, true_logictic - 1);
}

dboolean dsda_BuildMF(int x)
{
	if(x < 0 || x > 127)
		return false;

	build_cmd.forwardmove = x;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildMB(int x)
{
	if(x < 0 || x > 127)
		return false;

	build_cmd.forwardmove = -x;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildSR(int x)
{
	if(x < 0 || x > 127)
		return false;

	build_cmd.sidemove = x;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildSL(int x)
{
	if(x < 0 || x > 128)
		return false;

	build_cmd.sidemove = -x;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildTR(int x)
{
	if(x < 0 || x > 128)
		return false;

	build_cmd.angleturn = (-x << 8);

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildTL(int x)
{
	if(x < 0 || x > 127)
		return false;

	build_cmd.angleturn = (x << 8);

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildFU(int x)
{
	if(x < 0 || x > 7)
		return false;

	build_cmd.lookfly &= 0x0f;
	build_cmd.lookfly |= (x << 4);

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildFD(int x)
{
	if(x < 0 || x > 7)
		return false;

	if(x)
		x = 16 - x;

	build_cmd.lookfly &= 0x0f;
	build_cmd.lookfly |= (x << 4);

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildFC()
{
	build_cmd.lookfly &= 0x0f;
	build_cmd.lookfly |= 0x80;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildLU(int x)
{
	if(x < 0 || x > 7)
		return false;

	build_cmd.lookfly &= 0xf0;
	build_cmd.lookfly |= x;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildLD(int x)
{
	if(x < 0 || x > 7)
		return false;

	if(x)
		x = 16 - x;

	build_cmd.lookfly &= 0xf0;
	build_cmd.lookfly |= x;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildLC()
{
	build_cmd.lookfly &= 0xf0;
	build_cmd.lookfly |= 0x08;

	dsda_ChangeBuildCommand();

	return true;
}

dboolean dsda_BuildUA(int x)
{
	if(x < 0 || x > (heretic ? 10 : 15))
		return false;

	build_cmd.arti = x;

	dsda_ChangeBuildCommand();

	return true;
}

static void buildForward()
{
	if(allow_turbo)
	{
		if(build_cmd.forwardmove == 127)
			build_cmd.forwardmove = 0;
		else if(build_cmd.forwardmove == forward50())
			build_cmd.forwardmove = 127;
		else
			build_cmd.forwardmove = forward50();
	}
	else
	{
		if(build_cmd.forwardmove == forward50())
			build_cmd.forwardmove = 0;
		else
			build_cmd.forwardmove = forward50();
	}

	dsda_ChangeBuildCommand();
}

static void buildBackward()
{
	if(allow_turbo)
	{
		if(build_cmd.forwardmove == -127)
			build_cmd.forwardmove = 0;
		else if(build_cmd.forwardmove == -forward50())
			build_cmd.forwardmove = -127;
		else
			build_cmd.forwardmove = -forward50();
	}
	else
	{
		if(build_cmd.forwardmove == -forward50())
			build_cmd.forwardmove = 0;
		else
			build_cmd.forwardmove = -forward50();
	}

	dsda_ChangeBuildCommand();
}

static void buildFineForward()
{
	if(build_cmd.forwardmove < maxForward())
		++build_cmd.forwardmove;

	dsda_ChangeBuildCommand();
}

static void buildFineBackward()
{
	if(build_cmd.forwardmove > minBackward())
		--build_cmd.forwardmove;

	dsda_ChangeBuildCommand();
}

static void buildStrafeRight()
{
	if(allow_turbo)
	{
		if(build_cmd.sidemove == 127)
			build_cmd.sidemove = 0;
		else if(build_cmd.sidemove == strafe50())
			build_cmd.sidemove = 127;
		else
			build_cmd.sidemove = strafe50();
	}
	else
	{
		if(build_cmd.sidemove == strafe50())
			build_cmd.sidemove = 0;
		else
			build_cmd.sidemove = strafe50();
	}

	dsda_ChangeBuildCommand();
}

static void buildStrafeLeft()
{
	if(allow_turbo)
	{
		if(build_cmd.sidemove == -128)
			build_cmd.sidemove = 0;
		else if(build_cmd.sidemove == -strafe50())
			build_cmd.sidemove = -128;
		else
			build_cmd.sidemove = -strafe50();
	}
	else
	{
		if(build_cmd.sidemove == -strafe50())
			build_cmd.sidemove = 0;
		else
			build_cmd.sidemove = -strafe50();
	}

	dsda_ChangeBuildCommand();
}

static void buildFineStrafeRight()
{
	if(build_cmd.sidemove < maxStrafeRight())
		++build_cmd.sidemove;

	dsda_ChangeBuildCommand();
}

static void buildFineStrafeLeft()
{
	if(build_cmd.sidemove > minStrafeLeft())
		--build_cmd.sidemove;

	dsda_ChangeBuildCommand();
}

static void buildTurnRight()
{
	build_cmd.angleturn -= shortTic();

	dsda_ChangeBuildCommand();
}

static void buildTurnLeft()
{
	build_cmd.angleturn += shortTic();

	dsda_ChangeBuildCommand();
}

static void buildUse()
{
	build_cmd.buttons = static_cast<ButtonCode>(std::to_underlying(build_cmd.buttons) ^ std::to_underlying(ButtonCode::Use));

	dsda_ChangeBuildCommand();
}

static void buildFire()
{
	build_cmd.buttons = static_cast<ButtonCode>(std::to_underlying(build_cmd.buttons) ^ std::to_underlying(ButtonCode::Attack));

	dsda_ChangeBuildCommand();
}

static void buildWeapon(int weapon)
{
	int cmdweapon;

	cmdweapon = weapon << std::to_underlying(ButtonCode::WeaponShift);

	if((build_cmd.buttons & ButtonCode::Change) != ButtonCode{} && ButtonWeapon(build_cmd.buttons) == (cmdweapon >> std::to_underlying(ButtonCode::WeaponShift)))
		build_cmd.buttons -= ButtonCode::Change;
	else
		build_cmd.buttons |= ButtonCode::Change;

	build_cmd.buttons -= ButtonCode::WeaponMask;
	if((build_cmd.buttons & ButtonCode::Change) != ButtonCode{})
		build_cmd.buttons |= static_cast<ButtonCode>(cmdweapon);

	dsda_ChangeBuildCommand();
}

static void resetCmd()
{
	memset(&build_cmd, 0, sizeof(build_cmd));
}

dboolean dsda_AllowBuilding()
{
	return !dsda_StrictMode();
}

dboolean dsda_BuildMode()
{
	return build_mode;
}

void dsda_QueueBuildCommands(ticcmd_t* cmds, int depth)
{
	cmd_queue.original_depth = depth;
	cmd_queue.depth = depth;

	if(cmd_queue.cmds)
		Z_Free(cmd_queue.cmds);

	cmd_queue.cmds = static_cast<ticcmd_t *>(Z_Malloc(depth * sizeof(*cmds)));
	memcpy(cmd_queue.cmds, cmds, depth * sizeof(*cmds));
}

static void dsda_PopCommandQueue(ticcmd_t* cmd)
{
	*cmd = cmd_queue.cmds[cmd_queue.original_depth - cmd_queue.depth];
	--cmd_queue.depth;

	if(!cmd_queue.depth)
		dsda_ExitSkipMode();
}

dboolean dsda_BuildPlayback()
{
	return !replace_source;
}

void dsda_CopyBuildCmd(ticcmd_t* cmd)
{
	*cmd = build_cmd;
}

void dsda_ReadBuildCmd(ticcmd_t* cmd)
{
	if(cmd_queue.depth)
		dsda_PopCommandQueue(cmd);
	else if(dsda_BruteForce())
		dsda_CopyBruteForceCommand(cmd);
	else if(true_logictic == build_cmd_tic)
	{
		*cmd = build_cmd;
		build_cmd_tic = -1;
	}
	else
		dsda_CopyPendingCmd(cmd, 0);

	dsda_JoinDemoCmd(cmd);
}

void dsda_EnterBuildMode()
{
	dsda_TrackFeature(FeatureFlag::Build);

	if(!demorecording)
	{
		if(!build_mode)
			dsda_StoreTempKeyFrame();

		advance_frame = true;
	}

	if(!true_logictic)
		advance_frame = true;

	build_mode = true;
	dsda_ApplyPauseMode(PAUSE_BUILDMODE);

	dsda_RefreshExHudCommandDisplay();
}

void dsda_ExitBuildMode()
{
	build_mode = false;
	dsda_RemovePauseMode(PAUSE_BUILDMODE);

	dsda_RefreshExHudCommandDisplay();
}

void dsda_RefreshBuildMode()
{
	if(demoplayback)
		replace_source = false;

	if(!dsda_SkipMode() &&
		overwritten_logictic != true_logictic - 1 &&
		build_cmd_tic == -1 &&
		true_logictic > 0)
	{
		dsda_CopyPriorCmd(&overwritten_cmd, 1);
		build_cmd = overwritten_cmd;
		overwritten_logictic = true_logictic - 1;
		replace_source = false;
	}
}

dboolean dsda_BuildResponder(event_t* ev)
{
	if(!dsda_AllowBuilding())
		return false;

	if(dsda_InputActivated(InputId::Build))
	{
		if(dsda_BuildMode())
			dsda_ExitBuildMode();
		else
			dsda_EnterBuildMode();

		return true;
	}

	if(!build_mode || menuactive != MenuActive::Inactive)
		return false;

	if(dsda_InputActivated(InputId::BuildSource))
	{
		replace_source = !replace_source;

		if(!replace_source)
		{
			build_cmd = overwritten_cmd;

			dsda_ChangeBuildCommand();
			replace_source = false;
		}

		return true;
	}

	if(dsda_InputActivated(InputId::BuildAdvanceFrame))
	{
		advance_frame = true;
		build_cmd_tic = true_logictic;

		build_cmd.angleturn = 0;
		build_cmd.arti = 0;
		build_cmd.buttons -= ButtonCode::Use;
		if((build_cmd.buttons & ButtonCode::Change) != ButtonCode{})
			build_cmd.buttons -= ButtonCode::Change | ButtonCode::WeaponMask;

		if(dsda_CopyPendingCmd(&overwritten_cmd, 0))
		{
			if(!replace_source)
				build_cmd = overwritten_cmd;
		}
		else
		{
			overwritten_cmd = build_cmd;
			replace_source = true;
		}

		overwritten_logictic = true_logictic;

		if(!demorecording)
			dsda_StoreTempKeyFrame();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildReverseFrame))
	{
		if(!demorecording)
		{
			doom_printf("Cannot reverse outside demo");
			return true;
		}

		if(true_logictic > 1)
		{
			dsda_CopyPriorCmd(&build_cmd, 2);
			overwritten_cmd = build_cmd;
			overwritten_logictic = true_logictic - 2;
			replace_source = false;

			dsda_JumpToLogicTic(true_logictic - 1);
		}

		return true;
	}

	if(dsda_InputActivated(InputId::BuildResetCommand))
	{
		resetCmd();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildForward))
	{
		buildForward();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildBackward))
	{
		buildBackward();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildFineForward))
	{
		buildFineForward();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildFineBackward))
	{
		buildFineBackward();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildStrafeRight))
	{
		buildStrafeRight();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildStrafeLeft))
	{
		buildStrafeLeft();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildFineStrafeRight))
	{
		buildFineStrafeRight();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildFineStrafeLeft))
	{
		buildFineStrafeLeft();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildTurnRight))
	{
		buildTurnRight();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildTurnLeft))
	{
		buildTurnLeft();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildUse))
	{
		buildUse();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildFire))
	{
		buildFire();

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon1))
	{
		buildWeapon(0);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon2))
	{
		buildWeapon(1);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon3))
	{
		buildWeapon(2);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon4))
	{
		buildWeapon(3);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon5))
	{
		buildWeapon(4);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon6))
	{
		buildWeapon(5);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon7))
	{
		buildWeapon(6);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon8))
	{
		buildWeapon(7);

		return true;
	}

	if(dsda_InputActivated(InputId::BuildWeapon9))
	{
		if(!demo_compatibility && gamemode == GameMode::Commercial)
			buildWeapon(8);

		return true;
	}

	if(dsda_InputActivated(InputId::JoinDemo))
		dsda_JoinDemo(nullptr);

	return false;
}

void dsda_ToggleBuildTurbo()
{
	allow_turbo = !allow_turbo;

	if(!allow_turbo)
	{
		if(build_cmd.forwardmove > maxForward())
			build_cmd.forwardmove = maxForward();
		else if(build_cmd.forwardmove < minBackward())
			build_cmd.forwardmove = minBackward();

		if(build_cmd.sidemove > maxStrafeRight())
			build_cmd.sidemove = maxStrafeRight();
		else if(build_cmd.sidemove < minStrafeLeft())
			build_cmd.sidemove = minStrafeLeft();
	}
}

dboolean dsda_AdvanceFrame()
{
	dboolean result;

	if(dsda_SkipMode())
		advance_frame = true;

	result = advance_frame;
	advance_frame = false;

	return result;
}
