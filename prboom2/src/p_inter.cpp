// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Handling interactions (i.e., collisions).
 */

#include <utility>

#include "doomstat.hpp"
#include "dstrings.hpp"
#include "m_random.hpp"
#include "am_map.hpp"
#include "r_main.hpp"
#include "s_sound.hpp"
#include "smooth.hpp"
#include "sounds.hpp"
#include "d_deh.hpp"  // Ty 03/22/98 - externalized strings
#include "p_tick.hpp"
#include "lprintf.hpp"

#include "p_inter.hpp"
#include "p_enemy.hpp"
#include "p_spec.hpp"
#include "p_pspr.hpp"
#include "p_user.hpp"

#include "p_inter.hpp"
#include "e6y.hpp"//e6y

#include "dsda.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/messenger.hpp"
#include "dsda/skill_info.hpp"

#include "heretic/def.hpp"
#include "heretic/sb_bar.hpp"

#include "hexen/p_acs.hpp"

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values.  Need to externalize those for
// dehacked support (and future flexibility).  Most var names came from the key
// strings used in dehacked.

int initial_health = 100;
int initial_bullets = 50;
int maxhealth = 100; // was MAXHEALTH as a #define, used only in this module
int maxhealthbonus = 200;
int max_armor = 200;
int green_armor_class = 1; // these are involved with armortype below
int blue_armor_class = 2;
int max_soul = 200;
int soul_health = 100;
int mega_health = 200;
int god_health = 100; // these are used in cheats (see st_stuff.c)
int idfa_armor = 200;
int idfa_armor_class = 2;
// not actually used due to pairing of cheat_k and cheat_fa
int idkfa_armor = 200;
int idkfa_armor_class = 2;

int bfgcells = 40;        // used in p_pspr.c
int monsters_infight = 0; // e6y: Dehacked support - monsters infight
// Ty 03/07/98 - end deh externals

// a weapon is found with two clip loads,
// a big item has five clip loads
int maxammo[std::to_underlying(AmmoType::Count)] = {200, 50, 300, 50, 0, 0}; // heretic +2 ammo types
int clipammo[std::to_underlying(AmmoType::Count)] = {10, 4, 20, 1, 0, 0};    // heretic +2 ammo types

//
// GET STUFF
//

// heretic
static WeaponType GetAmmoChange[] = {
	WeaponType::GoldWand,
	WeaponType::Crossbow,
	WeaponType::Blaster,
	WeaponType::SkullRod,
	WeaponType::PhoenixRod,
	WeaponType::Mace
};

//
// P_AutoSwitchWeapon
// Autoswitches player to a weapon,
// Based on config and other conditions.
//

static void P_AutoSwitchWeapon(player_t* player, WeaponType weapon)
{
	int autoswitch_config = dsda_IntConfig(ConfigId::SwitchWeaponOnPickup);
	int autoswitch = ((allow_incompatibility && !deathmatch && !netgame) ? autoswitch_config : true);

	if(!autoswitch) return;

	player->pendingweapon = weapon;
}

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

static dboolean P_GiveAmmoAutoSwitch(player_t* player, AmmoType ammo, int oldammo)
{
	int i;

	if(
		weaponinfo[std::to_underlying(player->readyweapon)].flags & WPF_AUTOSWITCHFROM &&
		weaponinfo[std::to_underlying(player->readyweapon)].ammo != ammo
	)
	{
		for(i = std::to_underlying(WeaponType::Count) - 1; i > std::to_underlying(player->readyweapon); --i)
		{
			if(
				player->weaponowned[i] &&
				!(weaponinfo[i].flags & WPF_NOAUTOSWITCHTO) &&
				weaponinfo[i].ammo == ammo &&
				weaponinfo[i].ammopershot > oldammo &&
				weaponinfo[i].ammopershot <= player->ammo[std::to_underlying(ammo)]
			)
			{
				P_AutoSwitchWeapon(player, static_cast<WeaponType>(i));
				break;
			}
		}
	}

	return true;
}

static dboolean P_GiveAmmo(player_t* player, AmmoType ammo, int num)
{
	int oldammo;

	if(ammo == AmmoType::NoAmmo)
		return false;

#ifdef RANGECHECK
	if(ammo < 0 || ammo > std::to_underlying(AmmoType::Count))
		I_Error("P_GiveAmmo: bad type %i", ammo);
#endif

	if(player->ammo[std::to_underlying(ammo)] == player->maxammo[std::to_underlying(ammo)])
		return false;

	if(num)
		num *= clipammo[std::to_underlying(ammo)];
	else
		num = clipammo[std::to_underlying(ammo)] / 2;

	if(skill_info.ammo_factor)
		num = FixedMul(num, skill_info.ammo_factor);

	oldammo = player->ammo[std::to_underlying(ammo)];
	player->ammo[std::to_underlying(ammo)] += num;

	if(player->ammo[std::to_underlying(ammo)] > player->maxammo[std::to_underlying(ammo)])
		player->ammo[std::to_underlying(ammo)] = player->maxammo[std::to_underlying(ammo)];

	if(mbf21)
		return P_GiveAmmoAutoSwitch(player, ammo, oldammo);

	// If non zero ammo, don't change up weapons, player was lower on purpose.
	if(oldammo)
		return true;

	// We were down to zero, so select a new weapon.
	// Preferences are not user selectable.

	if(heretic)
	{
		if(player->readyweapon == WeaponType::Staff || player->readyweapon == WeaponType::Gauntlets)
		{
			if(player->weaponowned[std::to_underlying(GetAmmoChange[std::to_underlying(ammo)])])
			{
				P_AutoSwitchWeapon(player, static_cast<WeaponType>(GetAmmoChange[std::to_underlying(ammo)]));
			}
		}

		return true;
	}

	switch(ammo)
	{
		case AmmoType::Clip:
			if(player->readyweapon == WeaponType::Fist)
			{
				if(player->weaponowned[std::to_underlying(WeaponType::Chaingun)])
					P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Chaingun));
				else
					P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Pistol));
			}
			break;

		case AmmoType::Shell:
			if(player->readyweapon == WeaponType::Fist || player->readyweapon == WeaponType::Pistol)
				if(player->weaponowned[std::to_underlying(WeaponType::Shotgun)])
					P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Shotgun));
			break;

		case AmmoType::Cell:
			if(player->readyweapon == WeaponType::Fist || player->readyweapon == WeaponType::Pistol)
				if(player->weaponowned[std::to_underlying(WeaponType::Plasma)])
					P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Plasma));
			break;

		case AmmoType::Misl:
			if(player->readyweapon == WeaponType::Fist)
				if(player->weaponowned[std::to_underlying(WeaponType::Missile)])
					P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Missile));
		default:
			break;
	}
	return true;
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//

extern "C" dboolean P_GiveWeapon(player_t* player, WeaponType weapon, dboolean dropped)
{
	dboolean gaveammo;
	dboolean gaveweapon;

	if(heretic) return Heretic_P_GiveWeapon(player, weapon);

	if(netgame && deathmatch != 2 && !dropped)
	{
		// leave placed weapons forever on net games
		if(player->weaponowned[std::to_underlying(weapon)])
			return false;

		player->bonuscount += BONUSADD;
		player->weaponowned[std::to_underlying(weapon)] = true;

		P_GiveAmmo(player, static_cast<AmmoType>(weaponinfo[std::to_underlying(weapon)].ammo), deathmatch ? 5 : 2);
		P_AutoSwitchWeapon(player, static_cast<WeaponType>(weapon));
		/* cph 20028/10 - for old-school DM addicts, allow old behavior
		* where only consoleplayer's pickup sounds are heard */
		// displayplayer, not consoleplayer, for viewing multiplayer demos
		if(!comp[std::to_underlying(CompOption::Sound)])
			S_StartSound(player->mo, SfxAsPickup(SfxId::Wpnup)); // killough 4/25/98
		else if(player == &players[displayplayer])
			S_StartVoidSound(SfxAsPickup(SfxId::Wpnup));
		return false;
	}

	if(weaponinfo[std::to_underlying(weapon)].ammo != AmmoType::NoAmmo)
	{
		// give one clip with a dropped weapon,
		// two clips with a found weapon
		gaveammo = P_GiveAmmo(player, static_cast<AmmoType>(weaponinfo[std::to_underlying(weapon)].ammo), dropped ? 1 : 2);
	}
	else
		gaveammo = false;

	if(player->weaponowned[std::to_underlying(weapon)])
		gaveweapon = false;
	else
	{
		gaveweapon = true;
		player->weaponowned[std::to_underlying(weapon)] = true;
		P_AutoSwitchWeapon(player, static_cast<WeaponType>(weapon));
	}
	return gaveweapon || gaveammo;
}

int P_PlayerHealthIncrease(int value)
{
	if(skill_info.health_factor)
		value = FixedMul(value, skill_info.health_factor);

	return value;
}

