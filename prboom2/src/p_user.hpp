// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Player related stuff.
 *      Bobbing POV/weapon, movement.
 *      Pending weapon.
 */

#pragma once

#include "d_player.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

void P_PlayerThink(player_t* player);
void P_CalcHeight(player_t* player);
void P_DeathThink(player_t* player);
void P_MovePlayer(player_t* player);
void P_ForwardThrust(player_t* player, angle_t angle, fixed_t move);
void P_Thrust(player_t* player, angle_t angle, fixed_t move);

void P_SetPitch(player_t* player);

// heretic

int P_GetPlayerNum(player_t* player);
void P_PlayerRemoveArtifact(player_t* player, int slot);
void P_PlayerUseArtifact(player_t* player, ArtiType arti);
void P_PlayerNextArtifact(player_t* player);
dboolean P_UseArtifact(player_t* player, ArtiType arti);
void P_ChickenPlayerThink(player_t* player);
dboolean P_UndoPlayerChicken(player_t* player);
void Raven_P_MovePlayer(player_t* player);

// hexen

void ResetBlasted(mobj_t* mo);
void P_TeleportOther(mobj_t* victim);
dboolean P_UndoPlayerMorph(player_t* player);
void P_MorphPlayerThink(player_t* player);

void P_PlayerEndFlight(player_t* player);

#ifdef __cplusplus
}
#endif
