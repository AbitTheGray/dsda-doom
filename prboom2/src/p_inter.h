// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Thing events, and dehacked specified numbers controlling them.
 */

#pragma once

#include "d_player.h"
#include "p_mobj.h"

/* Ty 03/09/98 Moved to an int in p_inter.c for deh and externalization */
#define MAXHEALTH maxhealth

#define BONUSADD        6

/* follow a player exlusively for 3 seconds */
#define BASETHRESHOLD   (100)

dboolean P_GivePower(player_t *, int);
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher);
void P_DamageMobj(mobj_t *target,mobj_t *inflictor,mobj_t *source,int damage);
void P_HealMobj(mobj_t *mo, int num);
int P_PlayerHealthIncrease(int value);

/* killough 5/2/98: moved from d_deh.c, g_game.c, m_misc.c, others: */

extern int god_health;   /* Ty 03/09/98 - deh support, see also p_inter.c */
extern int idfa_armor;
extern int idfa_armor_class;
extern int idkfa_armor;
extern int idkfa_armor_class;  /* Ty - end */
/* Ty 03/13/98 - externalized initial settings for respawned player */
extern int initial_health;
extern int initial_bullets;
extern int maxhealth;
extern int maxhealthbonus;
extern int max_armor;
extern int green_armor_class;
extern int blue_armor_class;
extern int max_soul;
extern int soul_health;
extern int mega_health;
extern int bfgcells;
extern int monsters_infight; // e6y: Dehacked support - monsters infight
extern int maxammo[], clipammo[];

// heretic

#define MAXCHICKENHEALTH 30

extern int GetWeaponAmmo[NUMWEAPONS];

dboolean P_GiveBody(player_t * player, int num);
void P_SetMessage(player_t * player, const char *message, dboolean ultmsg);
dboolean P_GiveArtifact(player_t * player, artitype_t arti, mobj_t * mo);
dboolean Heretic_P_GiveWeapon(player_t * player, weapontype_t weapon);
void P_SetDormantArtifact(mobj_t * arti);
void P_HideSpecialThing(mobj_t * thing);
dboolean P_ChickenMorphPlayer(player_t * player);
dboolean P_ChickenMorph(mobj_t * target);
void P_TouchWhirlwind(mobj_t * target);
void P_MinotaurSlam(mobj_t * source, mobj_t * target);
dboolean P_AutoUseChaosDevice(player_t * player);
void P_AutoUseHealth(player_t * player, int saveHealth);

// hexen

#define MAXMORPHHEALTH 30

void P_SetYellowMessage(player_t * player, const char *message, dboolean ultmsg);
void P_FallingDamage(player_t * player);
void P_PoisonPlayer(player_t * player, mobj_t * poisoner, int poison);
void P_PoisonDamage(player_t * player, mobj_t * source, int damage, dboolean playPainSound);
dboolean P_GiveMana(player_t * player, manatype_t mana, int count);
dboolean Hexen_P_GiveArmor(player_t *player, armortype_t armortype, int amount);
dboolean P_MorphPlayer(player_t * player);