int P_PlayerArmorIncrease(int value)
{
	if(skill_info.armor_factor)
		value = FixedMul(value, skill_info.armor_factor);

	return value;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//

dboolean P_GiveBody(player_t* player, int num)
{
	int max;

	max = maxhealth;
	if(player->chickenTics)
	{
		max = MAXCHICKENHEALTH;
	}
	if(player->morphTics)
	{
		max = MAXMORPHHEALTH;
	}
	if(player->health >= max)
	{
		return (false);
	}
	player->health += P_PlayerHealthIncrease(num);
	if(player->health > max)
	{
		player->health = max;
	}
	player->mo->health = player->health;
	return (true);
}

void P_HealMobj(mobj_t* mo, int num)
{
	player_t* player = mo->player;

	if(mo->health <= 0 || (player && player->playerstate == PlayerState::Dead))
		return;

	if(player)
	{
		P_GiveBody(player, num);
		return;
	}
	else
	{
		int max = P_MobjSpawnHealth(mo);

		mo->health += num;
		if(mo->health > max)
			mo->health = max;
	}
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//

static dboolean P_GiveArmor(player_t* player, int armortype)
{
	int hits = P_PlayerArmorIncrease(armortype * 100);
	if(player->armorpoints[std::to_underlying(ArmorType::Armor)] >= hits)
		return false; // don't pick up
	player->armortype = armortype;
	player->armorpoints[std::to_underlying(ArmorType::Armor)] = hits;
	return true;
}

//
// P_GiveCard
//

void P_GiveCard(player_t* player, Card card)
{
	if(player->cards[std::to_underlying(card)])
		return;
	player->bonuscount = BONUSADD;
	player->cards[std::to_underlying(card)] = 1;

	if(player == &players[consoleplayer])
		player->ravenkeys |= 1 << std::to_underlying(card);

	dsda_WatchCard(card);
}

//
// P_GivePower
//
// Rewritten by Lee Killough
//

dboolean P_GivePower(player_t* player, PowerType power)
{
	static const int tics[std::to_underlying(PowerType::Count)] = {
		std::to_underlying(PowerDuration::Invulntics), 1 /* strength */, std::to_underlying(PowerDuration::Invistics),
		std::to_underlying(PowerDuration::Irontics), 1 /* allmap */, std::to_underlying(PowerDuration::Infratics),
		std::to_underlying(PowerDuration::Wpnlev2tics), std::to_underlying(PowerDuration::Flighttics), 1 /* shield */, 1 /* health2 */,
		std::to_underlying(PowerDuration::Speedtics), std::to_underlying(PowerDuration::Maulatortics)
	};

	if(
		raven &&
		tics[std::to_underlying(power)] > 1 &&
		power != PowerType::IronFeet && power != PowerType::Minotaur &&
		player->powers[std::to_underlying(power)] > BLINKTHRESHOLD
	)
		return false;

	switch(power)
	{
		case PowerType::Invulnerability:
			if(hexen)
			{
				player->mo->flags2 |= MF2_INVULNERABLE;
				if(player->pclass == PClass::Mage)
				{
					player->mo->flags2 |= MF2_REFLECTIVE;
				}
			}
			break;
		case PowerType::Invisibility:
			player->mo->flags |= MF_SHADOW;
			break;
		case PowerType::AllMap:
			if(player->powers[std::to_underlying(PowerType::AllMap)])
				return false;
			break;
		case PowerType::Strength:
			P_GiveBody(player, 100);
			break;
		case PowerType::Flight:
			player->mo->flags2 |= MF2_FLY;
			player->mo->flags |= MF_NOGRAVITY;
			if(player->mo->z <= player->mo->floorz)
			{
				player->flyheight = 10; // thrust the player in the air a bit
			}
			break;
	}

	if(hexen && player->powers[std::to_underlying(power)])
		return false;

	// Unless player has infinite duration cheat, set duration (killough)

	if(player->powers[std::to_underlying(power)] >= 0)
		player->powers[std::to_underlying(power)] = tics[std::to_underlying(power)];
	return true;
}

//
// P_TouchSpecialThing
//

static void Heretic_P_TouchSpecialThing(mobj_t* special, mobj_t* toucher);
static void Hexen_P_TouchSpecialThing(mobj_t* special, mobj_t* toucher);

void P_TouchSpecialThing(mobj_t* special, mobj_t* toucher)
{
	player_t* player;
	int i;
	SfxId sound;
	fixed_t delta = special->z - toucher->z;

	if(heretic) return Heretic_P_TouchSpecialThing(special, toucher);
	if(hexen) return Hexen_P_TouchSpecialThing(special, toucher);

	if(delta > toucher->height || delta < -8 * FRACUNIT)
		return; // out of reach

	sound = SfxId::Itemup;
	player = toucher->player;

	// Dead thing touching.
	// Can happen with a sliding player corpse.
	if(toucher->health <= 0)
		return;

	// Identify by sprite.
	switch(special->sprite)
	{
		// armor
		case SpriteId::Arm1:
			if(!P_GiveArmor(player, green_armor_class))
				return;
			dsda_AddPlayerMessage(s_GOTARMOR, player);
			break;

		case SpriteId::Arm2:
			if(!P_GiveArmor(player, blue_armor_class))
				return;
			dsda_AddPlayerMessage(s_GOTMEGA, player);
			break;

		// bonus items
		case SpriteId::Bon1:
			// can go over 100%
			player->health += P_PlayerHealthIncrease(1);
			if(player->health > (maxhealthbonus))  //e6y
				player->health = (maxhealthbonus); //e6y
			player->mo->health = player->health;
			dsda_AddPlayerMessage(s_GOTHTHBONUS, player);
			break;

		case SpriteId::Bon2:
			// can go over 100%
			player->armorpoints[std::to_underlying(ArmorType::Armor)] += P_PlayerArmorIncrease(1);
			// e6y
			// Doom 1.2 does not do check of armor points on overflow.
			// If you set the "IDKFA Armor" to MAX_INT (DWORD at 0x00064B5A -> FFFFFF7F)
			// and pick up one or more armor bonuses, your armor becomes negative
			// and you will die after reception of any damage since this moment.
			// It happens because the taken health damage depends from armor points
			// if they are present and becomes equal to very large value in this case
			if(player->armorpoints[std::to_underlying(ArmorType::Armor)] > max_armor && compatibility_level != CompLevel::Doom12)
				player->armorpoints[std::to_underlying(ArmorType::Armor)] = max_armor;
			// e6y
			// We always give armor type 1 for the armor bonuses;
			// dehacked only affects the GreenArmor.
			if(!player->armortype)
				player->armortype =
					((!demo_compatibility || prboom_comp[std::to_underlying(PrboomComp::ApplyGreenArmorClassToArmorBonuses)].state) ? green_armor_class : 1);
			dsda_AddPlayerMessage(s_GOTARMBONUS, player);
			break;

		case SpriteId::Bon3: // killough 7/11/98: evil sceptre from beta version
			dsda_AddPlayerMessage(s_BETA_BONUS3, player);
			break;

		case SpriteId::Bon4: // killough 7/11/98: unholy bible from beta version
			dsda_AddPlayerMessage(s_BETA_BONUS4, player);
			break;

		case SpriteId::Soul:
			player->health += P_PlayerHealthIncrease(soul_health);
			if(player->health > max_soul)
				player->health = max_soul;
			player->mo->health = player->health;
			dsda_AddPlayerMessage(s_GOTSUPER, player);
			sound = SfxId::Getpow;
			break;

		case SpriteId::Mega:
			if(gamemode != GameMode::Commercial)
				return;
			player->health = mega_health;
			player->mo->health = player->health;
			// e6y
			// We always give armor type 2 for the megasphere;
			// dehacked only affects the MegaArmor.
			P_GiveArmor(player,
				((!demo_compatibility || prboom_comp[std::to_underlying(PrboomComp::ApplyBlueArmorClassToMegasphere)].state) ? blue_armor_class : 2));
			dsda_AddPlayerMessage(s_GOTMSPHERE, player);
			sound = SfxId::Getpow;
			break;

		// cards
		// leave cards for everyone
		case SpriteId::Bkey:
			if(!player->cards[std::to_underlying(Card::BlueCard)])
				dsda_AddPlayerMessage(s_GOTBLUECARD, player);
			P_GiveCard(player, Card::BlueCard);
			if(!netgame)
				break;
			return;

		case SpriteId::Ykey:
			if(!player->cards[std::to_underlying(Card::YellowCard)])
				dsda_AddPlayerMessage(s_GOTYELWCARD, player);
			P_GiveCard(player, Card::YellowCard);
			if(!netgame)
				break;
			return;

		case SpriteId::Rkey:
			if(!player->cards[std::to_underlying(Card::RedCard)])
				dsda_AddPlayerMessage(s_GOTREDCARD, player);
			P_GiveCard(player, Card::RedCard);
			if(!netgame)
				break;
			return;

		case SpriteId::Bsku:
			if(!player->cards[std::to_underlying(Card::BlueSkull)])
				dsda_AddPlayerMessage(s_GOTBLUESKUL, player);
			P_GiveCard(player, Card::BlueSkull);
			if(!netgame)
				break;
			return;

		case SpriteId::Ysku:
			if(!player->cards[std::to_underlying(Card::YellowSkull)])
				dsda_AddPlayerMessage(s_GOTYELWSKUL, player);
			P_GiveCard(player, Card::YellowSkull);
			if(!netgame)
				break;
			return;

		case SpriteId::Rsku:
			if(!player->cards[std::to_underlying(Card::RedSkull)])
				dsda_AddPlayerMessage(s_GOTREDSKULL, player);
			P_GiveCard(player, Card::RedSkull);
			if(!netgame)
				break;
			return;

		// medikits, heals
		case SpriteId::Stim:
			if(!P_GiveBody(player, 10))
				return;
			dsda_AddPlayerMessage(s_GOTSTIM, player);
			break;

		case SpriteId::Medi:
			if(!P_GiveBody(player, 25))
				return;

			if(player->health < 50) // cph - 25 + the 25 just added, thanks to Quasar for reporting this bug
				dsda_AddPlayerMessage(s_GOTMEDINEED, player);
			else
				dsda_AddPlayerMessage(s_GOTMEDIKIT, player);
			break;


		// power ups
		case SpriteId::Pinv:
			if(!P_GivePower(player, PowerType::Invulnerability))
				return;
			dsda_AddPlayerMessage(s_GOTINVUL, player);
			sound = SfxId::Getpow;
			break;

		case SpriteId::Pstr:
			if(!P_GivePower(player, PowerType::Strength))
				return;
			dsda_AddPlayerMessage(s_GOTBERSERK, player);
			if(player->readyweapon != WeaponType::Fist)
				P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Fist));
			sound = SfxId::Getpow;
			break;

		case SpriteId::Pins:
			if(!P_GivePower(player, PowerType::Invisibility))
				return;
			dsda_AddPlayerMessage(s_GOTINVIS, player);
			sound = SfxId::Getpow;
			break;

		case SpriteId::Suit:
			if(!P_GivePower(player, PowerType::IronFeet))
				return;
			dsda_AddPlayerMessage(s_GOTSUIT, player);
			sound = SfxId::Getpow;
			break;

		case SpriteId::Pmap:
			if(!P_GivePower(player, PowerType::AllMap))
				return;
			dsda_AddPlayerMessage(s_GOTMAP, player);
			sound = SfxId::Getpow;
			break;

		case SpriteId::Pvis:
			if(!P_GivePower(player, PowerType::Infrared))
				return;
			dsda_AddPlayerMessage(s_GOTVISOR, player);
			sound = SfxId::Getpow;
			break;

		// ammo
		case SpriteId::Clip:
			if(special->flags & MF_DROPPED)
			{
				if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Clip), 0))
					return;
			}
			else
			{
				if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Clip), 1))
					return;
			}
			dsda_AddPlayerMessage(s_GOTCLIP, player);
			break;

		case SpriteId::Ammo:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Clip), 5))
				return;
			dsda_AddPlayerMessage(s_GOTCLIPBOX, player);
			break;

		case SpriteId::Rock:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Misl), 1))
				return;
			dsda_AddPlayerMessage(s_GOTROCKET, player);
			break;

		case SpriteId::Brok:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Misl), 5))
				return;
			dsda_AddPlayerMessage(s_GOTROCKBOX, player);
			break;

		case SpriteId::Cell:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Cell), 1))
				return;
			dsda_AddPlayerMessage(s_GOTCELL, player);
			break;

		case SpriteId::Celp:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Cell), 5))
				return;
			dsda_AddPlayerMessage(s_GOTCELLBOX, player);
			break;

		case SpriteId::Shel:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Shell), 1))
				return;
			dsda_AddPlayerMessage(s_GOTSHELLS, player);
			break;

		case SpriteId::Sbox:
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Shell), 5))
				return;
			dsda_AddPlayerMessage(s_GOTSHELLBOX, player);
			break;

		case SpriteId::Bpak:
			if(!player->backpack)
			{
				for(i = 0; i < std::to_underlying(AmmoType::Count); i++)
					player->maxammo[i] *= 2;
				player->backpack = true;
			}
			for(i = 0; i < std::to_underlying(AmmoType::Count); i++)
				P_GiveAmmo(player, static_cast<AmmoType>(i), 1);
			dsda_AddPlayerMessage(s_GOTBACKPACK, player);
			break;

		// weapons
		case SpriteId::Bfug:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Bfg), false))
				return;
			dsda_AddPlayerMessage(s_GOTBFG9000, player);
			sound = SfxId::Wpnup;
			break;

		case SpriteId::Mgun:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Chaingun), (special->flags & MF_DROPPED) != 0))
				return;
			dsda_AddPlayerMessage(s_GOTCHAINGUN, player);
			sound = SfxId::Wpnup;
			break;

		case SpriteId::Csaw:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Chainsaw), false))
				return;
			dsda_AddPlayerMessage(s_GOTCHAINSAW, player);
			sound = SfxId::Wpnup;
			break;

		case SpriteId::Laun:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Missile), false))
				return;
			dsda_AddPlayerMessage(s_GOTLAUNCHER, player);
			sound = SfxId::Wpnup;
			break;

		case SpriteId::Plas:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Plasma), false))
				return;
			dsda_AddPlayerMessage(s_GOTPLASMA, player);
			sound = SfxId::Wpnup;
			break;

		case SpriteId::Shot:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Shotgun), (special->flags & MF_DROPPED) != 0))
				return;
			dsda_AddPlayerMessage(s_GOTSHOTGUN, player);
			sound = SfxId::Wpnup;
			break;

		case SpriteId::Sgn2:
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Supershotgun), (special->flags & MF_DROPPED) != 0))
				return;
			dsda_AddPlayerMessage(s_GOTSHOTGUN2, player);
			sound = SfxId::Wpnup;
			break;

		default:
			I_Error("P_SpecialThing: Unknown gettable thing");
	}

	if(special->special)
	{
		map_format.execute_line_special(special->special, special->special_args, nullptr, 0, player->mo);
		special->special = 0;
	}

	if(special->flags & MF_COUNTITEM)
		player->itemcount++;

	if(special->flags2 & MF2_COUNTSECRET)
		P_PlayerCollectSecret(player);

	P_RemoveMobj(special);
	player->bonuscount += BONUSADD;

	/* cph 20028/10 - for old-school DM addicts, allow old behavior
	* where only consoleplayer's pickup sounds are heard */
	// displayplayer, not consoleplayer, for viewing multiplayer demos
	if(!comp[std::to_underlying(CompOption::Sound)])
		S_StartSound(player->mo, SfxAsPickup(sound)); // killough 4/25/98
	else if(player == &players[displayplayer])
		S_StartVoidSound(SfxAsPickup(sound));
}

//
// KillMobj
//

static mobj_t* ActiveMinotaur(player_t* master);

