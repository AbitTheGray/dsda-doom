// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Demo stuff
 */

#include <string.h>

#include "r_fps.h"

#include "smooth.h"

int demo_smoothturns;
int demo_smoothturnsfactor = 6;

static int smooth_playing_turns[SMOOTH_PLAYING_MAXFACTOR];
static int64_t smooth_playing_sum;
static int smooth_playing_index;
static angle_t smooth_playing_angle;

void R_SmoothPlaying_Reset(player_t* player)
{
	if(demo_smoothturns && demoplayback && !demorecording)
	{
		if(!player)
			player = &players[displayplayer];

		if(player == &players[displayplayer])
		{
			if(player->mo)
			{
				smooth_playing_angle = player->mo->angle;
				memset(smooth_playing_turns, 0, sizeof(smooth_playing_turns[0]) * SMOOTH_PLAYING_MAXFACTOR);
				smooth_playing_sum = 0;
				smooth_playing_index = 0;
			}
		}
	}
}

void R_SmoothPlaying_Add(int delta)
{
	if(demo_smoothturns && demoplayback && !demorecording)
	{
		smooth_playing_sum -= smooth_playing_turns[smooth_playing_index];
		smooth_playing_turns[smooth_playing_index] = delta;
		smooth_playing_index = (smooth_playing_index + 1) % (demo_smoothturnsfactor);
		smooth_playing_sum += delta;
		smooth_playing_angle += (int)(smooth_playing_sum / (demo_smoothturnsfactor));
	}
}

angle_t R_SmoothPlaying_Get(player_t* player)
{
	if(demo_smoothturns && demoplayback && !demorecording && player == &players[displayplayer])
		return smooth_playing_angle;
	else
		return player->mo->angle;
}

void R_ResetAfterTeleport(player_t* player)
{
	R_ResetViewInterpolation();
	R_SmoothPlaying_Reset(player);
}
