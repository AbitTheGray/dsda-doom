// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *    Contains the main wait loop, waiting for the next tic.
 *    Rewritten for LxDoom, but based around bits of the old code.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "doomtype.hpp"
#include "doomstat.hpp"
#include "d_net.hpp"
#include "z_zone.hpp"

#include "d_main.hpp"
#include "g_game.hpp"
#include "m_menu.hpp"

#include "i_system.hpp"
#include "i_main.hpp"
#include "i_video.hpp"
#include "r_fps.hpp"
#include "lprintf.hpp"
#include "e6y.hpp"

#include "dsda/args.hpp"
#include "dsda/settings.hpp"
#include "dsda/time.hpp"

ticcmd_t local_cmds[MAX_MAXPLAYERS][BACKUPTICS];
int maketic;
int solo_net = 0;

void D_InitFakeNetGame()
{
	int i;

	consoleplayer = displayplayer = 0;
	solo_net = dsda_Flag(ArgId::SoloNet);
	coop_spawns = dsda_Flag(ArgId::CoopSpawns);
	netgame = solo_net;

	playeringame[0] = true;
	for(i = 1; i < g_maxplayers; i++)
		playeringame[i] = false;
}

void FakeNetUpdate()
{
	static int lastmadetic;

	if(isExtraDDisplay)
		return;

	{
		// Build new ticcmds
		int newtics = dsda_GetTick() - lastmadetic;
		lastmadetic += newtics;

		while(newtics--)
		{
			I_StartTic();
			if(maketic - gametic > BACKUPTICS / 2) break;

			// e6y
			// Eliminating the sudden jump of six frames(BACKUPTICS/2)
			// after change of game_speed.
			if(maketic - gametic && gametic <= force_singletics_to && dsda_GameSpeed() < 200) break;

			G_BuildTiccmd(&local_cmds[0][maketic % BACKUPTICS]);
			maketic++;
		}
	}
}

// Implicitly tracked whenever we check the current tick
int ms_to_next_tick;

void TryRunTics()
{
	int runtics;
	int entertime = dsda_GetTick();

	// Wait for tics to run
	while(1)
	{
		FakeNetUpdate();
		runtics = maketic - gametic;
		if(!runtics)
		{
			if(!movement_smooth)
			{
				I_uSleep(ms_to_next_tick * 1000);
			}
			if(dsda_GetTick() - entertime > 10)
			{
				M_Ticker();
				return;
			}

			if(gametic > 0)
			{
				WasRenderedInTryRunTics = true;
				if(movement_smooth && gamestate == wipegamestate)
				{
					isExtraDDisplay = true;
					D_Display(-1);
					isExtraDDisplay = false;
				}
			}
		}
		else break;
	}

	while(runtics--)
	{
		if(advancedemo)
			D_DoAdvanceDemo();
		M_Ticker();
		G_Ticker();
		gametic++;
		FakeNetUpdate();
	}
}