// killough 11/98: make static
static void P_KillMobj(mobj_t* source, mobj_t* target)
{
	MobjType item;
	mobj_t* mo;
	int xdeath_limit;

	target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

	if(target->type != MobjType::Skull)
		target->flags &= ~MF_NOGRAVITY;

	target->flags |= MF_CORPSE | MF_DROPOFF;
	target->height >>= 2;

	// heretic
	target->flags2 &= ~MF2_PASSMOBJ;

	if(
		mbf21 || (
			compatibility_level == CompLevel::Mbf &&
			!prboom_comp[std::to_underlying(PrboomComp::MbfRemoveThinkerInKillmobj)].state
		)
	)
	{
		// killough 8/29/98: remove from threaded list
		P_UpdateThinker(&target->thinker);
	}

	if(!((target->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
		totallive--;

	dsda_WatchDeath(target);

	if(map_format.hexen && target->special)
	{
		if(!hexen || (target->flags & MF_COUNTKILL || target->type == MobjType::HexenZbell))
		{
			// Initiate monster death actions
			if(hexen && target->type == MobjType::HexenSorcboss)
			{
				byte dummyArgs[3] = {0, 0, 0};
				P_StartACS(target->special, 0, dummyArgs, target, nullptr, 0);
			}
			else
			{
				// TODO: tenuous "activate own death specials" mapinfo flag
				map_format.execute_line_special(target->special, target->special_args, nullptr, 0, target);
			}
		}
	}

	if(source && source->player)
	{
		// count for intermission
		if(target->flags & MF_COUNTKILL)
		{
			dsda_WatchKill(source->player, target);
		}
		if(target->player)
		{
			source->player->frags[target->player - players]++;

			if(heretic && target != source)
			{
				if(source->player == &players[consoleplayer])
				{
					S_StartVoidSound(SfxId::HereticGfrag);
				}
				if(source->player->chickenTics)
				{
					// Make a super chicken
					P_GivePower(source->player, PowerType::WeaponLevel2);
				}
			}
		}
	}
	else if(target->flags & MF_COUNTKILL)
	{
		/* Add to kills tally */
		if((compatibility_level < CompLevel::Lxdoom1) || !netgame)
		{
			if(!netgame)
			{
				dsda_WatchKill(&players[0], target);
			}
			else
			{
				if(!deathmatch)
				{
					if(target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player)
					{
						dsda_WatchKill(target->lastenemy->player, target);
					}
					else
					{
						unsigned int player;
						for(player = 0; player < g_maxplayers; player++)
						{
							if(playeringame[player])
							{
								dsda_WatchKill(&players[player], target);
								break;
							}
						}
					}
				}
			}
		}
		else if(!deathmatch)
		{
			// try and find a player to give the kill to, otherwise give the
			// kill to a random player.  this fixes the missing monsters bug
			// in coop - rain
			// CPhipps - not a bug as such, but certainly an inconsistency.
			if(target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player) // Fighting a player
			{
				dsda_WatchKill(target->lastenemy->player, target);
			}
			else
			{
				// cph - randomely choose a player in the game to be credited
				//  and do it uniformly between the active players
				unsigned int activeplayers = 0, player, i;

				for(player = 0; player < g_maxplayers; player++)
					if(playeringame[player])
						activeplayers++;

				if(activeplayers)
				{
					player = P_Random(RandomClass::Friends) % activeplayers;

					for(i = 0; i < g_maxplayers; i++)
						if(playeringame[i])
							if(!player--)
							{
								dsda_WatchKill(&players[i], target);
							}
				}
			}
		}
	}

	if(target->player)
	{
		// count environment kills against you
		if(!source)
			target->player->frags[target->player - players]++;

		target->flags &= ~MF_SOLID;

		// heretic
		target->flags2 &= ~MF2_FLY;
		target->player->powers[std::to_underlying(PowerType::Flight)] = 0;
		target->player->powers[std::to_underlying(PowerType::WeaponLevel2)] = 0;

		target->player->playerstate = PlayerState::Dead;
		P_DropWeapon(target->player);

		// heretic
		if(target->flags2 & MF2_FIREDAMAGE)
		{
			// Player flame death
			switch(target->player->pclass)
			{
				case PClass::Null: // heretic
					P_SetMobjState(target, StateId::HereticPlayFdth1);
					return;
				case PClass::Fighter:
					S_StartMobjSound(target, SfxId::HexenPlayerFighterBurnDeath);
					P_SetMobjState(target, StateId::HexenPlayFFdth1);
					return;
				case PClass::Cleric:
					S_StartMobjSound(target, SfxId::HexenPlayerClericBurnDeath);
					P_SetMobjState(target, StateId::HexenPlayCFdth1);
					return;
				case PClass::Mage:
					S_StartMobjSound(target, SfxId::HexenPlayerMageBurnDeath);
					P_SetMobjState(target, StateId::HexenPlayMFdth1);
					return;
				default:
					break;
			}
		}

		// hexen
		if(target->flags2 & MF2_ICEDAMAGE)
		{
			// Player ice death
			target->flags &= ~(7 << MF_TRANSSHIFT); //no translation
			target->flags |= MF_ICECORPSE;
			switch(target->player->pclass)
			{
				case PClass::Fighter:
					P_SetMobjState(target, StateId::HexenFplayIce);
					return;
				case PClass::Cleric:
					P_SetMobjState(target, StateId::HexenCplayIce);
					return;
				case PClass::Mage:
					P_SetMobjState(target, StateId::HexenMplayIce);
					return;
				case PClass::Pig:
					P_SetMobjState(target, StateId::HexenPigIce);
					return;
				default:
					break;
			}
		}

		if(target->player == &players[consoleplayer] && automap_full)
			AM_Stop(true); // don't die in auto map; switch view prior to dying
	}

	if(hexen)
	{
		if(target->flags2 & MF2_FIREDAMAGE)
		{
			if(target->type == MobjType::HexenFighterBoss
				|| target->type == MobjType::HexenClericBoss
				|| target->type == MobjType::HexenMageBoss)
			{
				switch(target->type)
				{
					case MobjType::HexenFighterBoss:
						S_StartMobjSound(target, SfxId::HexenPlayerFighterBurnDeath);
						P_SetMobjState(target, StateId::HexenPlayFFdth1);
						return;
					case MobjType::HexenClericBoss:
						S_StartMobjSound(target, SfxId::HexenPlayerClericBurnDeath);
						P_SetMobjState(target, StateId::HexenPlayCFdth1);
						return;
					case MobjType::HexenMageBoss:
						S_StartMobjSound(target, SfxId::HexenPlayerMageBurnDeath);
						P_SetMobjState(target, StateId::HexenPlayMFdth1);
						return;
					default:
						break;
				}
			}
			else if(target->type == MobjType::HexenTreedestructible)
			{
				P_SetMobjState(target, StateId::HexenZtreedesX1);
				target->height = 24 * FRACUNIT;
				S_StartMobjSound(target, SfxId::HexenTreeExplode);
				return;
			}
		}
		if(target->flags2 & MF2_ICEDAMAGE)
		{
			target->flags |= MF_ICECORPSE;
			switch(target->type)
			{
				case MobjType::HexenBishop:
					P_SetMobjState(target, StateId::HexenBishopIce);
					return;
				case MobjType::HexenCentaur:
				case MobjType::HexenCentaurleader:
					P_SetMobjState(target, StateId::HexenCentaurIce);
					return;
				case MobjType::HexenDemon:
				case MobjType::HexenDemon2:
					P_SetMobjState(target, StateId::HexenDemonIce);
					return;
				case MobjType::HexenSerpent:
				case MobjType::HexenSerpentleader:
					P_SetMobjState(target, StateId::HexenSerpentIce);
					return;
				case MobjType::HexenWraith:
				case MobjType::HexenWraithb:
					P_SetMobjState(target, StateId::HexenWraithIce);
					return;
				case MobjType::HexenEttin:
					P_SetMobjState(target, StateId::HexenEttinIce1);
					return;
				case MobjType::HexenFiredemon:
					P_SetMobjState(target, StateId::HexenFiredIce1);
					return;
				case MobjType::HexenFighterBoss:
					P_SetMobjState(target, StateId::HexenFighterIce);
					return;
				case MobjType::HexenClericBoss:
					P_SetMobjState(target, StateId::HexenClericIce);
					return;
				case MobjType::HexenMageBoss:
					P_SetMobjState(target, StateId::HexenMageIce);
					return;
				case MobjType::HexenPig:
					P_SetMobjState(target, StateId::HexenPigIce);
					return;
				default:
					target->flags &= ~MF_ICECORPSE;
					break;
			}
		}

		if(target->type == MobjType::HexenMinotaur)
		{
			mobj_t* master = target->special1.m;
			if(master->health > 0)
			{
				if(!ActiveMinotaur(master->player))
				{
					master->player->powers[std::to_underlying(PowerType::Minotaur)] = 0;
				}
			}
		}
		else if(target->type == MobjType::HexenTreedestructible)
		{
			target->height = 24 * FRACUNIT;
		}
		if(target->health < -(P_MobjSpawnHealth(target) >> 1)
			&& target->info->xdeathstate != StateId::Null)
		{
			// Extreme death
			P_SetMobjState(target, target->info->xdeathstate);
		}
		else
		{
			// Normal death
			if((target->type == MobjType::HexenFiredemon) &&
				(target->z <= target->floorz + 2 * FRACUNIT) &&
				(target->info->xdeathstate != StateId::Null))
			{
				// This is to fix the imps' staying in fall state
				P_SetMobjState(target, target->info->xdeathstate);
			}
			else
			{
				P_SetMobjState(target, target->info->deathstate);
			}
		}
	}
	else
	{
		xdeath_limit = heretic ? (P_MobjSpawnHealth(target) >> 1) : P_MobjSpawnHealth(target);
		if(target->health < -xdeath_limit && target->info->xdeathstate != StateId::Null)
			P_SetMobjState(target, target->info->xdeathstate);
		else
			P_SetMobjState(target, target->info->deathstate);
	}

	target->tics -= P_Random(RandomClass::Killtics) & 3;

	if(raven) return;

	if(target->tics < 1)
		target->tics = 1;

	// In Chex Quest, monsters don't drop items.
	if(gamemission == GameMission::TcChex)
	{
		return;
	}

	// Drop stuff.
	// This determines the kind of object spawned
	// during the death frame of a thing.

	if(target->info->droppeditem != MobjType::Null)
	{
		item = target->info->droppeditem;
	}
	else return;

	mo = P_SpawnMobj(target->x, target->y,ONFLOORZ, static_cast<MobjType>(item));
	mo->flags |= MF_DROPPED; // special versions of items

	if(target->momx == 0 && target->momy == 0)
	{
		target->flags |= MF_FOREGROUND;
	}
}

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//

static dboolean P_InfightingImmune(mobj_t* target, mobj_t* source)
{
	return // not default behaviour, and same group
		mobjinfo[std::to_underlying(target->type)].infighting_group != std::to_underlying(InfightingGroup::Default) &&
		mobjinfo[std::to_underlying(target->type)].infighting_group == mobjinfo[std::to_underlying(source->type)].infighting_group;
}

static dboolean P_MorphMonster(mobj_t* actor);

void P_DamageMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage)
{
	player_t* player;
	dboolean justhit = false; /* killough 11/98 */

	/* killough 8/31/98: allow bouncers to take damage */
	if(!(target->flags & (MF_SHOOTABLE | MF_BOUNCES)))
		return; // shouldn't happen...

	if(target->health <= 0)
	{
		// hexen
		if(inflictor && inflictor->flags2 & MF2_ICEDAMAGE)
		{
			return;
		}
		else if(target->flags & MF_ICECORPSE) // frozen
		{
			target->tics = 1;
			target->momx = target->momy = 0;
		}
		return;
	}

	// hexen has a different order of checks
	if(hexen)
	{
		if((target->flags2 & MF2_INVULNERABLE) && damage < 10000)
		{
			// mobj is invulnerable
			if(target->player)
				return; // for player, no exceptions
			if(inflictor)
			{
				switch(inflictor->type)
				{
					// These inflictors aren't foiled by invulnerability
					case MobjType::HexenHolyFx:
					case MobjType::HexenPoisoncloud:
					case MobjType::HexenFirebomb:
						break;
					default:
						return;
				}
			}
			else
			{
				return;
			}
		}
		if(target->player)
		{
			if(damage < 1000 && ((target->player->cheats & CF_GODMODE)
				|| target->player->powers[std::to_underlying(PowerType::Invulnerability)]))
			{
				return;
			}
		}
	}

	if(target->flags & MF_SKULLFLY)
	{
		if(heretic && target->type == MobjType::HereticMinotaur) return;
		target->momx = target->momy = target->momz = 0;
	}

	if(target->flags2 & MF2_DORMANT)
	{
		// Invulnerable, and won't wake up
		return;
	}

	player = target->player;
	if(player && skill_info.damage_factor)
		damage = FixedMul(damage, skill_info.damage_factor);

	// Special damage types
	if(heretic && inflictor)
	{
		switch(inflictor->type)
		{
			case MobjType::HereticEggfx:
				if(player)
				{
					P_ChickenMorphPlayer(player);
				}
				else
				{
					P_ChickenMorph(target);
				}
				return; // Always return
			case MobjType::HereticWhirlwind:
				P_TouchWhirlwind(target);
				return;
			case MobjType::HereticMinotaur:
				if(inflictor->flags & MF_SKULLFLY)
				{
					// Slam only when in charge mode
					P_MinotaurSlam(inflictor, target);
					return;
				}
				break;
			case MobjType::HereticMacefx4: // Death ball
				if((target->flags2 & MF2_BOSS) || target->type == MobjType::HereticHead)
				{
					// Don't allow cheap boss kills
					break;
				}
				else if(target->player)
				{
					// Player specific checks
					if(target->player->powers[std::to_underlying(PowerType::Invulnerability)])
					{
						// Can't hurt invulnerable players
						break;
					}
					if(P_AutoUseChaosDevice(target->player))
					{
						// Player was saved using chaos device
						return;
					}
				}
				damage = 10000; // Something's gonna die
				break;
			case MobjType::HereticPhoenixfx2: // Flame thrower
				if(target->player && P_Random(RandomClass::Heretic) < 128)
				{
					// Freeze player for a bit
					target->reactiontime += 4;
				}
				break;
			case MobjType::HereticRainplr1: // Rain missiles
			case MobjType::HereticRainplr2:
			case MobjType::HereticRainplr3:
			case MobjType::HereticRainplr4:
				if(target->flags2 & MF2_BOSS)
				{
					// Decrease damage for bosses
					damage = (P_Random(RandomClass::Heretic) & 7) + 1;
				}
				break;
			case MobjType::HereticHornrodfx2:
			case MobjType::HereticPhoenixfx1:
				if(target->type == MobjType::HereticSorcerer2 && P_Random(RandomClass::Heretic) < 96)
				{
					// D'Sparil teleports away
					P_DSparilTeleport(target);
					return;
				}
				break;
			case MobjType::HereticBlasterfx1:
			case MobjType::HereticRipper:
				if(target->type == MobjType::HereticHead)
				{
					// Less damage to Ironlich bosses
					damage = P_Random(RandomClass::Heretic) & 1;
					if(!damage)
					{
						return;
					}
				}
				break;
			default:
				break;
		}
	}
	else if(hexen && inflictor)
	{
		switch(inflictor->type)
		{
			case MobjType::HexenEggfx:
				if(player)
				{
					P_MorphPlayer(player);
				}
				else
				{
					P_MorphMonster(target);
				}
				return; // Always return
			case MobjType::HexenTelotherFx1:
			case MobjType::HexenTelotherFx2:
			case MobjType::HexenTelotherFx3:
			case MobjType::HexenTelotherFx4:
			case MobjType::HexenTelotherFx5:
				if((target->flags & MF_COUNTKILL) &&
					(target->type != MobjType::HexenSerpent) &&
					(target->type != MobjType::HexenSerpentleader) &&
					(!(target->flags2 & MF2_BOSS)))
				{
					P_TeleportOther(target);
				}
				return;
			case MobjType::HexenMinotaur:
				if(inflictor->flags & MF_SKULLFLY)
				{
					// Slam only when in charge mode
					P_MinotaurSlam(inflictor, target);
					return;
				}
				break;
			case MobjType::HexenBishFx:
				// Bishops are just too nasty
				damage >>= 1;
				break;
			case MobjType::HexenShardfx1:
				switch(inflictor->special2.i)
				{
					case 3:
						damage <<= 3;
						break;
					case 2:
						damage <<= 2;
						break;
					case 1:
						damage <<= 1;
						break;
					default:
						break;
				}
				break;
			case MobjType::HexenCstaffMissile:
				// Cleric Serpent Staff does poison damage
				if(target->player)
				{
					P_PoisonPlayer(target->player, source, 20);
					damage >>= 1;
				}
				break;
			case MobjType::HexenIceguyFx2:
				damage >>= 1;
				break;
			case MobjType::HexenPoisondart:
				if(target->player)
				{
					P_PoisonPlayer(target->player, source, 20);
					damage >>= 1;
				}
				break;
			case MobjType::HexenPoisoncloud:
				if(target->player)
				{
					if(target->player->poisoncount < 4)
					{
						P_PoisonDamage(target->player, source, 15 + (P_Random(RandomClass::Hexen) & 15), false); // Don't play painsound
						P_PoisonPlayer(target->player, source, 50);
						S_StartMobjSound(target, SfxId::HexenPlayerPoisoncough);
					}
					return;
				}
				else if(!(target->flags & MF_COUNTKILL))
				{
					// only damage monsters/players with the poison cloud
					return;
				}
				break;
			case MobjType::HexenFswordMissile:
				if(target->player)
				{
					damage -= damage >> 2;
				}
				break;
			default:
				break;
		}
	}

	// Some close combat weapons should not
	// inflict thrust and push the victim out of reach,
	// thus kick away unless using the chainsaw.

	if(
		inflictor &&
		!(target->flags & MF_NOCLIP) && // hexen_note: not done in hexen, does it matter?
		!(
			source &&
			source->player &&
			(hexen || weaponinfo[std::to_underlying(source->player->readyweapon)].flags & WPF_NOTHRUST)
		) &&
		!(inflictor->flags2 & MF2_NODMGTHRUST)
	)
	{
		unsigned ang = R_PointToAngle2(inflictor->x, inflictor->y,
			target->x, target->y);

		fixed_t thrust = damage * (FRACUNIT >> 3) * g_thrust_factor / target->info->mass;

		// make fall forwards sometimes
		if(damage < 40 && damage > target->health
			&& target->z - inflictor->z > 64 * FRACUNIT
			&& P_Random(RandomClass::Damagemobj) & 1)
		{
			ang += ANG180;
			thrust *= 4;
		}

		ang >>= ANGLETOFINESHIFT;

		if(source && source->player && (source == inflictor)
			&& source->player->powers[std::to_underlying(PowerType::WeaponLevel2)]
			&& source->player->readyweapon == WeaponType::Staff)
		{
			// Staff power level 2
			target->momx += FixedMul(10 * FRACUNIT, finecosine[ang]);
			target->momy += FixedMul(10 * FRACUNIT, finesine[ang]);
			if(!(target->flags & MF_NOGRAVITY))
			{
				target->momz += 5 * FRACUNIT;
			}
		}
		else
		{
			target->momx += FixedMul(thrust, finecosine[ang]);
			target->momy += FixedMul(thrust, finesine[ang]);
		}

		/* killough 11/98: thrust objects hanging off ledges */
		if((target->intflags & MobjIntFlag::Falling) != MobjIntFlag{} && target->gear >= MAXGEAR)
			target->gear = 0;
	}

	// player specific
	if(player)
	{
		// end of game hell hack
		if(!raven && target->subsector->sector->special == 11 && damage >= target->health)
			damage = target->health - 1;

		// Below certain threshold,
		// ignore damage in GOD mode, or with INVUL power.
		// killough 3/26/98: make god mode 100% god mode in non-compat mode

		if(
			!hexen &&
			(damage < 1000 || (!comp[std::to_underlying(CompOption::God)] && (player->cheats & CF_GODMODE))) &&
			(player->cheats & CF_GODMODE || player->powers[std::to_underlying(PowerType::Invulnerability)])
		)
			return;

		if(hexen)
		{
			int i;
			int saved;
			fixed_t savedPercent = pclass[std::to_underlying(player->pclass)].auto_armor_save
				+ player->armorpoints[std::to_underlying(ArmorType::Armor)]
				+ player->armorpoints[std::to_underlying(ArmorType::Shield)]
				+ player->armorpoints[std::to_underlying(ArmorType::Helmet)]
				+ player->armorpoints[std::to_underlying(ArmorType::Amulet)];
			if(savedPercent)
			{
				// armor absorbed some damage
				if(savedPercent > 100 * FRACUNIT)
				{
					savedPercent = 100 * FRACUNIT;
				}
				for(i = 0; i < std::to_underlying(ArmorType::Count); i++)
				{
					if(player->armorpoints[i])
					{
						player->armorpoints[i] -= FixedDiv(
							FixedMul(damage << FRACBITS, pclass[std::to_underlying(player->pclass)].armor_increment[i]),
							300 * FRACUNIT
						);
						if(player->armorpoints[i] < 2 * FRACUNIT)
						{
							player->armorpoints[i] = 0;
						}
					}
				}
				saved = FixedDiv(
					FixedMul(damage << FRACBITS, savedPercent),
					100 * FRACUNIT
				);
				if(saved > savedPercent * 2)
				{
					saved = savedPercent * 2;
				}
				damage -= saved >> FRACBITS;
			}
		}
		else
		{
			if(player->armortype)
			{
				int saved;

				if(heretic)
					saved = player->armortype == 1 ? (damage >> 1) : (damage >> 1) + (damage >> 2);
				else
					saved = player->armortype == 1 ? damage / 3 : damage / 2;

				if(player->armorpoints[std::to_underlying(ArmorType::Armor)] <= saved)
				{
					// armor is used up
					saved = player->armorpoints[std::to_underlying(ArmorType::Armor)];
					player->armortype = 0;
				}
				player->armorpoints[std::to_underlying(ArmorType::Armor)] -= saved;
				damage -= saved;
			}
		}

		if(
			raven &&
			damage >= player->health &&
			(skill_info.flags & SI_AUTO_USE_HEALTH || deathmatch) &&
			!player->chickenTics && !player->morphTics
		)
		{
			// Try to use some inventory health
			P_AutoUseHealth(player, damage - player->health + 1);
		}

		player->health -= damage; // mirror mobj health here for Dave
		if(player->health < 0)
			player->health = 0;

		player->attacker = source;
		player->damagecount += damage; // add damage after armor / invuln

		if(player->damagecount > 100)
			player->damagecount = 100; // teleport stomp does 10k points...

		if(raven && player == &players[consoleplayer])
		{
			SB_PaletteFlash(false);
		}
	}

	dsda_WatchDamage(target, inflictor, source, damage);

	// do the damage
	target->health -= damage;
	if(target->health <= 0)
	{
		if(heretic)
		{
			target->special1.i = damage;
			if(target->type == MobjType::HereticPod && source && source->type != MobjType::HereticPod)
			{
				// Make sure players get frags for chain-reaction kills
				P_SetTarget(&target->target, source);
			}
			if(player && inflictor && !player->chickenTics)
			{
				// Check for flame death
				if((inflictor->flags2 & MF2_FIREDAMAGE)
					|| ((inflictor->type == MobjType::HereticPhoenixfx1)
						&& (target->health > -50) && (damage > 25)))
				{
					target->flags2 |= MF2_FIREDAMAGE;
				}
			}
		}
		else if(hexen)
		{
			if(inflictor)
			{
				// check for special fire damage or ice damage deaths
				if(inflictor->flags2 & MF2_FIREDAMAGE)
				{
					if(player && !player->morphTics)
					{
						// Check for flame death
						if(target->health > -50 && damage > 25)
						{
							target->flags2 |= MF2_FIREDAMAGE;
						}
					}
					else
					{
						target->flags2 |= MF2_FIREDAMAGE;
					}
				}
				else if(inflictor->flags2 & MF2_ICEDAMAGE)
				{
					target->flags2 |= MF2_ICEDAMAGE;
				}
			}
			if(source && (source->type == MobjType::HexenMinotaur))
			{
				// Minotaur's kills go to his master
				mobj_t* master = source->special1.m;
				// Make sure still alive and not a pointer to fighter head
				if(master->player && (master->player->mo == master))
				{
					source = master;
				}
			}
			if(source && (source->player) &&
				(source->player->readyweapon == WeaponType::Fourth))
			{
				// Always extreme death from fourth weapon
				target->health = -5000;
			}
		}

		P_KillMobj(source, target);
		return;
	}

	// killough 9/7/98: keep track of targets so that friends can help friends
	if(mbf_features)
	{
		/* If target is a player, set player's target to source,
		* so that a friend can tell who's hurting a player
		*/
		if(player) P_SetTarget(&target->target, source);

		/* killough 9/8/98:
		* If target's health is less than 50%, move it to the front of its list.
		* This will slightly increase the chances that enemies will choose to
		* "finish it off", but its main purpose is to alert friends of danger.
		*/
		if(target->health * 2 < P_MobjSpawnHealth(target))
		{
			thinker_t* cap = &thinkerclasscap[std::to_underlying(target->flags & MF_FRIEND ? ThinkerClass::Friends : ThinkerClass::Enemies)];
			(target->thinker.cprev->cnext = target->thinker.cnext)->cprev =
				target->thinker.cprev;
			(target->thinker.cnext = cap->cnext)->cprev = &target->thinker;
			(target->thinker.cprev = cap)->cnext = &target->thinker;
		}
	}

	if(!(skill_info.flags & SI_NO_PAIN) &&
		P_Random(RandomClass::Painchance) < target->info->painchance &&
		!(target->flags & MF_SKULLFLY)) //killough 11/98: see below
	{
		if(hexen && inflictor && inflictor->type >= MobjType::HexenLightningFloor &&
			inflictor->type <= MobjType::HexenLightningZap)
		{
			if(P_Random(RandomClass::Hexen) < 96)
			{
				target->flags |= MF_JUSTHIT; // fight back!
				P_SetMobjState(target, target->info->painstate);
			}
			else
			{
				// "electrocute" the target
				target->frame |= FF_FULLBRIGHT;
				if(target->flags & MF_COUNTKILL && P_Random(RandomClass::Hexen) < 128
					&& !S_GetSoundPlayingInfo(target, SfxId::HexenPuppybeat))
				{
					if((target->type == MobjType::HexenCentaur) ||
						(target->type == MobjType::HexenCentaurleader) ||
						(target->type == MobjType::HexenEttin))
					{
						S_StartMobjSound(target, SfxId::HexenPuppybeat);
					}
				}
			}
		}
		else
		{
			if(mbf_features)
				justhit = true;
			else
				target->flags |= MF_JUSTHIT; // fight back!

			P_SetMobjState(target, target->info->painstate);

			if(hexen && inflictor && inflictor->type == MobjType::HexenPoisoncloud)
			{
				if(target->flags & MF_COUNTKILL && P_Random(RandomClass::Hexen) < 128
					&& !S_GetSoundPlayingInfo(target, SfxId::HexenPuppybeat))
				{
					if((target->type == MobjType::HexenCentaur) ||
						(target->type == MobjType::HexenCentaurleader) ||
						(target->type == MobjType::HexenEttin))
					{
						S_StartMobjSound(target, SfxId::HexenPuppybeat);
					}
				}
			}
		}
	}

	target->reactiontime = 0; // we're awake now...

	/* killough 9/9/98: cleaned up, made more consistent: */
	//e6y: Monsters could commit suicide in Doom v1.2 if they damaged themselves by exploding a barrel
	if(
		source &&
		(source != target || compatibility_level == CompLevel::Doom12) &&
		!(source->flags2 & MF2_DMGIGNORED) &&
		(!target->threshold || target->flags2 & MF2_NOTHRESHOLD) &&
		((source->flags ^ target->flags) & MF_FRIEND || monster_infighting || !mbf_features) &&
		!(
			raven && (
				source->flags2 & MF2_BOSS ||
				(target->type == MobjType::HereticSorcerer2 && source->type == MobjType::HereticWizard) ||
				target->type == MobjType::HexenBishop ||
				target->type == MobjType::HexenMinotaur ||
				(target->type == MobjType::HexenCentaur && source->type == MobjType::HexenCentaurleader) ||
				(target->type == MobjType::HexenCentaurleader && source->type == MobjType::HexenCentaur)
			)
		) &&
		!P_InfightingImmune(target, source)
	)
	{
		/* if not intent on another player, chase after this one
		*
		* killough 2/15/98: remember last enemy, to prevent
		* sleeping early; 2/21/98: Place priority on players
		* killough 9/9/98: cleaned up, made more consistent:
		*/

		if(
			!target->lastenemy ||
			target->lastenemy->health <= 0 ||
			(
				!mbf_features ? !target->lastenemy->player : !((target->flags ^ target->lastenemy->flags) & MF_FRIEND) && target->target != source
			)
		) // remember last enemy - killough
			P_SetTarget(&target->lastenemy, target->target);

		P_SetTarget(&target->target, source); // killough 11/98
		target->threshold = BASETHRESHOLD;
		if(target->state == &states[std::to_underlying(target->info->spawnstate)]
			&& target->info->seestate != g_s_null)
			P_SetMobjState(target, target->info->seestate);
	}

	/* killough 11/98: Don't attack a friend, unless hit by that friend.
	* cph 2006/04/01 - implicitly this is only if mbf_features */
	if(!demo_compatibility) //e6y
		if(justhit && (target->target == source || !target->target ||
			!(target->flags & target->target->flags & MF_FRIEND)))
			target->flags |= MF_JUSTHIT; // fight back!
}

// heretic

#include "p_user.hpp"

#define CHICKENTICS (40*TICRATE)

extern "C" void A_RestoreArtifact(mobj_t* arti)
{
	arti->flags |= MF_SPECIAL;
	P_SetMobjState(arti, arti->info->spawnstate);
	S_StartMobjSound(arti, g_sfx_respawn);
}

extern "C" void A_RestoreSpecialThing1(mobj_t* thing)
{
	if(thing->type == MobjType::HereticWmace)
	{
		// Do random mace placement
		P_RepositionMace(thing);
	}
	thing->flags2 &= ~MF2_DONTDRAW;
	S_StartMobjSound(thing, g_sfx_respawn);
}

extern "C" void A_RestoreSpecialThing2(mobj_t* thing)
{
	thing->flags |= MF_SPECIAL;
	P_SetMobjState(thing, thing->info->spawnstate);
}

// heretic

void P_SetMessage(player_t* player, const char* message, dboolean ultmsg)
{
	dsda_AddPlayerMessage(message, player);
	player->yellowMessage = false;
}

static void Heretic_P_TouchSpecialThing(mobj_t* special, mobj_t* toucher)
{
	int i;
	player_t* player;
	fixed_t delta;
	SfxId sound;

	delta = special->z - toucher->z;
	if(delta > toucher->height || delta < -32 * FRACUNIT)
	{
		// Out of reach
		return;
	}
	if(toucher->health <= 0)
	{
		// Toucher is dead
		return;
	}
	sound = SfxId::HereticItemup;
	player = toucher->player;

	switch(special->sprite)
	{
		// Items
		case SpriteId::HereticPtn1: // Item_HealingPotion
			if(!P_GiveBody(player, 10))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_ITEMHEALTH, false);
			break;
		case SpriteId::HereticShld: // Item_Shield1
			if(!P_GiveArmor(player, 1))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_ITEMSHIELD1, false);
			break;
		case SpriteId::HereticShd2: // Item_Shield2
			if(!P_GiveArmor(player, 2))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_ITEMSHIELD2, false);
			break;
		case SpriteId::HereticBagh: // Item_BagOfHolding
			if(!player->backpack)
			{
				for(i = 0; i < std::to_underlying(AmmoType::Count); i++)
				{
					player->maxammo[i] *= 2;
				}
				player->backpack = true;
			}
			P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::GoldWand), AMMO_GWND_WIMPY);
			P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Blaster), AMMO_BLSR_WIMPY);
			P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Crossbow), AMMO_CBOW_WIMPY);
			P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::SkullRod), AMMO_SKRD_WIMPY);
			P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::PhoenixRod), AMMO_PHRD_WIMPY);
			P_SetMessage(player, HERETIC_TXT_ITEMBAGOFHOLDING, false);
			break;
		case SpriteId::HereticSpmp: // Item_SuperMap
			if(!P_GivePower(player, PowerType::AllMap))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_ITEMSUPERMAP, false);
			break;

		// Keys
		case SpriteId::HereticBkyy: // Key_Blue
			if(!player->cards[std::to_underlying(Card::KeyBlue)])
			{
				P_SetMessage(player, HERETIC_TXT_GOTBLUEKEY, false);
			}
			P_GiveCard(player, Card::KeyBlue);
			sound = SfxId::HereticKeyup;
			if(!netgame)
			{
				break;
			}
			return;
		case SpriteId::HereticCkyy: // Key_Yellow
			if(!player->cards[std::to_underlying(Card::KeyYellow)])
			{
				P_SetMessage(player, HERETIC_TXT_GOTYELLOWKEY, false);
			}
			sound = SfxId::HereticKeyup;
			P_GiveCard(player, Card::KeyYellow);
			if(!netgame)
			{
				break;
			}
			return;
		case SpriteId::HereticAkyy: // Key_Green
			if(!player->cards[std::to_underlying(Card::KeyGreen)])
			{
				P_SetMessage(player, HERETIC_TXT_GOTGREENKEY, false);
			}
			sound = SfxId::HereticKeyup;
			P_GiveCard(player, Card::KeyGreen);
			if(!netgame)
			{
				break;
			}
			return;

		// Artifacts
		case SpriteId::HereticPtn2: // Arti_HealingPotion
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Health), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTIHEALTH, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticSoar: // Arti_Fly
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Fly), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTIFLY, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticInvu: // Arti_Invulnerability
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Invulnerability), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTIINVULNERABILITY, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticPwbk: // Arti_TomeOfPower
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::TomeOfPower), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTITOMEOFPOWER, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticInvs: // Arti_Invisibility
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Invisibility), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTIINVISIBILITY, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticEggc: // Arti_Egg
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Egg), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTIEGG, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticSphl: // Arti_SuperHealth
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::SuperHealth), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTISUPERHEALTH, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticTrch: // Arti_Torch
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Torch), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTITORCH, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticFbmb: // Arti_FireBomb
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Firebomb), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTIFIREBOMB, false);
				P_SetDormantArtifact(special);
			}
			return;
		case SpriteId::HereticAtlp: // Arti_Teleport
			if(P_GiveArtifact(player, static_cast<ArtiType>(ArtiType::Teleport), special))
			{
				P_SetMessage(player, HERETIC_TXT_ARTITELEPORT, false);
				P_SetDormantArtifact(special);
			}
			return;

		// Ammo
		case SpriteId::HereticAmg1: // Ammo_GoldWandWimpy
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::GoldWand), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOGOLDWAND1, false);
			break;
		case SpriteId::HereticAmg2: // Ammo_GoldWandHefty
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::GoldWand), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOGOLDWAND2, false);
			break;
		case SpriteId::HereticAmm1: // Ammo_MaceWimpy
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Mace), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOMACE1, false);
			break;
		case SpriteId::HereticAmm2: // Ammo_MaceHefty
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Mace), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOMACE2, false);
			break;
		case SpriteId::HereticAmc1: // Ammo_CrossbowWimpy
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Crossbow), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOCROSSBOW1, false);
			break;
		case SpriteId::HereticAmc2: // Ammo_CrossbowHefty
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Crossbow), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOCROSSBOW2, false);
			break;
		case SpriteId::HereticAmb1: // Ammo_BlasterWimpy
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Blaster), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOBLASTER1, false);
			break;
		case SpriteId::HereticAmb2: // Ammo_BlasterHefty
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::Blaster), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOBLASTER2, false);
			break;
		case SpriteId::HereticAms1: // Ammo_SkullRodWimpy
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::SkullRod), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOSKULLROD1, false);
			break;
		case SpriteId::HereticAms2: // Ammo_SkullRodHefty
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::SkullRod), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOSKULLROD2, false);
			break;
		case SpriteId::HereticAmp1: // Ammo_PhoenixRodWimpy
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::PhoenixRod), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOPHOENIXROD1, false);
			break;
		case SpriteId::HereticAmp2: // Ammo_PhoenixRodHefty
			if(!P_GiveAmmo(player, static_cast<AmmoType>(AmmoType::PhoenixRod), special->health))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_AMMOPHOENIXROD2, false);
			break;

		// Weapons
		case SpriteId::HereticWmce: // Weapon_Mace
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Mace), false))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_WPNMACE, false);
			sound = SfxId::HereticWpnup;
			break;
		case SpriteId::HereticWbow: // Weapon_Crossbow
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Crossbow), false))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_WPNCROSSBOW, false);
			sound = SfxId::HereticWpnup;
			break;
		case SpriteId::HereticWbls: // Weapon_Blaster
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Blaster), false))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_WPNBLASTER, false);
			sound = SfxId::HereticWpnup;
			break;
		case SpriteId::HereticWskl: // Weapon_SkullRod
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::SkullRod), false))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_WPNSKULLROD, false);
			sound = SfxId::HereticWpnup;
			break;
		case SpriteId::HereticWphx: // Weapon_PhoenixRod
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::PhoenixRod), false))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_WPNPHOENIXROD, false);
			sound = SfxId::HereticWpnup;
			break;
		case SpriteId::HereticWgnt: // Weapon_Gauntlets
			if(!P_GiveWeapon(player, static_cast<WeaponType>(WeaponType::Gauntlets), false))
			{
				return;
			}
			P_SetMessage(player, HERETIC_TXT_WPNGAUNTLETS, false);
			sound = SfxId::HereticWpnup;
			break;
		default:
			I_Error("Heretic_P_TouchSpecialThing: Unknown gettable thing");
	}
	if(special->flags & MF_COUNTITEM)
	{
		player->itemcount++;
	}
	if(deathmatch && !(special->flags & MF_DROPPED))
	{
		P_HideSpecialThing(special);
	}
	else
	{
		P_RemoveMobj(special);
	}
	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		S_StartVoidSound(sound);
		SB_PaletteFlash(false);
	}
}

