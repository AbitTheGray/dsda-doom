// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Player related stuff.
 *      Bobbing POV/weapon, movement.
 *      Pending weapon.
 */

#include <utility>

#include <math.h>

#include "doomstat.hpp"
#include "d_event.hpp"
#include "r_main.hpp"
#include "lprintf.hpp"
#include "p_map.hpp"
#include "p_maputl.hpp"
#include "p_enemy.hpp"
#include "p_spec.hpp"
#include "p_user.hpp"
#include "smooth.hpp"
#include "r_fps.hpp"
#include "g_game.hpp"
#include "p_tick.hpp"
#include "e6y.hpp"//e6y

#include "dsda/aim.hpp"
#include "dsda/death.hpp"
#include "dsda/excmd.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/settings.hpp"

// heretic needs
#include "heretic/def.hpp"
#include "s_sound.hpp"
#include "sounds.hpp"
#include "p_inter.hpp"
#include "m_random.hpp"

//
// Movement.
//

// 16 pixels of bob

#define MAXBOB  0x100000

dboolean onground; // whether player is on ground or in air
int offgroundtics; // how many tics player has been in air
#define AIRBOBFADETICS 4 // num tics to scale bobbing to 0 in midair

// heretic
int newtorch; // used in the torch flicker effect.
int newtorchdelta;

fixed_t P_PlayerSpeed(player_t* player)
{
	double vx, vy;

	vx = (double)player->mo->momx / FRACUNIT;
	vy = (double)player->mo->momy / FRACUNIT;

	return (fixed_t)(sqrt(vx * vx + vy * vy) * FRACUNIT);
}

//
// P_Thrust
// Moves the given origin along a given angle.
//

extern "C" void P_CompatiblePlayerThrust(player_t* player, angle_t angle, fixed_t move)
{
	player->mo->momx += FixedMul(move, finecosine[angle]);
	player->mo->momy += FixedMul(move, finesine[angle]);
}

extern "C" void P_HereticPlayerThrust(player_t* player, angle_t angle, fixed_t move)
{
	if(player->powers[std::to_underlying(PowerType::Flight)] && !(player->mo->z <= player->mo->floorz))
	{
		player->mo->momx += FixedMul(move, finecosine[angle]);
		player->mo->momy += FixedMul(move, finesine[angle]);
	}
	else if(player->mo->subsector->sector->special == 15)
	{
		player->mo->momx += FixedMul(move >> 2, finecosine[angle]);
		player->mo->momy += FixedMul(move >> 2, finesine[angle]);
	}
	else
	{
		player->mo->momx += FixedMul(move, finecosine[angle]);
		player->mo->momy += FixedMul(move, finesine[angle]);
	}
}

extern "C" void P_HexenPlayerThrust(player_t* player, angle_t angle, fixed_t move)
{
	if(player->powers[std::to_underlying(PowerType::Flight)] && !(player->mo->z <= player->mo->floorz))
	{
		player->mo->momx += FixedMul(move, finecosine[angle]);
		player->mo->momy += FixedMul(move, finesine[angle]);
	}
	else if(P_GetThingFloorType(player->mo) == FloorType::Ice) // Friction_Low
	{
		player->mo->momx += FixedMul(move >> 1, finecosine[angle]);
		player->mo->momy += FixedMul(move >> 1, finesine[angle]);
	}
	else
	{
		player->mo->momx += FixedMul(move, finecosine[angle]);
		player->mo->momy += FixedMul(move, finesine[angle]);
	}
}

// In doom, P_Thrust is always player-originated
// In heretic / hexen P_Thrust can come from effects
// Need to differentiate the two because of the flight cheat
void P_ForwardThrust(player_t* player, angle_t angle, fixed_t move)
{
	angle >>= ANGLETOFINESHIFT;

	if((player->mo->flags & MobjFlag::Fly) != MobjFlag{} && player->mo->pitch != 0)
	{
		angle_t pitch = player->mo->pitch >> ANGLETOFINESHIFT;
		fixed_t zpush = FixedMul(move, finesine[pitch]);
		player->mo->momz -= zpush;
		move = FixedMul(move, finecosine[pitch]);
	}

	map_format.player_thrust(player, angle, move);
}

void P_Thrust(player_t* player, angle_t angle, fixed_t move)
{
	angle >>= ANGLETOFINESHIFT;

	map_format.player_thrust(player, angle, move);
}

/*
 * P_Bob
 * Same as P_Thrust, but only affects bobbing.
 *
 * killough 10/98: We apply thrust separately between the real physical player
 * and the part which affects bobbing. This way, bobbing only comes from player
 * motion, nothing external, avoiding many problems, e.g. bobbing should not
 * occur on conveyors, unless the player walks on one, and bobbing should be
 * reduced at a regular rate, even on ice (where the player coasts).
 */

