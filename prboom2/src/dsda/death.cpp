// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Death

#include <utility>

#include "doomstat.hpp"
#include "d_player.hpp"
#include "v_video.hpp"

#include "dsda/configuration.hpp"
#include "dsda/excmd.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/save.hpp"
#include "dsda/skill_info.hpp"

#include "death.hpp"

extern int inv_ptr;
extern int curpos;
extern int newtorch;
extern int newtorchdelta;

enum struct DeathUseAction : int32_t
{
	Default,
	Nothing,
	Reload,
};

static DeathUseAction dsda_DeathUseAction()
{
	// TODO: possible "allow respawn" mapinfo flag
	dboolean mapinfo_respawn = skill_info.flags & SI_PLAYER_RESPAWN;

	if(demoplayback || demorecording || mapinfo_respawn)
		return DeathUseAction::Default;

	// the config stores the action as a plain int
	return static_cast<DeathUseAction>(dsda_IntConfig(ConfigId::DeathUseAction));
}

void dsda_DeathUse(player_t* player)
{
	switch(dsda_DeathUseAction())
	{
		case DeathUseAction::Default:
		default:
			if(raven)
			{
				if(player == &players[consoleplayer])
				{
					V_SetPalette(0);
					inv_ptr = 0;
					curpos = 0;
					newtorch = 0;
					newtorchdelta = 0;
				}

				if(hexen)
				{
					player->mo->special1.i = std::to_underlying(player->pclass);
					if(player->mo->special1.i > 2)
					{
						player->mo->special1.i = 0;
					}
				}

				// Let the mobj know the player has entered the reborn state.  Some
				// mobjs need to know when it's ok to remove themselves.
				player->mo->special2.i = 666;
			}

			player->playerstate = PlayerState::Reborn;
			break;
		case DeathUseAction::Nothing:
			break;
		case DeathUseAction::Reload:
		{
			int slot = dsda_LastSaveSlot();
			static int last_load_tic;

			if(slot >= 0 && gametic > last_load_tic + 1)
			{
				last_load_tic = gametic;
				dsda_QueueExCmdLoad(slot);
			}
		}
		break;
	}
}