dboolean P_GiveArtifact(player_t* player, ArtiType arti, mobj_t* mo)
{
	int i;
	dboolean slidePointer;

	slidePointer = false;
	i = 0;
	while(player->inventory[i].type != std::to_underlying(arti) && i < player->inventorySlotNum)
	{
		i++;
	}
	if(i == player->inventorySlotNum)
	{
		if(hexen && arti < ArtiType::HexenFirstpuzzitem)
		{
			i = 0;
			while(player->inventory[i].type < std::to_underlying(ArtiType::HexenFirstpuzzitem)
				&& i < player->inventorySlotNum)
			{
				i++;
			}
			if(i != player->inventorySlotNum)
			{
				int j;
				for(j = player->inventorySlotNum; j > i; j--)
				{
					player->inventory[j].count =
						player->inventory[j - 1].count;
					player->inventory[j].type = player->inventory[j - 1].type;
					slidePointer = true;
				}
			}
		}
		player->inventory[i].count = 1;
		player->inventory[i].type = std::to_underlying(arti);
		player->inventorySlotNum++;
	}
	else
	{
		if(hexen && arti >= ArtiType::HexenFirstpuzzitem && netgame && !deathmatch)
		{
			// Can't carry more than 1 puzzle item in coop netplay
			return false;
		}
		if(player->inventory[i].count >= g_arti_limit)
		{
			// Player already has 16 of this item
			return (false);
		}
		player->inventory[i].count++;
	}
	if(player->artifactCount == 0)
	{
		player->readyArtifact = arti;
	}
	else if(player == &players[consoleplayer] && slidePointer && i <= inv_ptr)
	{
		inv_ptr++;
		curpos++;
		if(curpos > 6)
		{
			curpos = 6;
		}
	}
	player->artifactCount++;
	if(mo && (mo->flags & MF_COUNTITEM))
	{
		player->itemcount++;
	}
	return (true);
}