static void P_Bob(player_t* player, angle_t angle, fixed_t move)
{
	//e6y
	if(!mbf_features && !prboom_comp[std::to_underlying(PrboomComp::PrboomFriction)].state)
		return;

	player->momx += FixedMul(move, finecosine[angle >>= ANGLETOFINESHIFT]);
	player->momy += FixedMul(move, finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//

void P_CalcHeight(player_t* player)
{
	int angle;
	fixed_t bob, totalviewoffset;

	// Regular movement bobbing
	// (needs to be calculated for gun swing
	// even if not on ground)
	// OPTIMIZE: tablify angle
	// Note: a LUT allows for effects
	//  like a ramp with low health.


	/* killough 10/98: Make bobbing depend only on player-applied motion.
	*
	* Note: don't reduce bobbing here if on ice: if you reduce bobbing here,
	* it causes bobbing jerkiness when the player moves from ice to non-ice,
	* and vice-versa.
	*/

	player->bob = 0;

	if((player->mo->flags & MobjFlag::Fly) != MobjFlag{} && !onground)
	{
		player->bob = FRACUNIT / 2;
	}

	if(mbf_features)
	{
		if(player_bobbing)
		{
			player->bob = (FixedMul(player->momx, player->momx) +
				FixedMul(player->momy, player->momy)) >> 2;
		}
	}
	else
	{
		if(demo_compatibility || player_bobbing || prboom_comp[std::to_underlying(PrboomComp::ForceIncorrectBobbingInBoom)].state)
		{
			player->bob = (FixedMul(player->mo->momx, player->mo->momx) +
				FixedMul(player->mo->momy, player->mo->momy)) >> 2;
		}
	}

	//e6y
	if(!prboom_comp[std::to_underlying(PrboomComp::PrboomFriction)].state &&
		compatibility_level >= CompLevel::Boom202 &&
		compatibility_level <= CompLevel::Lxdoom1 &&
		player->mo->friction > ORIG_FRICTION) // ice?
	{
		if(player->bob > (MAXBOB >> 2))
			player->bob = MAXBOB >> 2;
	}
	else
	{
		if(player->bob > MAXBOB)
			player->bob = MAXBOB;
	}

	if((player->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{} && !onground)
	{
		player->bob = FRACUNIT / 2;
	}

	offgroundtics = onground ? 0 : offgroundtics + 1;

	if(!onground && !raven && (offgroundtics > AIRBOBFADETICS || !dsda_FixViewBobFloorJolt()))
	{
		player->viewz = player->mo->z + g_viewheight;

		if(player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
			player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

		return;
	}

	angle = (FINEANGLES / 20 * leveltime) & FINEMASK;
	bob = player->bob * dsda_ViewBob() / 4;
	bob = FixedMul(bob / 2, finesine[angle]);

	// move viewheight

	if(player->playerstate == PlayerState::Live && (onground || raven))
	{
		player->viewheight += player->deltaviewheight;

		if(player->viewheight > g_viewheight)
		{
			player->viewheight = g_viewheight;
			player->deltaviewheight = 0;
		}

		if(player->viewheight < g_viewheight / 2)
		{
			player->viewheight = g_viewheight / 2;
			if(player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}

		if(player->deltaviewheight)
		{
			player->deltaviewheight += FRACUNIT / 4;
			if(!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}

	if(player->chickenTics || player->morphTics)
	{
		player->viewz = player->mo->z + player->viewheight - (20 * FRACUNIT);
	}
	else
	{
		totalviewoffset = player->viewheight + bob - g_viewheight;

		if(!onground && !raven)
			totalviewoffset = totalviewoffset * (AIRBOBFADETICS + 1 - offgroundtics) / AIRBOBFADETICS;

		player->viewz = player->mo->z + g_viewheight + totalviewoffset;
	}

	if(player->playerstate != PlayerState::Dead && player->mo->z <= player->mo->floorz)
	{
		if(player->mo->floorclip)
			player->viewz -= player->mo->floorclip;
		else if((player->mo->flags2 & MobjFlag2::FeetAreClipped) != MobjFlag2{})
			player->viewz -= FOOTCLIPSIZE;
	}

	if(player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
		player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

	if(heretic && player->viewz < player->mo->floorz + 4 * FRACUNIT)
		player->viewz = player->mo->floorz + 4 * FRACUNIT;
}

//
// P_MovePlayer
//
// Adds momentum if the player is not in the air
//
// killough 10/98: simplified

void P_HandleExCmdLook(player_t* player)
{
	int look;

	look = player->cmd.ex.look;
	if(look)
	{
		if(look == XC_LOOK_RESET)
		{
			player->mo->pitch = 0;
		}
		else
		{
			player->mo->pitch += look << 16;
			CheckPitch((signed int*)&player->mo->pitch);
		}
	}
}

void P_MovePlayer(player_t* player)
{
	ticcmd_t* cmd;
	mobj_t* mo;

	P_HandleExCmdLook(player);

	if(raven) return Raven_P_MovePlayer(player);

	cmd = &player->cmd;
	mo = player->mo;
	mo->angle += cmd->angleturn << 16;

	if(demo_smoothturns && player == &players[displayplayer])
	{
		R_SmoothPlaying_Add(cmd->angleturn << 16);
	}

	onground = (mo->z <= mo->floorz || (mo->flags2 & MobjFlag2::OnMobj) != MobjFlag2{});

	if((player->mo->flags & MobjFlag::Fly) != MobjFlag{} && player == &players[consoleplayer] && upmove != 0)
	{
		mo->momz = upmove << 8;
	}

	// killough 10/98:
	//
	// We must apply thrust to the player and bobbing separately, to avoid
	// anomalies. The thrust applied to bobbing is always the same strength on
	// ice, because the player still "works just as hard" to move, while the
	// thrust applied to the movement varies with 'movefactor'.

	//e6y
	if((!demo_compatibility && !mbf_features && !prboom_comp[std::to_underlying(PrboomComp::PrboomFriction)].state) ||
		(cmd->forwardmove | cmd->sidemove)) // killough 10/98
	{
		if(onground || (mo->flags & MobjFlag::Bounces) != MobjFlag{} || (mo->flags & MobjFlag::Fly) != MobjFlag{}) // killough 8/9/98
		{
			int friction, movefactor = P_GetMoveFactor(mo, &friction);

			// killough 11/98:
			// On sludge, make bobbing depend on efficiency.
			// On ice, make it depend on effort.

			int bobfactor =
				friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR;

			if(map_format.zdoom && !movefactor)
				bobfactor = movefactor;

			if(cmd->forwardmove)
			{
				P_Bob(player, mo->angle, cmd->forwardmove * bobfactor);
				P_ForwardThrust(player, mo->angle, cmd->forwardmove * movefactor);
			}

			if(cmd->sidemove)
			{
				P_Bob(player, mo->angle - ANG90, cmd->sidemove * bobfactor);
				P_Thrust(player, mo->angle - ANG90, cmd->sidemove * movefactor);
			}
		}
		else if(map_aircontrol)
		{
			int friction, movefactor = P_GetMoveFactor(mo, &friction);

			movefactor = FixedMul(movefactor, map_aircontrol);

			if(cmd->forwardmove)
			{
				P_Bob(player, mo->angle, cmd->forwardmove * movefactor);
				P_Thrust(player, player->mo->angle, cmd->forwardmove * movefactor);
			}

			if(cmd->sidemove)
			{
				P_Bob(player, mo->angle - ANG90, cmd->sidemove * movefactor);
				P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * movefactor);
			}
		}
		if(mo->state == states + std::to_underlying(StateId::Play))
			P_SetMobjState(mo, StateId::PlayRun1);
	}
}

#define ANG5 (ANG90/18)

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//

void P_DeathThink(player_t* player)
{
	angle_t angle;
	angle_t delta;

	P_MovePsprites(player);

	// fall to the ground

	onground = (player->mo->z <= player->mo->floorz);
	if(player->mo->type == g_skullpop_mt || (hexen && player->mo->type == MobjType::HexenIcechunk))
	{
		// Flying bloody skull
		player->viewheight = 6 * FRACUNIT;
		player->deltaviewheight = 0;
		if(onground)
		{
			if(raven && !dsda_FreeAim())
			{
				if(player->lookdir < 60)
				{
					int lookDelta;

					lookDelta = (60 - player->lookdir) / 8;
					if(lookDelta < 1 && (leveltime & 1))
					{
						lookDelta = 1;
					}
					else if(lookDelta > 6)
					{
						lookDelta = 6;
					}
					player->lookdir += lookDelta;
				}
			}
			else if((int)player->mo->pitch > -(int)ANG1 * 19)
			{
				player->mo->pitch -= ((int)ANG1 * 19 - player->mo->pitch) / 8;
			}
		}
	}
	else if((player->mo->flags2 & MobjFlag2::IceDamage) == MobjFlag2{})
	{
		if(player->viewheight > 6 * FRACUNIT)
			player->viewheight -= FRACUNIT;

		if(player->viewheight < 6 * FRACUNIT)
			player->viewheight = 6 * FRACUNIT;

		player->deltaviewheight = 0;

		if(dsda_FreeAim())
		{
			const int delta = dsda_LookDirToPitch(-6);

			if((int)player->mo->pitch > 0)
			{
				player->mo->pitch -= delta;
			}
			else if((int)player->mo->pitch < 0)
			{
				player->mo->pitch += delta;
			}

			if(abs((int)player->mo->pitch) < delta)
			{
				player->mo->pitch = 0;
			}
		}
		else
		{
			if(player->lookdir > 0)
			{
				player->lookdir -= 6;
			}
			else if(player->lookdir < 0)
			{
				player->lookdir += 6;
			}
			if(abs(player->lookdir) < 6)
			{
				player->lookdir = 0;
			}
		}
	}

	P_CalcHeight(player);

	if(player->attacker && player->attacker != player->mo)
	{
		if(hexen)
		{
			int dir = P_FaceMobj(player->mo, player->attacker, &delta);
			if(delta < ANG1 * 10)
			{
				// Looking at killer, so fade damage and poison counters
				if(player->damagecount)
				{
					player->damagecount--;
				}
				if(player->poisoncount)
				{
					player->poisoncount--;
				}
			}
			delta = delta / 8;
			if(delta > ANG1 * 5)
			{
				delta = ANG1 * 5;
			}
			if(dir)
			{
				// Turn clockwise
				player->mo->angle += delta;
			}
			else
			{
				// Turn counter clockwise
				player->mo->angle -= delta;
			}
		}
		else
		{
			angle = R_PointToAngle2(player->mo->x,
				player->mo->y,
				player->attacker->x,
				player->attacker->y);

			delta = angle - player->mo->angle;

			if(delta < ANG5 || delta > (unsigned)-ANG5)
			{
				// Looking at killer,
				//  so fade damage flash down.

				player->mo->angle = angle;

				if(player->damagecount)
					player->damagecount--;
			}
			else if(delta < ANG180)
				player->mo->angle += ANG5;
			else
				player->mo->angle -= ANG5;
		}
	}
	else if(player->damagecount || player->poisoncount)
	{
		if(player->damagecount)
			player->damagecount--;
		else
			player->poisoncount--;
	}

	if((player->cmd.buttons & ButtonCode::Use) != ButtonCode{})
	{
		dsda_DeathUse(player);
	}

	R_SmoothPlaying_Reset(player); // e6y
}

void P_PlayerEndFlight(player_t* player)
{
	if(player->mo->z != player->mo->floorz)
	{
		player->centering = true;
	}

	player->mo->flags2 -= MobjFlag2::Fly;
	player->mo->flags -= MobjFlag::NoGravity;
}

//
// P_PlayerThink
//

extern "C" void P_MorphPlayerThink(player_t* player);

void P_PlayerThink(player_t* player)
{
	ticcmd_t* cmd;
	WeaponType newweapon;
	FloorType floorType;

	if(movement_smooth)
	{
		player->prev_viewz = player->viewz;
		player->prev_viewangle = R_SmoothPlaying_Get(player);
		player->prev_viewpitch = dsda_PlayerPitch(player);

		if(&players[displayplayer] == player)
		{
			P_ResetWalkcam();
		}
	}

	// killough 2/8/98, 3/21/98:
	if(player->cheats & CF_NOCLIP)
		player->mo->flags |= MobjFlag::NoClip;
	else
		player->mo->flags -= MobjFlag::NoClip;

	// chain saw run forward

	cmd = &player->cmd;
	if((player->mo->flags & MobjFlag::JustAttacked) != MobjFlag{})
	{
		cmd->angleturn = 0;
		cmd->forwardmove = 0xc800 / 512;
		cmd->sidemove = 0;
		player->mo->flags -= MobjFlag::JustAttacked;
	}

	if(hexen)
		player->worldTimer++;

	if(player->playerstate == PlayerState::Dead)
	{
		P_DeathThink(player);
		return;
	}

	if(player->chickenTics)
	{
		P_ChickenPlayerThink(player);
	}

	if(player->jumpTics)
	{
		player->jumpTics--;
	}

	if(player->morphTics)
	{
		P_MorphPlayerThink(player);
	}

	// Move around.
	// Reactiontime is used to prevent movement
	//  for a bit after a teleport.

	if(player->mo->reactiontime)
		player->mo->reactiontime--;
	else
	{
		P_MovePlayer(player);

		if(hexen)
		{
			mobj_t* pmo = player->mo;
			if(player->powers[std::to_underlying(PowerType::Speed)] && !(leveltime & 1)
				&& P_AproxDistance(pmo->momx, pmo->momy) > 12 * FRACUNIT)
			{
				mobj_t* speedMo;
				int playerNum;

				speedMo = P_SpawnMobj(pmo->x, pmo->y, pmo->z, MobjType::HexenPlayerSpeed);
				if(speedMo)
				{
					speedMo->angle = pmo->angle;
					playerNum = P_GetPlayerNum(player);
					if(player->pclass == PClass::Fighter)
					{
						// The first type should be blue, and the
						// third should be the Fighter's original gold color
						if(playerNum == 0)
						{
							speedMo->flags |= MobjTranslationFlags(2u);
						}
						else if(playerNum != 2)
						{
							speedMo->flags |= MobjTranslationFlags(playerNum);
						}
					}
					else if(playerNum)
					{
						// Set color translation bits for player sprites
						speedMo->flags |= MobjTranslationFlags(playerNum);
					}
					P_SetTarget(&speedMo->target, pmo);
					speedMo->special1.i = std::to_underlying(player->pclass);
					if(speedMo->special1.i > 2)
					{
						speedMo->special1.i = 0;
					}
					speedMo->sprite = pmo->sprite;
					speedMo->floorclip = pmo->floorclip;
					if(player == &players[consoleplayer])
					{
						speedMo->flags2 |= MobjFlag2::DontDraw;
					}
				}
			}
		}
	}

	P_CalcHeight(player); // Determines view height and bobbing

	// Determine if there's anything about the sector you're in that's
	// going to affect you, like painful floors.

	if(P_IsSpecialSector(player->mo->subsector->sector))
		P_PlayerInSpecialSector(player);

	if(hexen)
	{
		if((floorType = P_GetThingFloorType(player->mo)) != FloorType::Solid)
		{
			P_PlayerOnSpecialFlat(player, floorType);
		}

		switch(player->pclass)
		{
			case PClass::Fighter:
				if(player->mo->momz <= -35 * FRACUNIT
					&& player->mo->momz >= -40 * FRACUNIT && !player->morphTics
					&& !S_GetSoundPlayingInfo(player->mo,
						SfxId::HexenPlayerFighterFallingScream))
				{
					S_StartMobjSound(player->mo, SfxId::HexenPlayerFighterFallingScream);
				}
				break;
			case PClass::Cleric:
				if(player->mo->momz <= -35 * FRACUNIT
					&& player->mo->momz >= -40 * FRACUNIT && !player->morphTics
					&& !S_GetSoundPlayingInfo(player->mo,
						SfxId::HexenPlayerClericFallingScream))
				{
					S_StartMobjSound(player->mo, SfxId::HexenPlayerClericFallingScream);
				}
				break;
			case PClass::Mage:
				if(player->mo->momz <= -35 * FRACUNIT
					&& player->mo->momz >= -40 * FRACUNIT && !player->morphTics
					&& !S_GetSoundPlayingInfo(player->mo,
						SfxId::HexenPlayerMageFallingScream))
				{
					S_StartMobjSound(player->mo, SfxId::HexenPlayerMageFallingScream);
				}
				break;
			default:
				break;
		}

		if(cmd->arti)
		{
			// Use an artifact
			if((cmd->arti & AFLAG_JUMP) && onground && !player->jumpTics)
			{
				if(player->morphTics)
				{
					player->mo->momz = 6 * FRACUNIT;
				}
				else
				{
					player->mo->momz = 9 * FRACUNIT;
				}
				player->mo->flags2 -= MobjFlag2::OnMobj;
				player->jumpTics = 18;
			}
			else if(cmd->arti & AFLAG_SUICIDE)
			{
				P_DamageMobj(player->mo, nullptr, nullptr, 10000);
			}
			if(cmd->arti == std::to_underlying(ArtiType::HexenCount))
			{
				// use one of each artifact (except puzzle artifacts)
				int i;

				for(i = 1; i < std::to_underlying(ArtiType::HexenFirstpuzzitem); i++)
				{
					P_PlayerUseArtifact(player, static_cast<ArtiType>(i));
				}
			}
			else
			{
				P_PlayerUseArtifact(player, static_cast<ArtiType>(cmd->arti & AFLAG_MASK));
			}
		}
	}
	else
	{
		if(cmd->arti)
		{
			// Use an artifact
			if(cmd->arti == 0xff)
			{
				P_PlayerNextArtifact(player);
			}
			else
			{
				P_PlayerUseArtifact(player, static_cast<ArtiType>(cmd->arti));
			}
		}
	}

	if(dsda_AllowExCmd())
	{
		if(cmd->ex.actions & XC_JUMP && onground && !player->jumpTics)
		{
			player->mo->momz = g_jump * FRACUNIT;
			player->mo->flags2 -= MobjFlag2::OnMobj;
			player->jumpTics = 18;
		}
	}

	if(raven && (cmd->buttons & ButtonCode::Special) != ButtonCode{})
	{
		cmd->buttons = static_cast<ButtonCode>(0);
	}

	// Check for weapon change.
	if((cmd->buttons & ButtonCode::Change) != ButtonCode{} && !player->morphTics)
	{
		// The actual changing of the weapon is done
		//  when the weapon psprite can do it
		//  (read: not in the middle of an attack).

		newweapon = static_cast<WeaponType>(ButtonWeapon(cmd->buttons));

		// killough 3/22/98: For demo compatibility we must perform the fist
		// and SSG weapons switches here, rather than in G_BuildTiccmd(). For
		// other games which rely on user preferences, we must use the latter.

		if(demo_compatibility)
		{
			// compatibility mode -- required for old demos -- killough
			//e6y
			if(!prboom_comp[std::to_underlying(PrboomComp::AllowSsgDirect)].state)
				newweapon = static_cast<WeaponType>((std::to_underlying(cmd->buttons) & std::to_underlying(ButtonCode::WeaponMaskOld)) >> std::to_underlying(ButtonCode::WeaponShift));

			if(!hexen)
			{
				if(
					newweapon == static_cast<WeaponType>(g_wp_fist) && player->weaponowned[g_wp_chainsaw]
					&& (
						player->readyweapon != static_cast<WeaponType>(g_wp_chainsaw) ||
						(!heretic && !player->powers[std::to_underlying(PowerType::Strength)])
					)
				)
					newweapon = static_cast<WeaponType>(g_wp_chainsaw);

				if(!heretic &&
					gamemode == GameMode::Commercial &&
					newweapon == WeaponType::Shotgun &&
					player->weaponowned[std::to_underlying(WeaponType::Supershotgun)] &&
					player->readyweapon != WeaponType::Supershotgun)
					newweapon = WeaponType::Supershotgun;
			}
		}

		// killough 2/8/98, 3/22/98 -- end of weapon selection changes

		if(player->weaponowned[std::to_underlying(newweapon)] && newweapon != player->readyweapon)

			// Do not go to plasma or BFG in shareware,
			//  even if cheated.

			// heretic_note: ignoring this...not sure it's worth worrying about
			if((newweapon != WeaponType::Plasma && newweapon != WeaponType::Bfg)
				|| (gamemode != GameMode::Shareware))
				player->pendingweapon = newweapon;
	}

	// check for use

	if((cmd->buttons & ButtonCode::Use) != ButtonCode{})
	{
		if(!player->usedown)
		{
			P_UseLines(player);
			player->usedown = true;
		}
	}
	else
		player->usedown = false;

	// Chicken counter
	if(player->chickenTics)
	{
		if(player->chickenPeck)
		{
			// Chicken attack counter
			player->chickenPeck -= 3;
		}
		if(!--player->chickenTics)
		{
			// Attempt to undo the chicken
			P_UndoPlayerChicken(player);
		}
	}

	// Morph counter
	if(player->morphTics)
	{
		if(!--player->morphTics)
		{
			// Attempt to undo the pig
			P_UndoPlayerMorph(player);
		}
	}

	// cycle psprites
	P_MovePsprites(player);

	// Counters, time dependent power ups.

	// Strength counts up to diminish fade.

	if(player->powers[std::to_underlying(PowerType::Strength)])
		player->powers[std::to_underlying(PowerType::Strength)]++;

	// killough 1/98: Make idbeholdx toggle:

	if(player->powers[std::to_underlying(PowerType::Invulnerability)] > 0) // killough
	{
		if(player->pclass == PClass::Cleric)
		{
			if(!(leveltime & 7) && (player->mo->flags & MobjFlag::Shadow) != MobjFlag{}
				&& (player->mo->flags2 & MobjFlag2::DontDraw) == MobjFlag2{})
			{
				player->mo->flags -= MobjFlag::Shadow;
				if((player->mo->flags & MobjFlag::AltShadow) == MobjFlag{})
				{
					player->mo->flags2 |= MobjFlag2::DontDraw | MobjFlag2::NonShootable;
				}
			}
			if(!(leveltime & 31))
			{
				if((player->mo->flags2 & MobjFlag2::DontDraw) != MobjFlag2{})
				{
					if((player->mo->flags & MobjFlag::Shadow) == MobjFlag{})
					{
						player->mo->flags |= MobjFlag::Shadow | MobjFlag::AltShadow;
					}
					else
					{
						player->mo->flags2 -= (MobjFlag2::DontDraw | MobjFlag2::NonShootable);
					}
				}
				else
				{
					player->mo->flags |= MobjFlag::Shadow;
					player->mo->flags -= MobjFlag::AltShadow;
				}
			}
		}

		if(!(--player->powers[std::to_underlying(PowerType::Invulnerability)]))
		{
			player->mo->flags2 -= (MobjFlag2::Invulnerable | MobjFlag2::Reflective);
			if(player->pclass == PClass::Cleric)
			{
				player->mo->flags2 -= (MobjFlag2::DontDraw | MobjFlag2::NonShootable);
				player->mo->flags -= (MobjFlag::Shadow | MobjFlag::AltShadow);
			}
		}
	}

	if(player->powers[std::to_underlying(PowerType::Minotaur)])
		player->powers[std::to_underlying(PowerType::Minotaur)]--;

	if(player->powers[std::to_underlying(PowerType::Speed)])
		player->powers[std::to_underlying(PowerType::Speed)]--;

	if(player->powers[std::to_underlying(PowerType::Invisibility)] > 0) // killough
		if(!--player->powers[std::to_underlying(PowerType::Invisibility)])
			player->mo->flags -= MobjFlag::Shadow;

	if(player->powers[std::to_underlying(PowerType::Infrared)] > 0) // killough
		player->powers[std::to_underlying(PowerType::Infrared)]--;

	if(player->powers[std::to_underlying(PowerType::IronFeet)] > 0) // killough
		player->powers[std::to_underlying(PowerType::IronFeet)]--;

	if(player->powers[std::to_underlying(PowerType::Flight)] && (!hexen || netgame))
	{
		if(!--player->powers[std::to_underlying(PowerType::Flight)])
		{
			P_PlayerEndFlight(player);
		}
	}

	if(player->powers[std::to_underlying(PowerType::WeaponLevel2)])
	{
		if(!--player->powers[std::to_underlying(PowerType::WeaponLevel2)])
		{
			if((player->readyweapon == WeaponType::PhoenixRod)
				&& (player->psprites[std::to_underlying(PspNum::Weapon)].state
					!= &states[std::to_underlying(StateId::HereticPhoenixready)])
				&& (player->psprites[std::to_underlying(PspNum::Weapon)].state
					!= &states[std::to_underlying(StateId::HereticPhoenixup)]))
			{
				P_SetPsprite(player, PspNum::Weapon, StateId::HereticPhoenixready);
				player->ammo[std::to_underlying(AmmoType::PhoenixRod)] -= USE_PHRD_AMMO_2;
				player->refire = 0;
			}
			else if((player->readyweapon == WeaponType::Gauntlets)
				|| (player->readyweapon == WeaponType::Staff))
			{
				player->pendingweapon = player->readyweapon;
			}
		}
	}

	if(player->damagecount)
		player->damagecount--;

	if(player->bonuscount)
		player->bonuscount--;

	if(player->hazardcount)
	{
		player->hazardcount--;
		if(!(leveltime % player->hazardinterval) && player->hazardcount > 16 * TICRATE)
			P_DamageMobj(player->mo, nullptr, nullptr, 5);
	}

	if(player->poisoncount && !(leveltime & 15))
	{
		player->poisoncount -= 5;
		if(player->poisoncount < 0)
		{
			player->poisoncount = 0;
		}
		P_PoisonDamage(player, player->poisoner, 1, true);
	}

	// Handling colormaps.
	// killough 3/20/98: reformat to terse C syntax
	if(!raven)
		player->fixedcolormap = dsda_PowerPalette() &&
			(player->powers[std::to_underlying(PowerType::Invulnerability)] > 4 * 32 ||
				player->powers[std::to_underlying(PowerType::Invulnerability)] & 8)
			? INVERSECOLORMAP
			: player->powers[std::to_underlying(PowerType::Infrared)] > 4 * 32 || player->powers[std::to_underlying(PowerType::Infrared)] & 8;
	else
	{
		if(!hexen && player->powers[std::to_underlying(PowerType::Invulnerability)])
		{
			if(player->powers[std::to_underlying(PowerType::Invulnerability)] > BLINKTHRESHOLD
				|| (player->powers[std::to_underlying(PowerType::Invulnerability)] & 8))
			{
				player->fixedcolormap = INVERSECOLORMAP;
			}
			else
			{
				player->fixedcolormap = 0;
			}
		}
		else if(player->powers[std::to_underlying(PowerType::Infrared)])
		{
			if(player->powers[std::to_underlying(PowerType::Infrared)] <= BLINKTHRESHOLD)
			{
				if(player->powers[std::to_underlying(PowerType::Infrared)] & 8)
				{
					player->fixedcolormap = 0;
				}
				else
				{
					player->fixedcolormap = 1;
				}
			}
			else if(!(leveltime & 16) && player == &players[consoleplayer])
			{
				if(newtorch)
				{
					if(player->fixedcolormap + newtorchdelta > 7
						|| player->fixedcolormap + newtorchdelta < 1
						|| newtorch == player->fixedcolormap)
					{
						newtorch = 0;
					}
					else
					{
						player->fixedcolormap += newtorchdelta;
					}
				}
				else
				{
					newtorch = (M_Random() & 7) + 1;
					newtorchdelta = (newtorch == player->fixedcolormap) ? 0 : ((newtorch > player->fixedcolormap) ? 1 : -1);
				}
			}
		}
		else
		{
			player->fixedcolormap = 0;
		}
	}
}

// heretic

#include "p_tick.hpp"

extern "C" void P_PlayerNextArtifact(player_t* player);

int P_GetPlayerNum(player_t* player)
{
	int i;

	for(i = 0; i < g_maxplayers; i++)
	{
		if(player == &players[i])
		{
			return (i);
		}
	}
	return (0);
}

dboolean P_UndoPlayerChicken(player_t* player)
{
	mobj_t* fog;
	mobj_t* mo;
	mobj_t* pmo;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	angle_t angle;
	int playerNum;
	WeaponType weapon;
	int oldFlags; // not MobjFlag: upstream truncates to int, see below
	int oldFlags2; // not MobjFlag2: upstream truncates to int, see below

	pmo = player->mo;
	x = pmo->x;
	y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	weapon = static_cast<WeaponType>(pmo->special1.i);
	// Truncates on purpose, as upstream: flags is 64 bits but upstream keeps it in an int,
	// so bits 32-63 are lost if a failed unmorph writes it back below.
	oldFlags = static_cast<int>(std::to_underlying(pmo->flags));
	// Truncates on purpose, as upstream: flags2 is 64 bits but upstream keeps it in an int,
	// so bits 32-63 are lost if a failed unmorph writes it back below.
	oldFlags2 = static_cast<int>(std::to_underlying(pmo->flags2));
	P_SetMobjState(pmo, StateId::HereticFreetargmobj);
	mo = P_SpawnMobj(x, y, z, static_cast<MobjType>(g_mt_player));
	if(P_TestMobjLocation(mo) == false)
	{
		// Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, MobjType::HereticChicplayer);
		mo->angle = angle;
		mo->health = player->health;
		mo->special1.i = std::to_underlying(weapon);
		mo->player = player;
		// The truncated copy: the original bits 32-63 are gone, replaced by copies of bit 31
		mo->flags = static_cast<MobjFlag>(static_cast<uint64_t>(oldFlags));
		// The truncated copy: the original bits 32-63 are gone, replaced by copies of bit 31
		mo->flags2 = static_cast<MobjFlag2>(static_cast<uint64_t>(oldFlags2));
		player->mo = mo;
		player->chickenTics = 2 * TICRATE;
		return (false);
	}
	playerNum = P_GetPlayerNum(player);
	if(playerNum != 0)
	{
		// Set color translation
		mo->flags |= MobjTranslationFlags(playerNum);
	}
	mo->angle = angle;
	mo->player = player;
	mo->reactiontime = 18;
	if(oldFlags2 & std::to_underlying(MobjFlag2::Fly))
	{
		mo->flags2 |= MobjFlag2::Fly;
		mo->flags |= MobjFlag::NoGravity;
	}
	player->chickenTics = 0;
	player->powers[std::to_underlying(PowerType::WeaponLevel2)] = 0;
	player->health = mo->health = MAXHEALTH;
	player->mo = mo;
	angle >>= ANGLETOFINESHIFT;
	fog = P_SpawnMobj(x + 20 * finecosine[angle],
		y + 20 * finesine[angle], z + TELEFOGHEIGHT, MobjType::HereticTfog);
	S_StartMobjSound(fog, SfxId::HereticTelept);
	P_PostChickenWeapon(player, weapon);
	return (true);
}

void P_ArtiTele(player_t* player)
{
	int i;
	int selections;
	fixed_t destX;
	fixed_t destY;
	angle_t destAngle;

	if(deathmatch)
	{
		selections = deathmatch_p - deathmatchstarts;
		i = P_Random(RandomClass::Heretic) % selections;
		destX = deathmatchstarts[i].x;
		destY = deathmatchstarts[i].y;
		destAngle = ANG45 * (deathmatchstarts[i].angle / 45);
	}
	else
	{
		destX = playerstarts[0][0].x;
		destY = playerstarts[0][0].y;
		destAngle = ANG45 * (playerstarts[0][0].angle / 45);
	}
	P_Teleport(player->mo, destX, destY, destAngle, true);
	if(player->morphTics)
	{
		// Teleporting away will undo any morph effects (pig)
		P_UndoPlayerMorph(player);
	}
	if(heretic)
		S_StartVoidSound(SfxId::HereticWpnup); // Full volume laugh
}

void P_PlayerNextArtifact(player_t* player)
{
	if(player == &players[consoleplayer])
	{
		inv_ptr--;
		if(inv_ptr < 6)
		{
			curpos--;
			if(curpos < 0)
			{
				curpos = 0;
			}
		}
		if(inv_ptr < 0)
		{
			inv_ptr = player->inventorySlotNum - 1;
			if(inv_ptr < 6)
			{
				curpos = inv_ptr;
			}
			else
			{
				curpos = 6;
			}
		}
		player->readyArtifact = static_cast<ArtiType>(player->inventory[inv_ptr].type);
	}
}

void P_PlayerRemoveArtifact(player_t* player, int slot)
{
	int i;
	player->artifactCount--;
	if(!(--player->inventory[slot].count))
	{
		// Used last of a type - compact the artifact list
		player->readyArtifact = ArtiType::None;
		player->inventory[slot].type = std::to_underlying(ArtiType::None);
		for(i = slot + 1; i < player->inventorySlotNum; i++)
		{
			player->inventory[i - 1] = player->inventory[i];
		}
		player->inventorySlotNum--;
		if(player == &players[consoleplayer])
		{
			// Set position markers and get next readyArtifact
			inv_ptr--;
			if(inv_ptr < 6)
			{
				curpos--;
				if(curpos < 0)
				{
					curpos = 0;
				}
			}
			if(inv_ptr >= player->inventorySlotNum)
			{
				inv_ptr = player->inventorySlotNum - 1;
			}
			if(inv_ptr < 0)
			{
				inv_ptr = 0;
			}
			player->readyArtifact = static_cast<ArtiType>(player->inventory[inv_ptr].type);
		}
	}
}

void P_PlayerUseArtifact(player_t* player, ArtiType arti)
{
	int i;

	for(i = 0; i < player->inventorySlotNum; i++)
	{
		if(player->inventory[i].type == std::to_underlying(arti))
		{
			// Found match - try to use
			if(P_UseArtifact(player, arti))
			{
				// Artifact was used - remove it from inventory
				P_PlayerRemoveArtifact(player, i);
				if(player == &players[consoleplayer])
				{
					if(hexen)
					{
						if(arti < ArtiType::HexenFirstpuzzitem)
						{
							S_StartVoidSound(SfxId::HexenArtifactUse);
						}
						else
						{
							S_StartVoidSound(SfxId::HexenPuzzleSuccess);
						}
					}
					else
					{
						S_StartVoidSound(SfxId::HereticArtiuse);
					}
					ArtifactFlash = 4;
				}
			}
			else if(!hexen || arti < ArtiType::HexenFirstpuzzitem)
			{
				// Unable to use artifact, advance pointer
				P_PlayerNextArtifact(player);
			}
			break;
		}
	}
}

static dboolean Hexen_P_UseArtifact(player_t* player, ArtiType arti);

dboolean P_UseArtifact(player_t* player, ArtiType arti)
{
	mobj_t* mo;
	angle_t angle;

	if(hexen) return Hexen_P_UseArtifact(player, arti);

	switch(arti)
	{
		case ArtiType::Invulnerability:
			if(!P_GivePower(player, PowerType::Invulnerability))
			{
				return (false);
			}
			break;
		case ArtiType::Invisibility:
			if(!P_GivePower(player, PowerType::Invisibility))
			{
				return (false);
			}
			break;
		case ArtiType::Health:
			if(!P_GiveBody(player, 25))
			{
				return (false);
			}
			break;
		case ArtiType::SuperHealth:
			if(!P_GiveBody(player, 100))
			{
				return (false);
			}
			break;
		case ArtiType::TomeOfPower:
			if(player->chickenTics)
			{
				// Attempt to undo chicken
				if(P_UndoPlayerChicken(player) == false)
				{
					// Failed
					P_DamageMobj(player->mo, nullptr, nullptr, 10000);
				}
				else
				{
					// Succeeded
					player->chickenTics = 0;
					S_StartMobjSound(player->mo, SfxId::HereticWpnup);
				}
			}
			else
			{
				if(!P_GivePower(player, PowerType::WeaponLevel2))
				{
					return (false);
				}
				if(player->readyweapon == WeaponType::Staff)
				{
					P_SetPsprite(player, PspNum::Weapon, StateId::HereticStaffready21);
				}
				else if(player->readyweapon == WeaponType::Gauntlets)
				{
					P_SetPsprite(player, PspNum::Weapon, StateId::HereticGauntletready21);
				}
			}
			break;
		case ArtiType::Torch:
			if(!P_GivePower(player, PowerType::Infrared))
			{
				return (false);
			}
			break;
		case ArtiType::Firebomb:
			angle = player->mo->angle >> ANGLETOFINESHIFT;

			// Vanilla bug here:
			// Original code here looks like:
			//   (player->mo->flags2 & MF2_FEETARECLIPPED != 0),
			// Which under C's operator precedence is:
			//   (player->mo->flags2 & (MF2_FEETARECLIPPED != 0)),
			// Which simplifies to:
			//   (player->mo->flags2 & 1),
			// Bit 0 is MobjFlag2::LoGrav.
			mo = P_SpawnMobj(player->mo->x + 24 * finecosine[angle],
				player->mo->y + 24 * finesine[angle],
				player->mo->z -
				15 * FRACUNIT * std::to_underlying(player->mo->flags2 & MobjFlag2::LoGrav),
				MobjType::HereticFirebomb);
			P_SetTarget(&mo->target, player->mo);
			break;
		case ArtiType::Egg:
			mo = player->mo;
			P_SpawnPlayerMissile(mo, MobjType::HereticEggfx);
			P_SPMAngle(mo, MobjType::HereticEggfx, mo->angle - (ANG45 / 6));
			P_SPMAngle(mo, MobjType::HereticEggfx, mo->angle + (ANG45 / 6));
			P_SPMAngle(mo, MobjType::HereticEggfx, mo->angle - (ANG45 / 3));
			P_SPMAngle(mo, MobjType::HereticEggfx, mo->angle + (ANG45 / 3));
			break;
		case ArtiType::Fly:
			if(!P_GivePower(player, PowerType::Flight))
			{
				return (false);
			}
			break;
		case ArtiType::Teleport:
			P_ArtiTele(player);
			break;
		default:
			return (false);
	}
	return (true);
}

void Raven_P_MovePlayer(player_t* player)
{
	int look;
	int fly;
	ticcmd_t* cmd;

	cmd = &player->cmd;
	player->mo->angle += (cmd->angleturn << 16);

	if(demo_smoothturns && player == &players[displayplayer])
	{
		R_SmoothPlaying_Add(cmd->angleturn << 16);
	}

	onground = (player->mo->z <= player->mo->floorz
		|| (player->mo->flags2 & MobjFlag2::OnMobj) != MobjFlag2{});

	if(player->chickenTics)
	{
		// Chicken speed
		if(cmd->forwardmove && (onground || (player->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{}))
			P_ForwardThrust(player, player->mo->angle, cmd->forwardmove * 2500);
		if(cmd->sidemove && (onground || (player->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{}))
			P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2500);
	}
	else
	{
		// Normal speed
		if(cmd->forwardmove)
		{
			if(onground || (player->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{})
				P_ForwardThrust(player, player->mo->angle, cmd->forwardmove * 2048);
			else if(hexen)
				P_ForwardThrust(player, player->mo->angle, map_aircontrol);
		}

		if(cmd->sidemove)
		{
			if(onground || (player->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{})
				P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2048);
			else if(hexen)
				P_Thrust(player, player->mo->angle, map_aircontrol);
		}
	}

	if(cmd->forwardmove || cmd->sidemove)
	{
		if(player->chickenTics)
		{
			if(player->mo->state == &states[std::to_underlying(StateId::HereticChicplay)])
			{
				P_SetMobjState(player->mo, StateId::HereticChicplayRun1);
			}
		}
		else
		{
			if(player->mo->state == &states[std::to_underlying(pclass[player->pclass].normal_state)])
			{
				P_SetMobjState(player->mo, static_cast<StateId>(pclass[player->pclass].run_state));
			}
		}
	}

	look = cmd->lookfly & 15;
	if(look > 7)
	{
		look -= 16;
	}
	if(look)
	{
		if(look == TOCENTER)
		{
			player->centering = true;
		}
		else
		{
			player->lookdir += 5 * look;
			if(player->lookdir > 90 || player->lookdir < -110)
			{
				player->lookdir -= 5 * look;
			}
		}
	}
	if(player->centering)
	{
		if(player->lookdir > 0)
		{
			player->lookdir -= 8;
		}
		else if(player->lookdir < 0)
		{
			player->lookdir += 8;
		}
		if(abs(player->lookdir) < 8)
		{
			player->lookdir = 0;
			player->centering = false;
		}
	}
	fly = cmd->lookfly >> 4;
	if(fly > 7)
	{
		fly -= 16;
	}
	if(fly && player->powers[std::to_underlying(PowerType::Flight)])
	{
		if(fly != TOCENTER)
		{
			player->flyheight = fly * 2;
			if((player->mo->flags2 & MobjFlag2::Fly) == MobjFlag2{})
			{
				player->mo->flags2 |= MobjFlag2::Fly;
				player->mo->flags |= MobjFlag::NoGravity;
				if(hexen && player->mo->momz <= -39 * FRACUNIT)
				{
					// stop falling scream
					S_StopSound(player->mo);
				}
			}
		}
		else
		{
			player->mo->flags2 -= MobjFlag2::Fly;
			player->mo->flags -= MobjFlag::NoGravity;
		}
	}
	else if(fly > 0)
	{
		P_PlayerUseArtifact(player, static_cast<ArtiType>(g_arti_fly));
	}
	if((player->mo->flags2 & MobjFlag2::Fly) != MobjFlag2{})
	{
		player->mo->momz = player->flyheight * FRACUNIT;
		if(player->flyheight)
		{
			player->flyheight /= 2;
		}
	}
}

void P_ChickenPlayerThink(player_t* player)
{
	mobj_t* pmo;

	if(player->health > 0)
	{
		// Handle beak movement
		P_UpdateBeak(player, &player->psprites[std::to_underlying(PspNum::Weapon)]);
	}
	if(player->chickenTics & 15)
	{
		return;
	}
	pmo = player->mo;
	if(!(pmo->momx + pmo->momy) && P_Random(RandomClass::Heretic) < 160)
	{
		// Twitch view angle
		pmo->angle += P_SubRandom() << 19;
	}
	if((pmo->z <= pmo->floorz) && (P_Random(RandomClass::Heretic) < 32))
	{
		// Jump and noise
		pmo->momz += FRACUNIT;
		P_SetMobjState(pmo, StateId::HereticChicplayPain);
		return;
	}
	if(P_Random(RandomClass::Heretic) < 48)
	{
		// Just noise
		S_StartMobjSound(pmo, SfxId::HereticChicact);
	}
}

// hexen

#define BLAST_RADIUS_DIST	255*FRACUNIT
#define BLAST_SPEED			20*FRACUNIT
#define BLAST_FULLSTRENGTH	255

void ResetBlasted(mobj_t* mo)
{
	mo->flags2 -= MobjFlag2::Blasted;
	if((mo->flags & MobjFlag::IceCorpse) == MobjFlag{})
	{
		mo->flags2 -= MobjFlag2::Slide;
	}
}

void P_BlastMobj(mobj_t* source, mobj_t* victim, fixed_t strength)
{
	angle_t angle, ang;
	mobj_t* mo;
	fixed_t x, y, z;

	angle = R_PointToAngle2(source->x, source->y, victim->x, victim->y);
	angle >>= ANGLETOFINESHIFT;
	if(strength < BLAST_FULLSTRENGTH)
	{
		victim->momx = FixedMul(strength, finecosine[angle]);
		victim->momy = FixedMul(strength, finesine[angle]);
		if(victim->player)
		{
			// Players handled automatically
		}
		else
		{
			victim->flags2 |= MobjFlag2::Slide;
			victim->flags2 |= MobjFlag2::Blasted;
		}
	}
	else // full strength blast from artifact
	{
		if((victim->flags & MobjFlag::Missile) != MobjFlag{})
		{
			switch(victim->type)
			{
				case MobjType::HexenSorcball1: // don't blast sorcerer balls
				case MobjType::HexenSorcball2:
				case MobjType::HexenSorcball3:
					return;
					break;
				case MobjType::HexenMstaffFx2: // Reflect to originator
					P_SetTarget(&victim->special1.m, victim->target);
					P_SetTarget(&victim->target, source);
					break;
				default:
					break;
			}
		}
		if(victim->type == MobjType::HexenHolyFx)
		{
			if(victim->special1.m == source)
			{
				P_SetTarget(&victim->special1.m, victim->target);
				P_SetTarget(&victim->target, source);
			}
		}
		victim->momx = FixedMul(BLAST_SPEED, finecosine[angle]);
		victim->momy = FixedMul(BLAST_SPEED, finesine[angle]);

		// Spawn blast puff
		ang = R_PointToAngle2(victim->x, victim->y, source->x, source->y);
		ang >>= ANGLETOFINESHIFT;
		x = victim->x + FixedMul(victim->radius + FRACUNIT, finecosine[ang]);
		y = victim->y + FixedMul(victim->radius + FRACUNIT, finesine[ang]);
		z = victim->z - victim->floorclip + (victim->height >> 1);
		mo = P_SpawnMobj(x, y, z, MobjType::HexenBlasteffect);
		if(mo)
		{
			mo->momx = victim->momx;
			mo->momy = victim->momy;
		}

		if((victim->flags & MobjFlag::Missile) != MobjFlag{})
		{
			victim->momz = 8 * FRACUNIT;
			mo->momz = victim->momz;
		}
		else
		{
			victim->momz = (1000 / victim->info->mass) << FRACBITS;
		}
		if(victim->player)
		{
			// Players handled automatically
		}
		else
		{
			victim->flags2 |= MobjFlag2::Slide;
			victim->flags2 |= MobjFlag2::Blasted;
		}
	}
}

// Blast all mobj things away
void P_BlastRadius(player_t* player)
{
	mobj_t* mo;
	mobj_t* pmo = player->mo;
	thinker_t* think;
	fixed_t dist;

	S_StartMobjSound(pmo, SfxId::HexenArtifactBlast);
	P_NoiseAlert(player->mo, player->mo);

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
		{
			// Not a mobj thinker
			continue;
		}
		mo = (mobj_t*)think;
		if((mo == pmo) || (mo->flags2 & MobjFlag2::Boss) != MobjFlag2{})
		{
			// Not a valid monster
			continue;
		}
		if((mo->type == MobjType::HexenPoisoncloud) || // poison cloud
			(mo->type == MobjType::HexenHolyFx) ||    // holy fx
			(mo->flags & MobjFlag::IceCorpse) != MobjFlag{})          // frozen corpse
		{
			// Let these special cases go
		}
		else if((mo->flags & MobjFlag::CountKill) != MobjFlag{} && (mo->health <= 0))
		{
			continue;
		}
		else if((mo->flags & MobjFlag::CountKill) == MobjFlag{} &&
			!(mo->player) && (mo->flags & MobjFlag::Missile) == MobjFlag{})
		{
			// Must be monster, player, or missile
			continue;
		}
		if((mo->flags2 & MobjFlag2::Dormant) != MobjFlag2{})
		{
			continue; // no dormant creatures
		}
		if((mo->type == MobjType::HexenWraithb) && (mo->flags2 & MobjFlag2::DontDraw) != MobjFlag2{})
		{
			continue; // no underground wraiths
		}
		if((mo->type == MobjType::HexenSplashbase) || (mo->type == MobjType::HexenSplash))
		{
			continue;
		}
		if(mo->type == MobjType::HexenSerpent || mo->type == MobjType::HexenSerpentleader)
		{
			continue;
		}
		dist = P_AproxDistance(pmo->x - mo->x, pmo->y - mo->y);
		if(dist > BLAST_RADIUS_DIST)
		{
			// Out of range
			continue;
		}
		P_BlastMobj(pmo, mo, BLAST_FULLSTRENGTH);
	}
}

void P_MorphPlayerThink(player_t* player)
{
	mobj_t* pmo;

	if(player->morphTics & 15)
	{
		return;
	}
	pmo = player->mo;
	if(!(pmo->momx + pmo->momy) && P_Random(RandomClass::Hexen) < 64)
	{
		// Snout sniff
		P_SetPspriteNF(player, PspNum::Weapon, StateId::HexenSnoutatk2);
		S_StartMobjSound(pmo, SfxId::HexenPigActive1); // snort
		return;
	}
	if(P_Random(RandomClass::Hexen) < 48)
	{
		if(P_Random(RandomClass::Hexen) < 128)
		{
			S_StartMobjSound(pmo, SfxId::HexenPigActive1);
		}
		else
		{
			S_StartMobjSound(pmo, SfxId::HexenPigActive2);
		}
	}
}

dboolean P_UndoPlayerMorph(player_t* player)
{
	mobj_t* fog;
	mobj_t* mo;
	mobj_t* pmo;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	angle_t angle;
	int playerNum;
	WeaponType weapon;
	int oldFlags; // not MobjFlag: upstream truncates to int, see below
	int oldFlags2; // not MobjFlag2: upstream truncates to int, see below
	MobjType oldBeast;

	pmo = player->mo;
	x = pmo->x;
	y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	weapon = static_cast<WeaponType>(pmo->special1.i);
	// Truncates on purpose, as upstream: flags is 64 bits but upstream keeps it in an int,
	// so bits 32-63 are lost if a failed unmorph writes it back below.
	oldFlags = static_cast<int>(std::to_underlying(pmo->flags));
	// Truncates on purpose, as upstream: flags2 is 64 bits but upstream keeps it in an int,
	// so bits 32-63 are lost if a failed unmorph writes it back below.
	oldFlags2 = static_cast<int>(std::to_underlying(pmo->flags2));
	oldBeast = pmo->type;
	P_SetMobjState(pmo, StateId::HexenFreetargmobj);
	playerNum = P_GetPlayerNum(player);
	switch(PlayerClass[playerNum])
	{
		case PClass::Fighter:
			mo = P_SpawnMobj(x, y, z, MobjType::HexenPlayerFighter);
			break;
		case PClass::Cleric:
			mo = P_SpawnMobj(x, y, z, MobjType::HexenPlayerCleric);
			break;
		case PClass::Mage:
			mo = P_SpawnMobj(x, y, z, MobjType::HexenPlayerMage);
			break;
		default:
			Log::Fatal("P_UndoPlayerMorph:  Unknown player class {}\n",
				std::to_underlying(player->pclass));
			return false;
	}
	if(P_TestMobjLocation(mo) == false)
	{
		// Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, static_cast<MobjType>(oldBeast));
		mo->angle = angle;
		mo->health = player->health;
		mo->special1.i = std::to_underlying(weapon);
		mo->player = player;
		// The truncated copy: the original bits 32-63 are gone, replaced by copies of bit 31
		mo->flags = static_cast<MobjFlag>(static_cast<uint64_t>(oldFlags));
		// The truncated copy: the original bits 32-63 are gone, replaced by copies of bit 31
		mo->flags2 = static_cast<MobjFlag2>(static_cast<uint64_t>(oldFlags2));
		player->mo = mo;
		player->morphTics = 2 * TICRATE;
		return (false);
	}
	if(player->pclass == PClass::Fighter)
	{
		// The first type should be blue, and the third should be the
		// Fighter's original gold color
		if(playerNum == 0)
		{
			mo->flags |= MobjTranslationFlags(2u);
		}
		else if(playerNum != 2)
		{
			mo->flags |= MobjTranslationFlags(playerNum);
		}
	}
	else if(playerNum)
	{
		// Set color translation bits for player sprites
		mo->flags |= MobjTranslationFlags(playerNum);
	}
	mo->angle = angle;
	mo->player = player;
	mo->reactiontime = 18;
	if(oldFlags2 & std::to_underlying(MobjFlag2::Fly))
	{
		mo->flags2 |= MobjFlag2::Fly;
		mo->flags |= MobjFlag::NoGravity;
	}
	player->morphTics = 0;
	player->health = mo->health = MAXHEALTH;
	player->mo = mo;
	player->pclass = PlayerClass[playerNum];
	angle >>= ANGLETOFINESHIFT;
	fog = P_SpawnMobj(x + 20 * finecosine[angle],
		y + 20 * finesine[angle], z + TELEFOGHEIGHT, MobjType::HexenTfog);
	S_StartMobjSound(fog, SfxId::HexenTeleport);
	P_PostMorphWeapon(player, weapon);
	return (true);
}

void P_ArtiTeleportOther(player_t* player)
{
	mobj_t* mo;

	mo = P_SpawnPlayerMissile(player->mo, MobjType::HexenTelotherFx1);
	if(mo)
	{
		P_SetTarget(&mo->target, player->mo);
	}
}


void P_TeleportToPlayerStarts(mobj_t* victim)
{
	int i, selections = 0;
	fixed_t destX, destY;
	angle_t destAngle;

	for(i = 0; i < g_maxplayers; i++)
	{
		if(!playeringame[i])
			continue;
		selections++;
	}
	i = P_Random(RandomClass::Hexen) % selections;
	destX = playerstarts[0][i].x;
	destY = playerstarts[0][i].y;
	destAngle = ANG45 * (playerstarts[0][i].angle / 45);
	P_Teleport(victim, destX, destY, destAngle, true);
}

void P_TeleportToDeathmatchStarts(mobj_t* victim)
{
	int i, selections;
	fixed_t destX, destY;
	angle_t destAngle;

	selections = deathmatch_p - deathmatchstarts;
	if(selections)
	{
		i = P_Random(RandomClass::Hexen) % selections;
		destX = deathmatchstarts[i].x;
		destY = deathmatchstarts[i].y;
		destAngle = ANG45 * (deathmatchstarts[i].angle / 45);
		P_Teleport(victim, destX, destY, destAngle, true);
	}
	else
	{
		P_TeleportToPlayerStarts(victim);
	}
}

void P_TeleportOther(mobj_t* victim)
{
	if(victim->player)
	{
		if(deathmatch)
			P_TeleportToDeathmatchStarts(victim);
		else
			P_TeleportToPlayerStarts(victim);
	}
	else
	{
		// If death action, run it upon teleport
		if((victim->flags & MobjFlag::CountKill) != MobjFlag{} && victim->special)
		{
			map_format.remove_mobj_thing_id(victim);
			map_format.execute_line_special(victim->special, victim->special_args, nullptr, 0, victim);
			victim->special = 0;
		}

		// Send all monsters to deathmatch spots
		P_TeleportToDeathmatchStarts(victim);
	}
}

#define HEAL_RADIUS_DIST	255*FRACUNIT

// Do class specific effect for everyone in radius
dboolean P_HealRadius(player_t* player)
{
	mobj_t* mo;
	mobj_t* pmo = player->mo;
	thinker_t* think;
	fixed_t dist;
	int effective = false;
	int amount;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
		{
			// Not a mobj thinker
			continue;
		}
		mo = (mobj_t*)think;

		if(!mo->player)
			continue;
		if(mo->health <= 0)
			continue;
		dist = P_AproxDistance(pmo->x - mo->x, pmo->y - mo->y);
		if(dist > HEAL_RADIUS_DIST)
		{
			// Out of range
			continue;
		}

		switch(player->pclass)
		{
			case PClass::Fighter: // Radius armor boost
				if((Hexen_P_GiveArmor(mo->player, ArmorType::Armor, 1)) ||
					(Hexen_P_GiveArmor(mo->player, ArmorType::Shield, 1)) ||
					(Hexen_P_GiveArmor(mo->player, ArmorType::Helmet, 1)) ||
					(Hexen_P_GiveArmor(mo->player, ArmorType::Amulet, 1)))
				{
					effective = true;
					S_StartMobjSound(mo, SfxId::HexenMysticincant);
				}
				break;
			case PClass::Cleric: // Radius heal
				amount = 50 + (P_Random(RandomClass::Hexen) % 50);
				if(P_GiveBody(mo->player, amount))
				{
					effective = true;
					S_StartMobjSound(mo, SfxId::HexenMysticincant);
				}
				break;
			case PClass::Mage: // Radius mana boost
				amount = 50 + (P_Random(RandomClass::Hexen) % 50);
				if((P_GiveMana(mo->player, AmmoType::Mana1, amount)) ||
					(P_GiveMana(mo->player, AmmoType::Mana2, amount)))
				{
					effective = true;
					S_StartMobjSound(mo, SfxId::HexenMysticincant);
				}
				break;
			case PClass::Pig:
			default:
				break;
		}
	}
	return (effective);
}

static dboolean Hexen_P_UseArtifact(player_t* player, ArtiType arti)
{
	mobj_t* mo;
	angle_t angle;
	int i;
	int count;

	switch(arti)
	{
		case ArtiType::HexenInvulnerability:
			if(!P_GivePower(player, PowerType::Invulnerability))
			{
				return (false);
			}
			break;
		case ArtiType::HexenHealth:
			if(!P_GiveBody(player, 25))
			{
				return (false);
			}
			break;
		case ArtiType::HexenSuperhealth:
			if(!P_GiveBody(player, 100))
			{
				return (false);
			}
			break;
		case ArtiType::HexenHealingradius:
			if(!P_HealRadius(player))
			{
				return (false);
			}
			break;
		case ArtiType::HexenTorch:
			if(!P_GivePower(player, PowerType::Infrared))
			{
				return (false);
			}
			break;
		case ArtiType::HexenEgg:
			mo = player->mo;
			P_SpawnPlayerMissile(mo, MobjType::HexenEggfx);
			P_SPMAngle(mo, MobjType::HexenEggfx, mo->angle - (ANG45 / 6));
			P_SPMAngle(mo, MobjType::HexenEggfx, mo->angle + (ANG45 / 6));
			P_SPMAngle(mo, MobjType::HexenEggfx, mo->angle - (ANG45 / 3));
			P_SPMAngle(mo, MobjType::HexenEggfx, mo->angle + (ANG45 / 3));
			break;
		case ArtiType::HexenFly:
			if(!P_GivePower(player, PowerType::Flight))
			{
				return (false);
			}
			if(player->mo->momz <= -35 * FRACUNIT)
			{
				// stop falling scream
				S_StopSound(player->mo);
			}
			break;
		case ArtiType::HexenSummon:
			mo = P_SpawnPlayerMissile(player->mo, MobjType::HexenSummonFx);
			if(mo)
			{
				P_SetTarget(&mo->target, player->mo);
				P_SetTarget(&mo->special1.m, player->mo);
				mo->momz = 5 * FRACUNIT;
			}
			break;
		case ArtiType::HexenTeleport:
			P_ArtiTele(player);
			break;
		case ArtiType::HexenTeleportother:
			P_ArtiTeleportOther(player);
			break;
		case ArtiType::HexenPoisonbag:
			angle = player->mo->angle >> ANGLETOFINESHIFT;
			if(player->pclass == PClass::Cleric)
			{
				mo = P_SpawnMobj(player->mo->x + 16 * finecosine[angle],
					player->mo->y + 24 * finesine[angle],
					player->mo->z - player->mo->floorclip +
					8 * FRACUNIT, MobjType::HexenPoisonbag);
				if(mo)
				{
					P_SetTarget(&mo->target, player->mo);
				}
			}
			else if(player->pclass == PClass::Mage)
			{
				mo = P_SpawnMobj(player->mo->x + 16 * finecosine[angle],
					player->mo->y + 24 * finesine[angle],
					player->mo->z - player->mo->floorclip +
					8 * FRACUNIT, MobjType::HexenFirebomb);
				if(mo)
				{
					P_SetTarget(&mo->target, player->mo);
				}
			}
			else // PCLASS_FIGHTER, obviously (also pig, not so obviously)
			{
				mo = P_SpawnMobj(player->mo->x, player->mo->y,
					player->mo->z - player->mo->floorclip +
					35 * FRACUNIT, MobjType::HexenThrowingbomb);
				if(mo)
				{
					mo->angle =
						player->mo->angle + (((P_Random(RandomClass::Hexen) & 7) - 4) << 24);
					mo->momz =
						4 * FRACUNIT + (dsda_PlayerLookDir(player) << (FRACBITS - 4));
					mo->z += dsda_PlayerLookDir(player) << (FRACBITS - 4);
					P_ThrustMobj(mo, mo->angle, mo->info->speed);
					mo->momx += player->mo->momx >> 1;
					mo->momy += player->mo->momy >> 1;
					P_SetTarget(&mo->target, player->mo);
					mo->tics -= P_Random(RandomClass::Hexen) & 3;
					P_CheckMissileSpawn(mo);
				}
			}
			break;
		case ArtiType::HexenSpeed:
			if(!P_GivePower(player, PowerType::Speed))
			{
				return (false);
			}
			break;
		case ArtiType::HexenBoostmana:
			if(!P_GiveMana(player, AmmoType::Mana1, MAX_MANA))
			{
				if(!P_GiveMana(player, AmmoType::Mana2, MAX_MANA))
				{
					return false;
				}
			}
			else
			{
				P_GiveMana(player, AmmoType::Mana2, MAX_MANA);
			}
			break;
		case ArtiType::HexenBoostarmor:
			count = 0;

			for(i = 0; i < std::to_underlying(ArmorType::Count); i++)
			{
				count += Hexen_P_GiveArmor(player, static_cast<ArmorType>(i), 1); // 1 point per armor type
			}
			if(!count)
			{
				return false;
			}
			break;
		case ArtiType::HexenBlastradius:
			P_BlastRadius(player);
			break;

		case ArtiType::HexenPuzzskull:
		case ArtiType::HexenPuzzgembig:
		case ArtiType::HexenPuzzgemred:
		case ArtiType::HexenPuzzgemgreen1:
		case ArtiType::HexenPuzzgemgreen2:
		case ArtiType::HexenPuzzgemblue1:
		case ArtiType::HexenPuzzgemblue2:
		case ArtiType::HexenPuzzbook1:
		case ArtiType::HexenPuzzbook2:
		case ArtiType::HexenPuzzskull2:
		case ArtiType::HexenPuzzfweapon:
		case ArtiType::HexenPuzzcweapon:
		case ArtiType::HexenPuzzmweapon:
		case ArtiType::HexenPuzzgear1:
		case ArtiType::HexenPuzzgear2:
		case ArtiType::HexenPuzzgear3:
		case ArtiType::HexenPuzzgear4:
			if(P_UsePuzzleItem(player, std::to_underlying(arti) - std::to_underlying(ArtiType::HexenFirstpuzzitem)))
			{
				return true;
			}
			else
			{
				P_SetYellowMessage(player, TXT_USEPUZZLEFAILED, false);
				return false;
			}
			break;
		default:
			return false;
	}
	return true;
}

extern "C" void A_SpeedFade(mobj_t* actor)
{
	actor->flags |= MobjFlag::Shadow;
	actor->flags -= MobjFlag::AltShadow;
	actor->sprite = actor->target->sprite;
}