void P_SetDormantArtifact(mobj_t* arti)
{
	arti->flags &= ~MF_SPECIAL;
	if(deathmatch && (arti->type != MobjType::HereticArtiinvulnerability)
		&& (arti->type != MobjType::HereticArtiinvisibility))
	{
		P_SetMobjState(arti, StateId::HereticDormantarti1);
	}
	else
	{
		// Don't respawn
		P_SetMobjState(arti, StateId::HereticDeadarti1);
	}
	S_StartMobjSound(arti, SfxId::HereticArtiup);
}

int GetWeaponAmmo[std::to_underlying(WeaponType::Count)] = {
	0,  // staff
	25, // gold wand
	10, // crossbow
	30, // blaster
	50, // skull rod
	2,  // phoenix rod
	50, // mace
	0,  // gauntlets
	0   // beak
};

int WeaponValue[] = {
	1, // staff
	3, // goldwand
	4, // crossbow
	5, // blaster
	6, // skullrod
	7, // phoenixrod
	8, // mace
	2, // gauntlets
	0  // beak
};

dboolean Heretic_P_GiveWeapon(player_t* player, WeaponType weapon)
{
	dboolean gaveAmmo;
	dboolean gaveWeapon;

	if(netgame && !deathmatch)
	{
		// Cooperative net-game
		if(player->weaponowned[std::to_underlying(weapon)])
		{
			return (false);
		}
		player->bonuscount += BONUSADD;
		player->weaponowned[std::to_underlying(weapon)] = true;
		P_GiveAmmo(player, static_cast<AmmoType>(wpnlev1info[std::to_underlying(weapon)].ammo), GetWeaponAmmo[std::to_underlying(weapon)]);
		P_AutoSwitchWeapon(player, static_cast<WeaponType>(weapon));
		if(player == &players[consoleplayer])
		{
			S_StartVoidSound(SfxId::HereticWpnup);
		}
		return (false);
	}
	gaveAmmo = P_GiveAmmo(player, static_cast<AmmoType>(wpnlev1info[std::to_underlying(weapon)].ammo),
		GetWeaponAmmo[std::to_underlying(weapon)]);
	if(player->weaponowned[std::to_underlying(weapon)])
	{
		gaveWeapon = false;
	}
	else
	{
		gaveWeapon = true;
		player->weaponowned[std::to_underlying(weapon)] = true;
		if(WeaponValue[std::to_underlying(weapon)] > WeaponValue[std::to_underlying(player->readyweapon)])
		{
			// Only switch to more powerful weapons
			P_AutoSwitchWeapon(player, static_cast<WeaponType>(weapon));
		}
	}
	return (gaveWeapon || gaveAmmo);
}

void P_HideSpecialThing(mobj_t* thing)
{
	thing->flags &= ~MF_SPECIAL;
	thing->flags2 |= MF2_DONTDRAW;
	P_SetMobjState(thing, static_cast<StateId>(g_hide_state));
}

void P_MinotaurSlam(mobj_t* source, mobj_t* target)
{
	angle_t angle;
	fixed_t thrust;

	angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
	angle >>= ANGLETOFINESHIFT;
	thrust = 16 * FRACUNIT + (P_Random(RandomClass::Heretic) << 10);
	target->momx += FixedMul(thrust, finecosine[angle]);
	target->momy += FixedMul(thrust, finesine[angle]);
	if(hexen)
	{
		P_DamageMobj(target, nullptr, source, HITDICE(4));
	}
	else
	{
		P_DamageMobj(target, nullptr, nullptr, HITDICE(6));
	}
	if(target->player)
	{
		target->reactiontime = 14 + (P_Random(RandomClass::Heretic) & 7);
	}
	source->special_args[0] = 0; // Stop charging
}

void P_TouchWhirlwind(mobj_t* target)
{
	int randVal;

	target->angle += P_SubRandom() << 20;
	target->momx += P_SubRandom() << 10;
	target->momy += P_SubRandom() << 10;
	if(leveltime & 16 && !(target->flags2 & MF2_BOSS))
	{
		randVal = P_Random(RandomClass::Heretic);
		if(randVal > 160)
		{
			randVal = 160;
		}
		target->momz += randVal << 10;
		if(target->momz > 12 * FRACUNIT)
		{
			target->momz = 12 * FRACUNIT;
		}
	}
	if(!(leveltime & 7))
	{
		P_DamageMobj(target, nullptr, nullptr, 3);
	}

	if(target->player) R_SmoothPlaying_Reset(target->player); // e6y
}

dboolean P_ChickenMorphPlayer(player_t* player)
{
	mobj_t* pmo;
	mobj_t* fog;
	mobj_t* chicken;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	angle_t angle;
	int oldFlags2;

	if(player->chickenTics)
	{
		if((player->chickenTics < CHICKENTICS - TICRATE)
			&& !player->powers[std::to_underlying(PowerType::WeaponLevel2)])
		{
			// Make a super chicken
			P_GivePower(player, PowerType::WeaponLevel2);
		}
		return (false);
	}
	if(player->powers[std::to_underlying(PowerType::Invulnerability)])
	{
		// Immune when invulnerable
		return (false);
	}
	pmo = player->mo;
	x = pmo->x;
	y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	oldFlags2 = pmo->flags2;
	P_SetMobjState(pmo, StateId::HereticFreetargmobj);
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MobjType::HereticTfog);
	S_StartMobjSound(fog, SfxId::HereticTelept);
	chicken = P_SpawnMobj(x, y, z, MobjType::HereticChicplayer);
	chicken->special1.i = std::to_underlying(player->readyweapon);
	chicken->angle = angle;
	chicken->player = player;
	player->health = chicken->health = MAXCHICKENHEALTH;
	player->mo = chicken;
	player->armorpoints[std::to_underlying(ArmorType::Armor)] = player->armortype = 0;
	player->powers[std::to_underlying(PowerType::Invisibility)] = 0;
	player->powers[std::to_underlying(PowerType::WeaponLevel2)] = 0;
	if(oldFlags2 & MF2_FLY)
	{
		chicken->flags2 |= MF2_FLY;
	}
	player->chickenTics = CHICKENTICS;
	P_ActivateBeak(player);
	return (true);
}

dboolean P_ChickenMorph(mobj_t* actor)
{
	mobj_t* fog;
	mobj_t* chicken;
	mobj_t* target;
	MobjType moType;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	angle_t angle;
	int ghost;

	if(actor->player)
	{
		return (false);
	}
	moType = actor->type;
	switch(moType)
	{
		case MobjType::HereticPod:
		case MobjType::HereticChicken:
		case MobjType::HereticHead:
		case MobjType::HereticMinotaur:
		case MobjType::HereticSorcerer1:
		case MobjType::HereticSorcerer2:
			return (false);
		default:
			break;
	}
	x = actor->x;
	y = actor->y;
	z = actor->z;
	angle = actor->angle;
	ghost = actor->flags & MF_SHADOW;
	target = actor->target;
	P_SetMobjState(actor, StateId::HereticFreetargmobj);
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MobjType::HereticTfog);
	S_StartMobjSound(fog, SfxId::HereticTelept);
	chicken = P_SpawnMobj(x, y, z, MobjType::HereticChicken);
	chicken->special2.i = std::to_underlying(moType);
	chicken->special1.i = CHICKENTICS + P_Random(RandomClass::Heretic);
	chicken->flags |= ghost;
	P_SetTarget(&chicken->target, target);
	chicken->angle = angle;
	dsda_WatchMorph(chicken);
	return (true);
}

dboolean P_AutoUseChaosDevice(player_t* player)
{
	int i;

	for(i = 0; i < player->inventorySlotNum; i++)
	{
		if(player->inventory[i].type == std::to_underlying(ArtiType::Teleport))
		{
			P_PlayerUseArtifact(player, static_cast<ArtiType>(std::to_underlying(ArtiType::Teleport)));
			player->health = player->mo->health = (player->health + 1) / 2;
			return (true);
		}
	}
	return (false);
}

void P_AutoUseHealth(player_t* player, int saveHealth)
{
	int i;
	int count;
	int normalCount = 0;
	int normalSlot = 0;
	int superCount = 0;
	int superSlot = 0;

	for(i = 0; i < player->inventorySlotNum; i++)
	{
		if(player->inventory[i].type == g_arti_health)
		{
			normalSlot = i;
			normalCount = player->inventory[i].count;
		}
		else if(player->inventory[i].type == g_arti_superhealth)
		{
			superSlot = i;
			superCount = player->inventory[i].count;
		}
	}
	if(skill_info.flags & SI_AUTO_USE_HEALTH && (normalCount * 25 >= saveHealth))
	{
		// Use quartz flasks
		count = (saveHealth + 24) / 25;
		for(i = 0; i < count; i++)
		{
			player->health += P_PlayerHealthIncrease(25);
			P_PlayerRemoveArtifact(player, normalSlot);
		}
	}
	else if(superCount * 100 >= saveHealth)
	{
		// Use mystic urns
		count = (saveHealth + 99) / 100;
		for(i = 0; i < count; i++)
		{
			player->health += P_PlayerHealthIncrease(100);
			P_PlayerRemoveArtifact(player, superSlot);
		}
	}
	else if(skill_info.flags & SI_AUTO_USE_HEALTH
		&& (superCount * 100 + normalCount * 25 >= saveHealth))
	{
		// Use mystic urns and quartz flasks
		count = (saveHealth + 24) / 25;
		saveHealth -= count * 25;
		for(i = 0; i < count; i++)
		{
			player->health += P_PlayerHealthIncrease(25);
			P_PlayerRemoveArtifact(player, normalSlot);
		}
		count = (saveHealth + 99) / 100;
		for(i = 0; i < count; i++)
		{
			player->health += P_PlayerHealthIncrease(100);
			P_PlayerRemoveArtifact(player, normalSlot);
		}
	}
	player->mo->health = player->health;
}

// hexen

#define TXT_MANA_1    "BLUE MANA"
#define TXT_MANA_2    "GREEN MANA"
#define TXT_MANA_BOTH "COMBINED MANA"

#define TXT_KEY_STEEL   "STEEL KEY"
#define TXT_KEY_CAVE    "CAVE KEY"
#define TXT_KEY_AXE     "AXE KEY"
#define TXT_KEY_FIRE    "FIRE KEY"
#define TXT_KEY_EMERALD "EMERALD KEY"
#define TXT_KEY_DUNGEON "DUNGEON KEY"
#define TXT_KEY_SILVER  "SILVER KEY"
#define TXT_KEY_RUSTED  "RUSTED KEY"
#define TXT_KEY_HORN    "HORN KEY"
#define TXT_KEY_SWAMP   "SWAMP KEY"
#define TXT_KEY_CASTLE  "CASTLE KEY"

#define TXT_ARTIINVULNERABILITY "ICON OF THE DEFENDER"
#define TXT_ARTIHEALTH          "QUARTZ FLASK"
#define TXT_ARTISUPERHEALTH     "MYSTIC URN"
#define TXT_ARTIHEALINGRADIUS   "MYSTIC AMBIT INCANT"
#define TXT_ARTISUMMON          "DARK SERVANT"
#define TXT_ARTITORCH           "TORCH"
#define TXT_ARTIEGG             "PORKALATOR"
#define TXT_ARTIFLY             "WINGS OF WRATH"
#define TXT_ARTIBLASTRADIUS     "DISC OF REPULSION"
#define TXT_ARTIPOISONBAG       "FLECHETTE"
#define TXT_ARTITELEPORTOTHER   "BANISHMENT DEVICE"
#define TXT_ARTISPEED           "BOOTS OF SPEED"
#define TXT_ARTIBOOSTMANA       "KRATER OF MIGHT"
#define TXT_ARTIBOOSTARMOR      "DRAGONSKIN BRACERS"
#define TXT_ARTITELEPORT        "CHAOS DEVICE"

#define TXT_ITEMHEALTH        "CRYSTAL VIAL"
#define TXT_ITEMBAGOFHOLDING "BAG OF HOLDING"
#define TXT_ITEMSHIELD1      "SILVER SHIELD"
#define TXT_ITEMSHIELD2      "ENCHANTED SHIELD"
#define TXT_ITEMSUPERMAP     "MAP SCROLL"

#define TXT_ARMOR1 "MESH ARMOR"
#define TXT_ARMOR2 "FALCON SHIELD"
#define TXT_ARMOR3 "PLATINUM HELMET"
#define TXT_ARMOR4 "AMULET OF WARDING"

#define TXT_WEAPON_F2 "TIMON'S AXE"
#define TXT_WEAPON_F3 "HAMMER OF RETRIBUTION"
#define TXT_WEAPON_F4 "QUIETUS ASSEMBLED"
#define TXT_WEAPON_C2 "SERPENT STAFF"
#define TXT_WEAPON_C3 "FIRESTORM"
#define TXT_WEAPON_C4 "WRAITHVERGE ASSEMBLED"
#define TXT_WEAPON_M2 "FROST SHARDS"
#define TXT_WEAPON_M3 "ARC OF DEATH"
#define TXT_WEAPON_M4 "BLOODSCOURGE ASSEMBLED"

#define TXT_QUIETUS_PIECE      "SEGMENT OF QUIETUS"
#define TXT_WRAITHVERGE_PIECE  "SEGMENT OF WRAITHVERGE"
#define TXT_BLOODSCOURGE_PIECE "SEGMENT OF BLOODSCOURGE"

const char* TextKeyMessages[] = {
	TXT_KEY_STEEL,
	TXT_KEY_CAVE,
	TXT_KEY_AXE,
	TXT_KEY_FIRE,
	TXT_KEY_EMERALD,
	TXT_KEY_DUNGEON,
	TXT_KEY_SILVER,
	TXT_KEY_RUSTED,
	TXT_KEY_HORN,
	TXT_KEY_SWAMP,
	TXT_KEY_CASTLE
};

void P_FallingDamage(player_t* player)
{
	int damage;
	int mom;
	int dist;

	mom = abs(player->mo->momz);
	dist = FixedMul(mom, 16 * FRACUNIT / 23);

	if(mom >= 63 * FRACUNIT)
	{
		// automatic death
		P_DamageMobj(player->mo, nullptr, nullptr, 10000);
		return;
	}
	damage = ((FixedMul(dist, dist) / 10) >> FRACBITS) - 24;
	if(player->mo->momz > -39 * FRACUNIT && damage > player->mo->health
		&& player->mo->health != 1)
	{
		// No-death threshold
		damage = player->mo->health - 1;
	}
	S_StartMobjSound(player->mo, SfxId::HexenPlayerLand);
	P_DamageMobj(player->mo, nullptr, nullptr, damage);
}

void P_PoisonDamage(player_t* player, mobj_t* source, int damage,
	dboolean playPainSound)
{
	mobj_t* target;
	mobj_t* inflictor;

	target = player->mo;
	inflictor = source;
	if(target->health <= 0)
	{
		return;
	}
	if(target->flags2 & MF2_INVULNERABLE && damage < 10000)
	{
		// mobj is invulnerable
		return;
	}
	if(skill_info.damage_factor)
	{
		damage = FixedMul(damage, skill_info.damage_factor);
	}
	if(damage < 1000 && ((player->cheats & CF_GODMODE)
		|| player->powers[std::to_underlying(PowerType::Invulnerability)]))
	{
		return;
	}
	if(damage >= player->health
		&& (skill_info.flags & SI_AUTO_USE_HEALTH || deathmatch) && !player->morphTics)
	{
		// Try to use some inventory health
		P_AutoUseHealth(player, damage - player->health + 1);
	}
	player->health -= damage; // mirror mobj health here for Dave
	if(player->health < 0)
	{
		player->health = 0;
	}
	player->attacker = source;

	//
	// do the damage
	//
	target->health -= damage;
	if(target->health <= 0)
	{
		// Death
		target->special1.i = damage;
		if(inflictor && !player->morphTics)
		{
			// Check for flame death
			if((inflictor->flags2 & MF2_FIREDAMAGE)
				&& (target->health > -50) && (damage > 25))
			{
				target->flags2 |= MF2_FIREDAMAGE;
			}
			if(inflictor->flags2 & MF2_ICEDAMAGE)
			{
				target->flags2 |= MF2_ICEDAMAGE;
			}
		}
		P_KillMobj(source, target);
		return;
	}
	if(!(leveltime & 63) && playPainSound)
	{
		P_SetMobjState(target, target->info->painstate);
	}
}

dboolean P_GiveMana(player_t* player, manatype_t mana, int count)
{
	int prevMana;

	if(mana == AmmoType::ManaNone || mana == AmmoType::ManaBoth)
	{
		return (false);
	}
	if((unsigned int)mana > std::to_underlying(AmmoType::ManaCount))
	{
		I_Error("P_GiveMana: bad type %i", mana);
	}
	if(player->ammo[std::to_underlying(mana)] == MAX_MANA)
	{
		return (false);
	}
	if(skill_info.ammo_factor)
	{
		count = FixedMul(count, skill_info.ammo_factor);
	}
	prevMana = player->ammo[std::to_underlying(mana)];

	player->ammo[std::to_underlying(mana)] += count;
	if(player->ammo[std::to_underlying(mana)] > MAX_MANA)
	{
		player->ammo[std::to_underlying(mana)] = MAX_MANA;
	}
	if(player->pclass == PClass::Fighter && player->readyweapon == WeaponType::Second
		&& mana == AmmoType::Mana1 && prevMana <= 0)
	{
		P_SetPsprite(player, PspNum::Weapon, StateId::HexenFaxereadyG);
	}
	return (true);
}

dboolean Hexen_P_GiveArmor(player_t* player, ArmorType armortype, int amount)
{
	int hits;
	int totalArmor;

	if(amount == -1)
	{
		hits = pclass[std::to_underlying(player->pclass)].armor_increment[std::to_underlying(armortype)];
		if(player->armorpoints[std::to_underlying(armortype)] >= hits)
		{
			return false;
		}
		else
		{
			player->armorpoints[std::to_underlying(armortype)] = hits;
		}
	}
	else
	{
		hits = amount * 5 * FRACUNIT;
		totalArmor = player->armorpoints[std::to_underlying(ArmorType::Armor)]
			+ player->armorpoints[std::to_underlying(ArmorType::Shield)]
			+ player->armorpoints[std::to_underlying(ArmorType::Helmet)]
			+ player->armorpoints[std::to_underlying(ArmorType::Amulet)]
			+ pclass[std::to_underlying(player->pclass)].auto_armor_save;
		if(totalArmor < pclass[std::to_underlying(player->pclass)].armor_max)
		{
			player->armorpoints[std::to_underlying(armortype)] += hits;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void P_SetYellowMessage(player_t* player, const char* message, dboolean ultmsg)
{
	dsda_AddPlayerMessage(message, player);
	player->yellowMessage = true;
}

void TryPickupWeapon(player_t* player, PClass weaponClass,
	WeaponType weaponType, mobj_t* weapon,
	const char* message)
{
	dboolean remove;
	dboolean gaveMana;
	dboolean gaveWeapon;

	remove = true;
	if(player->pclass != weaponClass)
	{
		// Wrong class, but try to pick up for mana
		if(netgame && !deathmatch)
		{
			// Can't pick up weapons for other classes in coop netplay
			return;
		}
		if(weaponType == WeaponType::Second)
		{
			if(!P_GiveMana(player, AmmoType::Mana1, 25))
			{
				return;
			}
		}
		else
		{
			if(!P_GiveMana(player, AmmoType::Mana2, 25))
			{
				return;
			}
		}
	}
	else if(netgame && !deathmatch)
	{
		// Cooperative net-game
		if(player->weaponowned[std::to_underlying(weaponType)])
		{
			return;
		}
		player->weaponowned[std::to_underlying(weaponType)] = true;
		if(weaponType == WeaponType::Second)
		{
			P_GiveMana(player, AmmoType::Mana1, 25);
		}
		else
		{
			P_GiveMana(player, AmmoType::Mana2, 25);
		}
		P_AutoSwitchWeapon(player, static_cast<WeaponType>(weaponType));
		remove = false;
	}
	else
	{
		// Deathmatch or single player game
		if(weaponType == WeaponType::Second)
		{
			gaveMana = P_GiveMana(player, AmmoType::Mana1, 25);
		}
		else
		{
			gaveMana = P_GiveMana(player, AmmoType::Mana2, 25);
		}
		if(player->weaponowned[std::to_underlying(weaponType)])
		{
			gaveWeapon = false;
		}
		else
		{
			gaveWeapon = true;
			player->weaponowned[std::to_underlying(weaponType)] = true;
			if(weaponType > player->readyweapon)
			{
				// Only switch to more powerful weapons
				P_AutoSwitchWeapon(player, static_cast<WeaponType>(weaponType));
			}
		}
		if(!(gaveWeapon || gaveMana))
		{
			// Player didn't need the weapon or any mana
			return;
		}
	}

	P_SetMessage(player, message, false);
	if(weapon->special)
	{
		map_format.execute_line_special(weapon->special, weapon->special_args, nullptr, 0, player->mo);
		weapon->special = 0;
	}

	if(remove && (weapon->intflags & MobjIntFlag::Fake) == MobjIntFlag{})
	{
		if(deathmatch && !(weapon->flags & MF_DROPPED))
		{
			P_HideSpecialThing(weapon);
		}
		else
		{
			P_RemoveMobj(weapon);
		}
	}

	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		S_StartVoidSound(SfxId::HexenPickupWeapon);
		SB_PaletteFlash(false);
	}
}

static void TryPickupWeaponPiece(player_t* player, PClass matchClass,
	int pieceValue, mobj_t* pieceMobj)
{
	dboolean remove;
	dboolean checkAssembled;
	dboolean gaveWeapon;
	int gaveMana;
	static const char* fourthWeaponText[] = {
		nullptr,
		TXT_WEAPON_F4,
		TXT_WEAPON_C4,
		TXT_WEAPON_M4
	};
	static const char* weaponPieceText[] = {
		nullptr,
		TXT_QUIETUS_PIECE,
		TXT_WRAITHVERGE_PIECE,
		TXT_BLOODSCOURGE_PIECE
	};
	static int pieceValueTrans[] = {
		0,                           // 0: never
		WPIECE1 | WPIECE2 | WPIECE3, // WPIECE1 (1)
		WPIECE2 | WPIECE3,           // WPIECE2 (2)
		0,                           // 3: never
		WPIECE3                      // WPIECE3 (4)
	};

	remove = true;
	checkAssembled = true;
	gaveWeapon = false;
	if(player->pclass != matchClass)
	{
		// Wrong class, but try to pick up for mana
		if(netgame && !deathmatch)
		{
			// Can't pick up wrong-class weapons in coop netplay
			return;
		}
		checkAssembled = false;
		gaveMana = P_GiveMana(player, AmmoType::Mana1, 20) +
			P_GiveMana(player, AmmoType::Mana2, 20);
		if(!gaveMana)
		{
			// Didn't need the mana, so don't pick it up
			return;
		}
	}
	else if(netgame && !deathmatch)
	{
		// Cooperative net-game
		if(player->pieces & pieceValue)
		{
			// Already has the piece
			return;
		}
		pieceValue = pieceValueTrans[pieceValue];
		P_GiveMana(player, AmmoType::Mana1, 20);
		P_GiveMana(player, AmmoType::Mana2, 20);
		remove = false;
	}
	else
	{
		// Deathmatch or single player game
		gaveMana = P_GiveMana(player, AmmoType::Mana1, 20) +
			P_GiveMana(player, AmmoType::Mana2, 20);
		if(player->pieces & pieceValue)
		{
			// Already has the piece, check if mana needed
			if(!gaveMana)
			{
				// Didn't need the mana, so don't pick it up
				return;
			}
			checkAssembled = false;
		}
	}

	// Pick up the weapon piece
	if(pieceMobj->special)
	{
		map_format.execute_line_special(pieceMobj->special, pieceMobj->special_args, nullptr, 0, player->mo);
		pieceMobj->special = 0;
	}
	if(remove)
	{
		if(deathmatch && !(pieceMobj->flags & MF_DROPPED))
		{
			P_HideSpecialThing(pieceMobj);
		}
		else
		{
			P_RemoveMobj(pieceMobj);
		}
	}
	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		SB_PaletteFlash(false);
	}

	// Check if fourth weapon assembled
	if(checkAssembled)
	{
		player->pieces |= pieceValue;
		if(player->pieces == (WPIECE1 | WPIECE2 | WPIECE3))
		{
			gaveWeapon = true;
			player->weaponowned[std::to_underlying(WeaponType::Fourth)] = true;
			P_AutoSwitchWeapon(player, static_cast<WeaponType>(WeaponType::Fourth));
		}
	}

	if(gaveWeapon)
	{
		P_SetMessage(player, fourthWeaponText[std::to_underlying(matchClass)], false);
		// Play the build-sound full volume for all players
		S_StartVoidSound(SfxId::HexenWeaponBuild);
	}
	else
	{
		P_SetMessage(player, weaponPieceText[std::to_underlying(matchClass)], false);
		if(player == &players[consoleplayer])
		{
			S_StartVoidSound(SfxId::HexenPickupWeapon);
		}
	}
}

int P_GiveKey(player_t* player, Card key)
{
	if(player->cards[std::to_underlying(key)])
	{
		return false;
	}
	player->bonuscount += BONUSADD;
	player->cards[std::to_underlying(key)] = true;

	if(player == &players[consoleplayer])
		player->ravenkeys |= 1 << std::to_underlying(key);

	return true;
}

static void SetDormantArtifact(mobj_t* arti)
{
	arti->flags &= ~MF_SPECIAL;
	if(deathmatch && !(arti->flags & MF_DROPPED))
	{
		if(arti->type == MobjType::HexenArtiinvulnerability)
		{
			P_SetMobjState(arti, StateId::HexenDormantarti31);
		}
		else if(arti->type == MobjType::HexenSummonmaulator || arti->type == MobjType::HexenArtifly)
		{
			P_SetMobjState(arti, StateId::HexenDormantarti21);
		}
		else
		{
			P_SetMobjState(arti, StateId::HexenDormantarti11);
		}
	}
	else
	{
		// Don't respawn
		P_SetMobjState(arti, StateId::HexenDeadarti1);
	}
}

static void TryPickupArtifact(player_t* player, ArtiType artifactType, mobj_t* artifact)
{
	static const char* artifactMessages[std::to_underlying(ArtiType::HexenCount)] = {
		nullptr,
		TXT_ARTIINVULNERABILITY,
		TXT_ARTIHEALTH,
		TXT_ARTISUPERHEALTH,
		TXT_ARTIHEALINGRADIUS,
		TXT_ARTISUMMON,
		TXT_ARTITORCH,
		TXT_ARTIEGG,
		TXT_ARTIFLY,
		TXT_ARTIBLASTRADIUS,
		TXT_ARTIPOISONBAG,
		TXT_ARTITELEPORTOTHER,
		TXT_ARTISPEED,
		TXT_ARTIBOOSTMANA,
		TXT_ARTIBOOSTARMOR,
		TXT_ARTITELEPORT,
		TXT_ARTIPUZZSKULL,
		TXT_ARTIPUZZGEMBIG,
		TXT_ARTIPUZZGEMRED,
		TXT_ARTIPUZZGEMGREEN1,
		TXT_ARTIPUZZGEMGREEN2,
		TXT_ARTIPUZZGEMBLUE1,
		TXT_ARTIPUZZGEMBLUE2,
		TXT_ARTIPUZZBOOK1,
		TXT_ARTIPUZZBOOK2,
		TXT_ARTIPUZZSKULL2,
		TXT_ARTIPUZZFWEAPON,
		TXT_ARTIPUZZCWEAPON,
		TXT_ARTIPUZZMWEAPON,
		TXT_ARTIPUZZGEAR, // All gear pickups use the same text
		TXT_ARTIPUZZGEAR,
		TXT_ARTIPUZZGEAR,
		TXT_ARTIPUZZGEAR
	};

	if(gamemode == GameMode::Shareware)
	{
		artifactMessages[std::to_underlying(ArtiType::HexenBlastradius)] = TXT_ARTITELEPORT;
		artifactMessages[std::to_underlying(ArtiType::HexenTeleport)] = TXT_ARTIBLASTRADIUS;
	}

	if(P_GiveArtifact(player, static_cast<ArtiType>(artifactType), artifact))
	{
		if(artifact->special)
		{
			map_format.execute_line_special(artifact->special, artifact->special_args, nullptr, 0, nullptr);
			artifact->special = 0;
		}
		player->bonuscount += BONUSADD;
		if(artifactType < ArtiType::HexenFirstpuzzitem)
		{
			SetDormantArtifact(artifact);
			S_StartMobjSound(artifact, SfxId::HexenPickupArtifact);
			P_SetMessage(player, artifactMessages[std::to_underlying(artifactType)], false);
		}
		else
		{
			// Puzzle item
			S_StartVoidSound(SfxId::HexenPickupItem);
			P_SetMessage(player, artifactMessages[std::to_underlying(artifactType)], true);
			if(!netgame || deathmatch)
			{
				// Remove puzzle items if not cooperative netplay
				P_RemoveMobj(artifact);
			}
		}
	}
}

static void Hexen_P_TouchSpecialThing(mobj_t* special, mobj_t* toucher)
{
	player_t* player;
	fixed_t delta;
	SfxId sound;
	dboolean respawn;

	delta = special->z - toucher->z;
	if(delta > toucher->height || delta < -32 * FRACUNIT)
	{
		// Out of reach
		return;
	}
	if(toucher->health <= 0)
	{
		// Toucher is dead
		return;
	}
	sound = SfxId::HexenPickupItem;
	player = toucher->player;
	respawn = true;
	switch(special->sprite)
	{
		// Items
		case SpriteId::HexenPtn1: // Item_HealingPotion
			if(!P_GiveBody(player, 10))
			{
				return;
			}
			P_SetMessage(player, TXT_ITEMHEALTH, false);
			break;
		case SpriteId::HexenArm1:
			if(!Hexen_P_GiveArmor(player, ArmorType::Armor, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR1, false);
			break;
		case SpriteId::HexenArm2:
			if(!Hexen_P_GiveArmor(player, ArmorType::Shield, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR2, false);
			break;
		case SpriteId::HexenArm3:
			if(!Hexen_P_GiveArmor(player, ArmorType::Helmet, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR3, false);
			break;
		case SpriteId::HexenArm4:
			if(!Hexen_P_GiveArmor(player, ArmorType::Amulet, -1))
			{
				return;
			}
			P_SetMessage(player, TXT_ARMOR4, false);
			break;

		// Keys
		case SpriteId::HexenKey1:
		case SpriteId::HexenKey2:
		case SpriteId::HexenKey3:
		case SpriteId::HexenKey4:
		case SpriteId::HexenKey5:
		case SpriteId::HexenKey6:
		case SpriteId::HexenKey7:
		case SpriteId::HexenKey8:
		case SpriteId::HexenKey9:
		case SpriteId::HexenKeya:
		case SpriteId::HexenKeyb:
			if(!P_GiveKey(player, static_cast<Card>(std::to_underlying(special->sprite) - std::to_underlying(SpriteId::HexenKey1))))
			{
				return;
			}
			P_SetMessage(player, TextKeyMessages[std::to_underlying(special->sprite) - std::to_underlying(SpriteId::HexenKey1)],
				true);
			sound = SfxId::HexenPickupKey;

			// Check and process the special now in case the key doesn't
			// get removed for coop netplay
			if(special->special)
			{
				map_format.execute_line_special(special->special, special->special_args, nullptr, 0, toucher);
				special->special = 0;
			}

			if(!netgame)
			{
				// Only remove keys in single player game
				break;
			}
			player->bonuscount += BONUSADD;
			if(player == &players[consoleplayer])
			{
				S_StartVoidSound(sound);
				SB_PaletteFlash(false);
			}
			return;

		// Artifacts
		case SpriteId::HexenPtn2:
			TryPickupArtifact(player, ArtiType::HexenHealth, special);
			return;
		case SpriteId::HexenSoar:
			TryPickupArtifact(player, ArtiType::HexenFly, special);
			return;
		case SpriteId::HexenInvu:
			TryPickupArtifact(player, ArtiType::HexenInvulnerability, special);
			return;
		case SpriteId::HexenSumn:
			TryPickupArtifact(player, ArtiType::HexenSummon, special);
			return;
		case SpriteId::HexenPork:
			TryPickupArtifact(player, ArtiType::HexenEgg, special);
			return;
		case SpriteId::HexenSphl:
			TryPickupArtifact(player, ArtiType::HexenSuperhealth, special);
			return;
		case SpriteId::HexenHrad:
			TryPickupArtifact(player, ArtiType::HexenHealingradius, special);
			return;
		case SpriteId::HexenTrch:
			TryPickupArtifact(player, ArtiType::HexenTorch, special);
			return;
		case SpriteId::HexenAtlp:
			TryPickupArtifact(player, ArtiType::HexenTeleport, special);
			return;
		case SpriteId::HexenTelo:
			TryPickupArtifact(player, ArtiType::HexenTeleportother, special);
			return;
		case SpriteId::HexenPsbg:
			TryPickupArtifact(player, ArtiType::HexenPoisonbag, special);
			return;
		case SpriteId::HexenSped:
			TryPickupArtifact(player, ArtiType::HexenSpeed, special);
			return;
		case SpriteId::HexenBman:
			TryPickupArtifact(player, ArtiType::HexenBoostmana, special);
			return;
		case SpriteId::HexenBrac:
			TryPickupArtifact(player, ArtiType::HexenBoostarmor, special);
			return;
		case SpriteId::HexenBlst:
			TryPickupArtifact(player, ArtiType::HexenBlastradius, special);
			return;

		// Puzzle artifacts
		case SpriteId::HexenAsku:
			TryPickupArtifact(player, ArtiType::HexenPuzzskull, special);
			return;
		case SpriteId::HexenAbgm:
			TryPickupArtifact(player, ArtiType::HexenPuzzgembig, special);
			return;
		case SpriteId::HexenAgmr:
			TryPickupArtifact(player, ArtiType::HexenPuzzgemred, special);
			return;
		case SpriteId::HexenAgmg:
			TryPickupArtifact(player, ArtiType::HexenPuzzgemgreen1, special);
			return;
		case SpriteId::HexenAgg2:
			TryPickupArtifact(player, ArtiType::HexenPuzzgemgreen2, special);
			return;
		case SpriteId::HexenAgmb:
			TryPickupArtifact(player, ArtiType::HexenPuzzgemblue1, special);
			return;
		case SpriteId::HexenAgb2:
			TryPickupArtifact(player, ArtiType::HexenPuzzgemblue2, special);
			return;
		case SpriteId::HexenAbk1:
			TryPickupArtifact(player, ArtiType::HexenPuzzbook1, special);
			return;
		case SpriteId::HexenAbk2:
			TryPickupArtifact(player, ArtiType::HexenPuzzbook2, special);
			return;
		case SpriteId::HexenAsk2:
			TryPickupArtifact(player, ArtiType::HexenPuzzskull2, special);
			return;
		case SpriteId::HexenAfwp:
			TryPickupArtifact(player, ArtiType::HexenPuzzfweapon, special);
			return;
		case SpriteId::HexenAcwp:
			TryPickupArtifact(player, ArtiType::HexenPuzzcweapon, special);
			return;
		case SpriteId::HexenAmwp:
			TryPickupArtifact(player, ArtiType::HexenPuzzmweapon, special);
			return;
		case SpriteId::HexenAger:
			TryPickupArtifact(player, ArtiType::HexenPuzzgear1, special);
			return;
		case SpriteId::HexenAgr2:
			TryPickupArtifact(player, ArtiType::HexenPuzzgear2, special);
			return;
		case SpriteId::HexenAgr3:
			TryPickupArtifact(player, ArtiType::HexenPuzzgear3, special);
			return;
		case SpriteId::HexenAgr4:
			TryPickupArtifact(player, ArtiType::HexenPuzzgear4, special);
			return;

		// Mana
		case SpriteId::HexenMan1:
			if(!P_GiveMana(player, AmmoType::Mana1, 15))
			{
				return;
			}
			P_SetMessage(player, TXT_MANA_1, false);
			break;
		case SpriteId::HexenMan2:
			if(!P_GiveMana(player, AmmoType::Mana2, 15))
			{
				return;
			}
			P_SetMessage(player, TXT_MANA_2, false);
			break;
		case SpriteId::HexenMan3: // Double Mana Dodecahedron
			if(!P_GiveMana(player, AmmoType::Mana1, 20))
			{
				if(!P_GiveMana(player, AmmoType::Mana2, 20))
				{
					return;
				}
			}
			else
			{
				P_GiveMana(player, AmmoType::Mana2, 20);
			}
			P_SetMessage(player, TXT_MANA_BOTH, false);
			break;

		// 2nd and 3rd Mage Weapons
		case SpriteId::HexenWmcs: // Frost Shards
			TryPickupWeapon(player, PClass::Mage, WeaponType::Second,
				special, TXT_WEAPON_M2);
			return;
		case SpriteId::HexenWmlg: // Arc of Death
			TryPickupWeapon(player, PClass::Mage, WeaponType::Third,
				special, TXT_WEAPON_M3);
			return;

		// 2nd and 3rd Fighter Weapons
		case SpriteId::HexenWfax: // Timon's Axe
			TryPickupWeapon(player, PClass::Fighter, WeaponType::Second,
				special, TXT_WEAPON_F2);
			return;
		case SpriteId::HexenWfhm: // Hammer of Retribution
			TryPickupWeapon(player, PClass::Fighter, WeaponType::Third,
				special, TXT_WEAPON_F3);
			return;

		// 2nd and 3rd Cleric Weapons
		case SpriteId::HexenWcss: // Serpent Staff
			TryPickupWeapon(player, PClass::Cleric, WeaponType::Second,
				special, TXT_WEAPON_C2);
			return;
		case SpriteId::HexenWcfm: // Firestorm
			TryPickupWeapon(player, PClass::Cleric, WeaponType::Third,
				special, TXT_WEAPON_C3);
			return;

		// Fourth Weapon Pieces
		case SpriteId::HexenWfr1:
			TryPickupWeaponPiece(player, PClass::Fighter, WPIECE1, special);
			return;
		case SpriteId::HexenWfr2:
			TryPickupWeaponPiece(player, PClass::Fighter, WPIECE2, special);
			return;
		case SpriteId::HexenWfr3:
			TryPickupWeaponPiece(player, PClass::Fighter, WPIECE3, special);
			return;
		case SpriteId::HexenWch1:
			TryPickupWeaponPiece(player, PClass::Cleric, WPIECE1, special);
			return;
		case SpriteId::HexenWch2:
			TryPickupWeaponPiece(player, PClass::Cleric, WPIECE2, special);
			return;
		case SpriteId::HexenWch3:
			TryPickupWeaponPiece(player, PClass::Cleric, WPIECE3, special);
			return;
		case SpriteId::HexenWms1:
			TryPickupWeaponPiece(player, PClass::Mage, WPIECE1, special);
			return;
		case SpriteId::HexenWms2:
			TryPickupWeaponPiece(player, PClass::Mage, WPIECE2, special);
			return;
		case SpriteId::HexenWms3:
			TryPickupWeaponPiece(player, PClass::Mage, WPIECE3, special);
			return;

		default:
			I_Error("P_SpecialThing: Unknown gettable thing");
	}
	if(special->special)
	{
		map_format.execute_line_special(special->special, special->special_args, nullptr, 0, toucher);
		special->special = 0;
	}
	if(deathmatch && respawn && !(special->flags & MF_DROPPED))
	{
		P_HideSpecialThing(special);
	}
	else
	{
		P_RemoveMobj(special);
	}
	player->bonuscount += BONUSADD;
	if(player == &players[consoleplayer])
	{
		S_StartVoidSound(sound);
		SB_PaletteFlash(false);
	}
}

// Search thinker list for minotaur
static mobj_t* ActiveMinotaur(player_t* master)
{
	byte args[5];
	mobj_t* mo;
	player_t* plr;
	thinker_t* think;
	unsigned int starttime;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != reinterpret_cast<think_t>(P_MobjThinker))
			continue;
		mo = (mobj_t*)think;
		if(mo->type != MobjType::HexenMinotaur)
			continue;
		if(mo->health <= 0)
			continue;
		if(!(mo->flags & MF_COUNTKILL))
			continue; // for morphed minotaurs
		if(mo->flags & MF_CORPSE)
			continue;

		COLLAPSE_SPECIAL_ARGS(args, mo->special_args);
		memcpy(&starttime, args, sizeof(unsigned int));
		if(leveltime - LittleLong(starttime) >= std::to_underlying(PowerDuration::Maulatortics))
			continue;

		plr = mo->special1.m->player;
		if(plr == master)
			return (mo);
	}
	return (nullptr);
}

dboolean P_MorphPlayer(player_t* player)
{
	mobj_t* pmo;
	mobj_t* fog;
	mobj_t* beastMo;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	angle_t angle;
	int oldFlags2;

	if(player->powers[std::to_underlying(PowerType::Invulnerability)])
	{
		// Immune when invulnerable
		return (false);
	}
	if(player->morphTics)
	{
		// Player is already a beast
		return false;
	}
	pmo = player->mo;
	x = pmo->x;
	y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	oldFlags2 = pmo->flags2;
	P_SetMobjState(pmo, StateId::HexenFreetargmobj);
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MobjType::HexenTfog);
	S_StartMobjSound(fog, SfxId::HexenTeleport);
	beastMo = P_SpawnMobj(x, y, z, MobjType::HexenPigplayer);
	beastMo->special1.i = std::to_underlying(player->readyweapon);
	beastMo->angle = angle;
	beastMo->player = player;
	player->health = beastMo->health = MAXMORPHHEALTH;
	player->mo = beastMo;
	memset(&player->armorpoints[0], 0, std::to_underlying(ArmorType::Count) * sizeof(int));
	player->pclass = PClass::Pig;
	if(oldFlags2 & MF2_FLY)
	{
		beastMo->flags2 |= MF2_FLY;
	}
	player->morphTics = std::to_underlying(PowerDuration::Morphtics);
	P_ActivateMorphWeapon(player);
	return (true);
}

static dboolean P_MorphMonster(mobj_t* actor)
{
	mobj_t *master, *monster, *fog;
	MobjType moType;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	mobj_t oldMonster;

	if(actor->player)
		return (false);
	if(!(actor->flags & MF_COUNTKILL))
		return false;
	if(actor->flags2 & MF2_BOSS)
		return false;
	moType = actor->type;
	switch(moType)
	{
		case MobjType::HexenPig:
			return (false);
		case MobjType::HexenFighterBoss:
		case MobjType::HexenClericBoss:
		case MobjType::HexenMageBoss:
			return (false);
		default:
			break;
	}

	oldMonster = *actor;
	x = oldMonster.x;
	y = oldMonster.y;
	z = oldMonster.z;
	map_format.remove_mobj_thing_id(actor);
	P_SetMobjState(actor, StateId::HexenFreetargmobj);
	fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MobjType::HexenTfog);
	S_StartMobjSound(fog, SfxId::HexenTeleport);
	monster = P_SpawnMobj(x, y, z, MobjType::HexenPig);
	monster->special2.i = std::to_underlying(moType);
	monster->special1.i = std::to_underlying(PowerDuration::Morphtics) + P_Random(RandomClass::Hexen);
	monster->flags |= (oldMonster.flags & MF_SHADOW);
	P_SetTarget(&monster->target, oldMonster.target);
	monster->angle = oldMonster.angle;
	monster->tid = oldMonster.tid;
	monster->special = oldMonster.special;
	map_format.add_mobj_thing_id(monster, oldMonster.tid);
	memcpy(monster->special_args, oldMonster.special_args, SPECIAL_ARGS_SIZE);
	dsda_WatchMorph(monster);

	// check for turning off minotaur power for active icon
	if(moType == MobjType::HexenMinotaur)
	{
		master = oldMonster.special1.m;
		if(master->health > 0)
		{
			if(!ActiveMinotaur(master->player))
			{
				master->player->powers[std::to_underlying(PowerType::Minotaur)] = 0;
			}
		}
	}
	return (true);
}

void P_PoisonPlayer(player_t* player, mobj_t* poisoner, int poison)
{
	if((player->cheats & CF_GODMODE) || player->powers[std::to_underlying(PowerType::Invulnerability)])
	{
		return;
	}
	player->poisoncount += poison;
	player->poisoner = poisoner;
	if(player->poisoncount > 100)
	{
		player->poisoncount = 100;
	}
}
