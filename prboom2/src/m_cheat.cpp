// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Cheat sequence checking.
 */

#include <utility>

#include "doomstat.hpp"
#include "am_map.hpp"
#include "g_game.hpp"
#include "r_data.hpp"
#include "p_inter.hpp"
#include "p_tick.hpp"
#include "m_cheat.hpp"
#include "s_sound.hpp"
#include "s_advsound.hpp"
#include "sounds.hpp"
#include "dstrings.hpp"
#include "r_main.hpp"
#include "p_map.hpp"
#include "d_deh.hpp"  // Ty 03/27/98 - externalized strings
/* cph 2006/07/23 - needs direct access to thinkercap */
#include "p_tick.hpp"
#include "w_wad.hpp"
#include "p_setup.hpp"
#include "p_user.hpp"
#include "lprintf.hpp"

#include "heretic/def.hpp"
#include "heretic/sb_bar.hpp"

#include "dsda.hpp"
#include "dsda/configuration.hpp"
#include "dsda/excmd.hpp"
#include "dsda/exhud.hpp"
#include "dsda/features.hpp"
#include "dsda/input.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/messenger.hpp"
#include "dsda/settings.hpp"
#include "dsda/skill_info.hpp"

#define plyr (players+consoleplayer)     /* the console player */

//-----------------------------------------------------------------------------
//
// CHEAT SEQUENCE PACKAGE
//
//-----------------------------------------------------------------------------

static void cheat_mus(char buf[3]);
static void cheat_choppers();
static void cheat_god();
static void cheat_fa();
static void cheat_k();
static void cheat_kfa();
static void cheat_noclip();
static void cheat_pw(int pw);
static void cheat_behold();
static void cheat_clev0();
static void cheat_clev(char buf[3]);
static void cheat_mypos();
static void cheat_rate();
static void cheat_comp0();
static void cheat_comp(char buf[3]);
static void cheat_skill0();
static void cheat_skill(char buf[1]);
static void cheat_friction();
static void cheat_pushers();
static void cheat_massacre();
static void cheat_ddt();
static void cheat_reveal_secret();
static void cheat_reveal_kill();
static void cheat_reveal_item();
static void cheat_hom();
static void cheat_fast();
static void cheat_tntkey();
static void cheat_tntkeyx();
static void cheat_tntkeyxx(int key);
static void cheat_tntweap();
static void cheat_tntweapx(char buf[3]);
static void cheat_tntammo();
static void cheat_tntammox(char buf[1]);
static void cheat_smart();
static void cheat_pitch();
static void cheat_megaarmour();
static void cheat_health();
static void cheat_notarget();
static void cheat_freeze();
static void cheat_fly();

// heretic
static void cheat_reset_health();
static void cheat_tome();
static void cheat_chicken();
static void cheat_artifact(char buf[3]);

// hexen
static void cheat_inventory();
static void cheat_puzzle();
static void cheat_class(char buf[2]);
static void cheat_init();
static void cheat_script(char buf[3]);

//-----------------------------------------------------------------------------
//
// List of cheat codes, functions, and special argument indicators.
//
// The first argument is the cheat code.
//
// The second argument is its DEH name, or NULL if it's not supported by -deh.
//
// The third argument is a combination of the bitmasks,
// which excludes the cheat during certain modes of play.
//
// The fourth argument is the handler function.
//
// The fifth argument is passed to the handler function if it's non-negative;
// if negative, then its negative indicates the number of extra characters
// expected after the cheat code, which are passed to the handler function
// via a pointer to a buffer (after folding any letters to lowercase).
//
//-----------------------------------------------------------------------------

cheatseq_t cheat[] = {
	CHEAT("idmus", "Change music", CheatWhen::Always, cheat_mus, -2, false),
	CHEAT("idchoppers", "Chainsaw", CheatWhen::NotDemo, cheat_choppers, 0, false),
	CHEAT("iddqd", "God mode", CheatWhen::NotClassicDemo, cheat_god, 0, false),
	CHEAT("idkfa", "Ammo & Keys", CheatWhen::NotDemo, cheat_kfa, 0, false),
	CHEAT("idfa", "Ammo", CheatWhen::NotDemo, cheat_fa, 0, false),
	CHEAT("idspispopd", "No Clipping 1", CheatWhen::NotClassicDemo, cheat_noclip, 0, false),
	CHEAT("idclip", "No Clipping 2", CheatWhen::NotClassicDemo, cheat_noclip, 0, false),
	CHEAT("idbeholdh", "Invincibility", CheatWhen::NotDemo, cheat_health, 0, false),
	CHEAT("idbeholdm", "Invincibility", CheatWhen::NotDemo, cheat_megaarmour, 0, false),
	CHEAT("idbeholdv", "Invincibility", CheatWhen::NotDemo, cheat_pw, std::to_underlying(PowerType::Invulnerability), false),
	CHEAT("idbeholds", "Berserk", CheatWhen::NotDemo, cheat_pw, std::to_underlying(PowerType::Strength), false),
	CHEAT("idbeholdi", "Invisibility", CheatWhen::NotDemo, cheat_pw, std::to_underlying(PowerType::Invisibility), false),
	CHEAT("idbeholdr", "Radiation Suit", CheatWhen::NotDemo, cheat_pw, std::to_underlying(PowerType::IronFeet), false),
	CHEAT("idbeholda", "Auto-map", CheatWhen::Always, cheat_pw, std::to_underlying(PowerType::AllMap), false),
	CHEAT("idbeholdl", "Lite-Amp Goggles", CheatWhen::Always, cheat_pw, std::to_underlying(PowerType::Infrared), false),
	CHEAT("idbehold", "BEHOLD menu", CheatWhen::Always, cheat_behold, 0, false),
	CHEAT("idclev", "Level Warp", CheatWhen::NotDemo | CheatWhen::NotMenu, cheat_clev, -2, false),
	CHEAT("idclev", "Level Warp", CheatWhen::NotDemo | CheatWhen::NotMenu, cheat_clev0, 0, false),
	CHEAT("idmypos", nullptr, CheatWhen::Always, cheat_mypos, 0, false),
	CHEAT("idrate", "Frame rate", CheatWhen::Always, cheat_rate, 0, false),
	// phares
	CHEAT("tntcomp", nullptr, CheatWhen::NotDemo, cheat_comp, -2, false),
	CHEAT("tntcomp", nullptr, CheatWhen::NotDemo, cheat_comp0, 0, false),
	CHEAT("skill", nullptr, CheatWhen::NotDemo, cheat_skill, -1, false),
	CHEAT("skill", nullptr, CheatWhen::NotDemo, cheat_skill0, 0, false),
	// jff 2/01/98 kill all monsters
	CHEAT("tntem", nullptr, CheatWhen::NotDemo, cheat_massacre, 0, false),
	// killough 2/07/98: moved from am_map.c
	CHEAT("iddt", "Map cheat", CheatWhen::Always, cheat_ddt, 0, true),
	CHEAT("iddst", nullptr, CheatWhen::Always, cheat_reveal_secret, 0, true),
	CHEAT("iddkt", nullptr, CheatWhen::Always, cheat_reveal_kill, 0, true),
	CHEAT("iddit", nullptr, CheatWhen::Always, cheat_reveal_item, 0, true),
	// killough 2/07/98: HOM autodetector
	CHEAT("tnthom", nullptr, CheatWhen::Always, cheat_hom, 0, false),
	// killough 2/16/98: generalized key cheats
	CHEAT("tntkey", nullptr, CheatWhen::NotDemo, cheat_tntkey, 0, false),
	CHEAT("tntkeyr", nullptr, CheatWhen::NotDemo, cheat_tntkeyx, 0, false),
	CHEAT("tntkeyy", nullptr, CheatWhen::NotDemo, cheat_tntkeyx, 0, false),
	CHEAT("tntkeyb", nullptr, CheatWhen::NotDemo, cheat_tntkeyx, 0, false),
	CHEAT("tntkeyrc", nullptr, CheatWhen::NotDemo, cheat_tntkeyxx, std::to_underlying(Card::RedCard), false),
	CHEAT("tntkeyyc", nullptr, CheatWhen::NotDemo, cheat_tntkeyxx, std::to_underlying(Card::YellowCard), false),
	CHEAT("tntkeybc", nullptr, CheatWhen::NotDemo, cheat_tntkeyxx, std::to_underlying(Card::BlueCard), false),
	CHEAT("tntkeyrs", nullptr, CheatWhen::NotDemo, cheat_tntkeyxx, std::to_underlying(Card::RedSkull), false),
	CHEAT("tntkeyys", nullptr, CheatWhen::NotDemo, cheat_tntkeyxx, std::to_underlying(Card::YellowSkull), false),
	// killough 2/16/98: end generalized keys
	CHEAT("tntkeybs", nullptr, CheatWhen::NotDemo, cheat_tntkeyxx, std::to_underlying(Card::BlueSkull), false),
	// Ty 04/11/98 - Added TNTKA
	CHEAT("tntka", nullptr, CheatWhen::NotDemo, cheat_k, 0, false),
	// killough 2/16/98: generalized weapon cheats
	CHEAT("tntweap", nullptr, CheatWhen::NotDemo, cheat_tntweap, 0, false),
	CHEAT("tntweap", nullptr, CheatWhen::NotDemo, cheat_tntweapx, -1, false),
	CHEAT("tntammo", nullptr, CheatWhen::NotDemo, cheat_tntammo, 0, false),
	// killough 2/16/98: end generalized weapons
	CHEAT("tntammo", nullptr, CheatWhen::NotDemo, cheat_tntammox, -1, false),
	// killough 2/21/98: smart monster toggle
	CHEAT("tntsmart", nullptr, CheatWhen::NotDemo, cheat_smart, 0, false),
	// killough 2/21/98: pitched sound toggle
	CHEAT("tntpitch", nullptr, CheatWhen::Always, cheat_pitch, 0, false),
	// killough 2/21/98: reduce RSI injury by adding simpler alias sequences:
	// killough 2/21/98: same as tntammo
	CHEAT("tntamo", nullptr, CheatWhen::NotDemo, cheat_tntammo, 0, false),
	// killough 2/21/98: same as tntammo
	CHEAT("tntamo", nullptr, CheatWhen::NotDemo, cheat_tntammox, -1, false),
	// killough 3/6/98: -fast toggle
	CHEAT("tntfast", nullptr, CheatWhen::NotDemo, cheat_fast, 0, false),
	// phares 3/10/98: toggle variable friction effects
	CHEAT("tntice", nullptr, CheatWhen::NotDemo, cheat_friction, 0, false),
	// phares 3/10/98: toggle pushers
	CHEAT("tntpush", nullptr, CheatWhen::NotDemo, cheat_pushers, 0, false),

	// [RH] Monsters don't target
	CHEAT("notarget", nullptr, CheatWhen::NotDemo, cheat_notarget, 0, false),
	// fly mode is active
	CHEAT("fly", nullptr, CheatWhen::NotDemo, cheat_fly, 0, false),

	// heretic
	CHEAT("quicken", nullptr, CheatWhen::NotClassicDemo, cheat_god, 0, false),
	CHEAT("ponce", nullptr, CheatWhen::NotDemo, cheat_reset_health, 0, false),
	CHEAT("kitty", nullptr, CheatWhen::NotClassicDemo, cheat_noclip, 0, false),
	CHEAT("massacre", nullptr, CheatWhen::NotDemo, cheat_massacre, 0, false),
	CHEAT("rambo", nullptr, CheatWhen::NotDemo, cheat_fa, 0, false),
	CHEAT("skel", nullptr, CheatWhen::NotDemo, cheat_k, 0, false),
	CHEAT("gimme", nullptr, CheatWhen::NotDemo, cheat_artifact, -2, false),
	CHEAT("shazam", nullptr, CheatWhen::NotDemo, cheat_tome, 0, false),
	CHEAT("engage", nullptr, CheatWhen::NotDemo | CheatWhen::NotMenu, cheat_clev, -2, false),
	CHEAT("ravmap", nullptr, CheatWhen::Always, cheat_ddt, 0, true),
	CHEAT("cockadoodledoo", nullptr, CheatWhen::NotDemo, cheat_chicken, 0, false),

	// hexen
	CHEAT("satan", nullptr, CheatWhen::NotClassicDemo, cheat_god, 0, false),
	CHEAT("clubmed", nullptr, CheatWhen::NotDemo, cheat_reset_health, 0, false),
	CHEAT("butcher", nullptr, CheatWhen::NotDemo, cheat_massacre, 0, false),
	CHEAT("nra", nullptr, CheatWhen::NotDemo, cheat_fa, 0, false),
	CHEAT("indiana", nullptr, CheatWhen::NotDemo, cheat_inventory, 0, false),
	CHEAT("locksmith", nullptr, CheatWhen::NotDemo, cheat_k, 0, false),
	CHEAT("sherlock", nullptr, CheatWhen::NotDemo, cheat_puzzle, 0, false),
	CHEAT("casper", nullptr, CheatWhen::NotClassicDemo, cheat_noclip, 0, false),
	CHEAT("shadowcaster", nullptr, CheatWhen::NotDemo, cheat_class, -1, false),
	CHEAT("visit", nullptr, CheatWhen::NotDemo | CheatWhen::NotMenu, cheat_clev, -2, false),
	CHEAT("init", nullptr, CheatWhen::NotDemo, cheat_init, 0, false),
	CHEAT("puke", nullptr, CheatWhen::NotDemo, cheat_script, -2, false),
	CHEAT("mapsco", nullptr, CheatWhen::Always, cheat_ddt, 0, true),
	CHEAT("deliverance", nullptr, CheatWhen::NotDemo, cheat_chicken, 0, false),

	// end-of-list marker
	{nullptr}
};

//-----------------------------------------------------------------------------

static void cheat_mus(char buf[3])
{
	int musnum, muslump;
	int epsd, map;
	char* mapname;

	//jff 3/20/98 note: this cheat allowed in netgame/demorecord

	//jff 3/17/98 avoid musnum being negative and crashing
	if(!isdigit(buf[0]) || !isdigit(buf[1]))
		return;

	if(gamemode == GameMode::Commercial)
	{
		epsd = 1; //jff was 0, but espd is 1-based
		map = (buf[0] - '0') * 10 + buf[1] - '0';
	}
	else
	{
		epsd = buf[0] - '0';
		map = buf[1] - '0';
	}

	idmusnum = -1;
	dsda_MapMusic(&musnum, &muslump, epsd, map);
	idmusnum = musnum; //jff 3/17/98 remember idmus number for restore

	mapname = VANILLA_MAP_LUMP_NAME(epsd, map);

	if(W_LumpNameExists(mapname))
	{
		doom_printf("%s: %s", s_STSTR_MUS, mapname);

		if(muslump != -1)
		{
			S_ChangeMusInfoMusic(muslump, true);
		}
		else if(musnum != -1)
		{
			S_ChangeMusic(static_cast<MusicId>(musnum), 1);
		}
	}
	else
	{
		dsda_AddMessage(s_STSTR_NOMUS);
	}
}

// 'choppers' invulnerability & chainsaw
static void cheat_choppers()
{
	plyr->weaponowned[std::to_underlying(WeaponType::Chainsaw)] = true;
	plyr->powers[std::to_underlying(PowerType::Invulnerability)] = true;
	dsda_AddMessage(s_STSTR_CHOPPERS);
}

void M_CheatGod()
{
	// dead players are first respawned at the current position
	if(plyr->playerstate == PlayerState::Dead)
	{
		signed int an;
		mapthing_t mt = {0};

		P_MapStart();
		mt.x = plyr->mo->x;
		mt.y = plyr->mo->y;
		mt.angle = (plyr->mo->angle + ANG45 / 2) * (uint64_t)45 / ANG45;
		mt.type = consoleplayer + 1;
		mt.options = 1; // arbitrary non-zero value
		P_SpawnPlayer(consoleplayer, &mt);

		// reset view to center (heretic / hexen)
		plyr->lookdir = 0;

		// spawn a teleport fog
		an = plyr->mo->angle >> ANGLETOFINESHIFT;
		P_SpawnMobj(plyr->mo->x + 20 * finecosine[an],
			plyr->mo->y + 20 * finesine[an],
			plyr->mo->z + g_telefog_height,
			static_cast<MobjType>(g_mt_tfog));
		S_StartMobjSound(plyr->mo, g_sfx_revive);
		P_MapEnd();
	}

	plyr->cheats ^= CF_GODMODE;
	if(plyr->cheats & CF_GODMODE)
	{
		if(plyr->mo)
			plyr->mo->health = god_health; // Ty 03/09/98 - deh

		plyr->health = god_health;
		dsda_AddMessage(s_STSTR_DQDON);
	}
	else
		dsda_AddMessage(s_STSTR_DQDOFF);

	if(raven) SB_Start();
}

static void cheat_god()
{
	// 'dqd' cheat for toggleable god mode
	if(demorecording)
	{
		dsda_QueueExCmdGod();
		return;
	}

	M_CheatGod();
}

// CPhipps - new health and armour cheat codes
static void cheat_health()
{
	if(!(plyr->cheats & CF_GODMODE))
	{
		if(plyr->mo)
			plyr->mo->health = mega_health;
		plyr->health = mega_health;
		dsda_AddMessage(s_STSTR_BEHOLDX);
	}
}

static void cheat_megaarmour()
{
	plyr->armorpoints[std::to_underlying(ArmorType::Armor)] = idfa_armor; // Ty 03/09/98 - deh
	plyr->armortype = idfa_armor_class;          // Ty 03/09/98 - deh
	dsda_AddMessage(s_STSTR_BEHOLDX);
}

static void cheat_fa()
{
	int i;

	if(hexen)
	{
		for(i = 0; i < std::to_underlying(ArmorType::Count); i++)
		{
			plyr->armorpoints[i] = pclass[plyr->pclass].armor_increment[i];
		}
		for(i = 0; i < std::to_underlying(WeaponType::HexenCount); i++)
		{
			plyr->weaponowned[i] = true;
		}
		for(i = 0; i < std::to_underlying(AmmoType::ManaCount); i++)
		{
			plyr->ammo[i] = MAX_MANA;
		}
	}
	else
	{
		if(!plyr->backpack)
		{
			for(i = 0; i < std::to_underlying(AmmoType::Count); i++)
				plyr->maxammo[i] *= 2;
			plyr->backpack = true;
		}

		plyr->armorpoints[std::to_underlying(ArmorType::Armor)] = idfa_armor; // Ty 03/09/98 - deh
		plyr->armortype = idfa_armor_class;          // Ty 03/09/98 - deh

		// You can't own weapons that aren't in the game // phares 02/27/98
		for(i = 0; i < std::to_underlying(WeaponType::Count); i++)
			if(!(((i == std::to_underlying(WeaponType::Plasma) || i == std::to_underlying(WeaponType::Bfg)) && gamemode == GameMode::Shareware) ||
				(i == std::to_underlying(WeaponType::Supershotgun) && gamemode != GameMode::Commercial)))
				plyr->weaponowned[i] = true;

		for(i = 0; i < std::to_underlying(AmmoType::Count); i++)
			if(i != std::to_underlying(AmmoType::Cell) || gamemode != GameMode::Shareware)
				plyr->ammo[i] = plyr->maxammo[i];

		dsda_AddMessage(s_STSTR_FAADDED);
	}
}

static void cheat_k()
{
	int i;
	for(i = 0; i < std::to_underlying(Card::Count); i++)
		if(!plyr->cards[i]) // only print message if at least one key added
		{
			// however, caller may overwrite message anyway
			plyr->cards[i] = true;
			dsda_AddMessage("Keys Added");
		}

	// heretic - reset status bar
	SB_Start();
}

static void cheat_kfa()
{
	cheat_k();
	cheat_fa();
	dsda_AddMessage(s_STSTR_KFAADDED);
}

void M_CheatNoClip()
{
	dsda_AddMessage((plyr->cheats ^= CF_NOCLIP) & CF_NOCLIP ? s_STSTR_NCON : s_STSTR_NCOFF);
}

static void cheat_noclip()
{
	if(demorecording)
	{
		dsda_QueueExCmdNoClip();
		return;
	}

	M_CheatNoClip();
}

// 'behold?' power-up cheats (modified for infinite duration -- killough)
static void cheat_pw(int pw)
{
	if(pw == std::to_underlying(PowerType::AllMap))
		dsda_TrackFeature(FeatureFlag::Automap);

	if(pw == std::to_underlying(PowerType::Infrared))
		dsda_TrackFeature(FeatureFlag::Liteamp);

	if(plyr->powers[pw])
		plyr->powers[pw] = pw != std::to_underlying(PowerType::Strength) && pw != std::to_underlying(PowerType::AllMap); // killough
	else
	{
		P_GivePower(plyr, static_cast<PowerType>(pw));
		if(pw != std::to_underlying(PowerType::Strength))
			plyr->powers[pw] = -1; // infinite duration -- killough
	}
	dsda_AddMessage(s_STSTR_BEHOLDX);
}

// 'behold' power-up menu
static void cheat_behold()
{
	dsda_AddMessage(s_STSTR_BEHOLD);
}

// check 'clev' change-level cheat
static void cheat_clev0()
{
	int epsd, map;
	char *cur, *next;
	cur = Z_Strdup(VANILLA_MAP_LUMP_NAME(gameepisode, gamemap));

	dsda_NextMap(&epsd, &map);
	next = VANILLA_MAP_LUMP_NAME(epsd, map);

	if(W_LumpNameExists(next))
		doom_printf("Current: %s, next: %s", cur, next);
	else
		doom_printf("Current: %s", cur);

	Z_Free(cur);
}

// 'clev' change-level cheat
static void cheat_clev(char buf[3])
{
	int epsd, map;

	if(gamemode == GameMode::Commercial)
	{
		epsd = 1; //jff was 0, but espd is 1-based
		map = (buf[0] - '0') * 10 + buf[1] - '0';
	}
	else
	{
		epsd = buf[0] - '0';
		map = buf[1] - '0';
	}

	if(dsda_ResolveCLEV(&epsd, &map))
	{
		dsda_AddMessage(s_STSTR_CLEV);

		G_DeferedInitNew(gameskill, epsd, map);
	}
}

// 'mypos' for player position
static void cheat_mypos()
{
	dsda_ToggleConfig(ConfigId::CoordinateDisplay, false);
}

// cph - cheat to toggle frame rate/rendering stats display
static void cheat_rate()
{
	dsda_ToggleRenderStats();
}

// check compatibility cheat
static void cheat_comp0()
{
	if(raven)
		return doom_printf("Cheat disabled for %s", heretic ? "Heretic" : "Hexen");

	doom_printf("Complevel: %i - %s", compatibility_level, comp_lev_str[std::to_underlying(compatibility_level)]);
}

// compatibility cheat
static void cheat_comp(char buf[3])
{
	int compinput = (buf[0] - '0') * 10 + buf[1] - '0';

	if(raven) return;

	if(compinput < 0 ||
		compinput >= std::to_underlying(CompLevel::Max) ||
		(compinput > 17 && compinput < 21))
	{
		return; //doom_printf("Invalid complevel");
	}
	else
	{
		compatibility_level = static_cast<CompLevel>(compinput);
		G_Compatibility(); // this is missing options checking
		doom_printf("New Complevel: %i - %s", compatibility_level, comp_lev_str[std::to_underlying(compatibility_level)]);
	}
}

// Get skill strings
static const char* dsda_skill_str()
{
	if(hexen)
	{
		if(PlayerClass[consoleplayer] == PClass::Fighter)
			return hexen_skill_fighter[gameskill];
		else if(PlayerClass[consoleplayer] == PClass::Cleric)
			return hexen_skill_cleric[gameskill];
		else if(PlayerClass[consoleplayer] == PClass::Mage)
			return hexen_skill_mage[gameskill];
	}

	return skill_infos[gameskill].name;
}

// Check skill cheat
static void cheat_skill0()
{
	if(!tc_game)
		doom_printf("Skill: %i - %s", gameskill + 1, dsda_skill_str());
	else
		doom_printf("Skill: %i", gameskill + 1);
}

// Skill cheat
static void cheat_skill(char buf[1])
{
	int skill = buf[0] - '0';

	if(skill >= 1 && skill <= num_skills)
	{
		gameskill = skill - 1;

		if(!tc_game)
			doom_printf("Next Level Skill: %i - %s", gameskill + 1, dsda_skill_str());
		else
			doom_printf("Next Level Skill: %i", gameskill + 1);

		dsda_UpdateGameSkill(gameskill);
	}
}

// variable friction cheat
static void cheat_friction()
{
	dsda_AddMessage((variable_friction = !variable_friction) ? "Variable Friction enabled" : "Variable Friction disabled");
}


// Pusher cheat
// phares 3/10/98
static void cheat_pushers()
{
	dsda_AddMessage((allow_pushers = !allow_pushers) ? "Pushers enabled" : "Pushers disabled");
}

extern "C" void A_PainDie(mobj_t*);
static void cheat_massacre() // jff 2/01/98 kill all monsters
{
	// jff 02/01/98 'em' cheat - kill all monsters
	// partially taken from Chi's .46 port
	//
	// killough 2/7/98: cleaned up code and changed to use dprintf;
	// fixed lost soul bug (LSs left behind when PEs are killed)

	int killcount = 0;
	thinker_t* currentthinker = nullptr;

	// killough 7/20/98: kill friendly monsters only if no others to kill
	MobjFlag mask = MobjFlag::Friend;
	P_MapStart();
	do
		while((currentthinker = P_NextThinker(currentthinker, ThinkerClass::All)) != nullptr)
			if(currentthinker->function == reinterpret_cast<think_t>(P_MobjThinker) &&
				(((mobj_t*)currentthinker)->flags & mask) == MobjFlag{} && // killough 7/20/98
				((((mobj_t*)currentthinker)->flags & MobjFlag::CountKill) != MobjFlag{} ||
					((mobj_t*)currentthinker)->type == MobjType::Skull))
			{
				// killough 3/6/98: kill even if PE is dead
				if(((mobj_t*)currentthinker)->health > 0)
				{
					killcount++;
					P_DamageMobj((mobj_t*)currentthinker, nullptr, nullptr, 10000);
				}
				if(((mobj_t*)currentthinker)->type == MobjType::Pain)
				{
					A_PainDie((mobj_t*)currentthinker); // killough 2/8/98
					P_SetMobjState((mobj_t*)currentthinker, StateId::PainDie6);
				}
			}
	while(!killcount && mask != MobjFlag{} ? mask = MobjFlag{}, 1 : 0); // killough 7/20/98
	P_MapEnd();
	// killough 3/22/98: make more intelligent about plural
	// Ty 03/27/98 - string(s) *not* externalized
	doom_printf("%d Monster%s Killed", killcount, killcount == 1 ? "" : "s");
}

void M_CheatIDDT()
{
	extern int dsda_reveal_map;

	dsda_TrackFeature(FeatureFlag::Iddt);

	dsda_reveal_map = (dsda_reveal_map + 1) % 3;
}

// killough 2/7/98: move iddt cheat from am_map.c to here
// killough 3/26/98: emulate Doom better
static void cheat_ddt()
{
	if(automap_input)
		M_CheatIDDT();
}

static void cheat_reveal_secret()
{
	static int last_secret = -1;

	if(automap_input)
	{
		int i, start_i;

		dsda_TrackFeature(FeatureFlag::Iddt);

		i = last_secret + 1;
		if(i >= numsectors)
			i = 0;
		start_i = i;

		do
		{
			sector_t* sec = &sectors[i];

			if(P_IsSecret(sec))
			{
				dsda_UpdateIntConfig(ConfigId::AutomapFollow, false, true);

				// This is probably not necessary
				if(sec->lines && sec->lines[0] && sec->lines[0]->v1)
				{
					AM_SetMapCenter(sec->lines[0]->v1->x, sec->lines[0]->v1->y);
					last_secret = i;
					break;
				}
			}

			i++;
			if(i >= numsectors)
				i = 0;
		}
		while(i != start_i);
	}
}

static void cheat_cycle_mobj(mobj_t** last_mobj, int* last_count, MobjFlag flags, int alive)
{
	extern int init_thinkers_count;
	thinker_t *th, *start_th;

	// If the thinkers have been wiped, addresses are invalid
	if(*last_count != init_thinkers_count)
	{
		*last_count = init_thinkers_count;
		*last_mobj = nullptr;
	}

	if(*last_mobj)
		th = &(*last_mobj)->thinker;
	else
		th = &thinkercap;

	start_th = th;

	do
	{
		th = th->next;
		if(th->function == reinterpret_cast<think_t>(P_MobjThinker))
		{
			mobj_t* mobj;

			mobj = (mobj_t*)th;

			if((mobj->intflags & MobjIntFlag::SpawnedByIcon) != MobjIntFlag{})
			{
				continue;
			}

			if((!alive || mobj->health > 0) && (mobj->flags & flags) != MobjFlag{})
			{
				dsda_UpdateIntConfig(ConfigId::AutomapFollow, false, true);
				AM_SetMapCenter(mobj->x, mobj->y);
				P_SetTarget(last_mobj, mobj);
				break;
			}
		}
	}
	while(th != start_th);
}

static void cheat_reveal_kill()
{
	if(automap_input)
	{
		static int last_count;
		static mobj_t* last_mobj;

		dsda_TrackFeature(FeatureFlag::Iddt);

		cheat_cycle_mobj(&last_mobj, &last_count, MobjFlag::CountKill, true);
	}
}

static void cheat_reveal_item()
{
	if(automap_input)
	{
		static int last_count;
		static mobj_t* last_mobj;

		dsda_TrackFeature(FeatureFlag::Iddt);

		cheat_cycle_mobj(&last_mobj, &last_count, MobjFlag::CountItem, false);
	}
}

// killough 2/7/98: HOM autodetection
static void cheat_hom()
{
	dsda_AddMessage(dsda_ToggleConfig(ConfigId::FlashingHom, true)
		? "HOM Detection On"
		: "HOM Detection Off");
}

// killough 3/6/98: -fast parameter toggle
static void cheat_fast()
{
	dsda_AddMessage(dsda_ToggleConfig(ConfigId::FastMonsters, true) ? "Fast Monsters On" : "Fast Monsters Off");
	dsda_RefreshGameSkill(); // refresh fast monsters
}

// killough 2/16/98: keycard/skullkey cheat functions
static void cheat_tntkey()
{
	dsda_AddMessage("Red, Yellow, Blue");
}

static void cheat_tntkeyx()
{
	dsda_AddMessage("Card, Skull");
}

static void cheat_tntkeyxx(int key)
{
	dsda_AddMessage((plyr->cards[key] = !plyr->cards[key]) ? "Key Added" : "Key Removed");
}

// killough 2/16/98: generalized weapon cheats

static void cheat_tntweap()
{
	dsda_AddMessage(gamemode == GameMode::Commercial ? "Weapon number 1-9" : "Weapon number 1-8");
}

static void cheat_tntweapx(char buf[3])
{
	int w = *buf - '1';

	if((w == std::to_underlying(WeaponType::Supershotgun) && gamemode != GameMode::Commercial) || // killough 2/28/98
		((w == std::to_underlying(WeaponType::Bfg) || w == std::to_underlying(WeaponType::Plasma)) && gamemode == GameMode::Shareware))
		return;

	if(w == std::to_underlying(WeaponType::Fist)) // make '1' apply beserker strength toggle
		cheat_pw(std::to_underlying(PowerType::Strength));
	else if(w >= 0 && w < std::to_underlying(WeaponType::Count))
	{
		if((plyr->weaponowned[w] = !plyr->weaponowned[w]))
			dsda_AddMessage("Weapon Added");
		else
		{
			dsda_AddMessage("Weapon Removed");
			if(w == std::to_underlying(plyr->readyweapon)) // maybe switch if weapon removed
				plyr->pendingweapon = P_SwitchWeapon(plyr);
		}
	}
}

// killough 2/16/98: generalized ammo cheats
static void cheat_tntammo()
{
	dsda_AddMessage("Ammo 1-4, Backpack");
}

static void cheat_tntammox(char buf[1])
{
	int a = *buf - '1';
	if(*buf == 'b') // Ty 03/27/98 - strings *not* externalized
		if((plyr->backpack = !plyr->backpack))
		{
			dsda_AddMessage("Backpack Added");
			for(a = 0; a < std::to_underlying(AmmoType::Count); a++)
				plyr->maxammo[a] <<= 1;
		}
		else
		{
			dsda_AddMessage("Backpack Removed");
			for(a = 0; a < std::to_underlying(AmmoType::Count); a++)
				if(plyr->ammo[a] > (plyr->maxammo[a] >>= 1))
					plyr->ammo[a] = plyr->maxammo[a];
		}
	else if(a >= 0 && a < std::to_underlying(AmmoType::Count)) // Ty 03/27/98 - *not* externalized
	{
		// killough 5/5/98: switch plasma and rockets for now -- KLUDGE
		a = a == std::to_underlying(AmmoType::Cell) ? std::to_underlying(AmmoType::Misl) : a == std::to_underlying(AmmoType::Misl) ? std::to_underlying(AmmoType::Cell) : a; // HACK
		dsda_AddMessage((plyr->ammo[a] = !plyr->ammo[a]) ? plyr->ammo[a] = plyr->maxammo[a], "Ammo Added" : "Ammo Removed");
	}
}

static void cheat_smart()
{
	dsda_AddMessage((monsters_remember = !monsters_remember) ? "Smart Monsters Enabled" : "Smart Monsters Disabled");
}

static void cheat_pitch()
{
	dsda_AddMessage(dsda_ToggleConfig(ConfigId::PitchedSounds, true)
		? "Pitch Effects Enabled"
		: "Pitch Effects Disabled");
}

static void cheat_notarget()
{
	plyr->cheats ^= CF_NOTARGET;
	if(plyr->cheats & CF_NOTARGET)
		dsda_AddMessage("Notarget Mode ON");
	else
		dsda_AddMessage("Notarget Mode OFF");
}

static void cheat_freeze()
{
	dsda_ToggleFrozenMode();
	dsda_AddMessage(dsda_FrozenMode() ? "FREEZE ON" : "FREEZE OFF");
}

static void cheat_fly()
{
	if(plyr->mo != nullptr)
	{
		if(raven)
		{
			if(plyr->powers[std::to_underlying(PowerType::Flight)])
			{
				P_PlayerEndFlight(plyr);
				plyr->powers[std::to_underlying(PowerType::Flight)] = 0;
				dsda_AddMessage("Fly mode OFF");
			}
			else
			{
				P_GivePower(plyr, PowerType::Flight);
				plyr->powers[std::to_underlying(PowerType::Flight)] = INT_MAX;
				dsda_AddMessage("Fly mode ON");
			}
		}
		else
		{
			plyr->cheats ^= CF_FLY;
			if(plyr->cheats & CF_FLY)
			{
				plyr->mo->flags |= MobjFlag::NoGravity;
				plyr->mo->flags |= MobjFlag::Fly;
				dsda_AddMessage("Fly mode ON");
			}
			else
			{
				plyr->mo->flags -= MobjFlag::NoGravity;
				plyr->mo->flags -= MobjFlag::Fly;
				dsda_AddMessage("Fly mode OFF");
			}
		}
	}
}

static dboolean M_ClassicDemo()
{
	return (demorecording || demoplayback) && !dsda_AllowCasualExCmdFeatures();
}

static dboolean M_CheatAllowed(CheatWhen when)
{
	return !dsda_StrictMode() &&
		!((when & CheatWhen::NotDemo) != CheatWhen{} && (demorecording || demoplayback)) &&
		!((when & CheatWhen::NotClassicDemo) != CheatWhen{} && M_ClassicDemo()) &&
		!((when & CheatWhen::NotMenu) != CheatWhen{} && menuactive != MenuActive::Inactive);
}

static void cht_InitCheats()
{
	static int init = false;

	if(!init)
	{
		cheatseq_t* cht;

		init = true;

		for(cht = cheat; cht->cheat; cht++)
		{
			cht->sequence_len = strlen(cht->cheat);
		}
	}
}

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
static int M_FindCheats(int key)
{
	int rc = 0;
	cheatseq_t* cht;
	char char_key;

	cht_InitCheats();

	char_key = (char)key;

	for(cht = cheat; cht->cheat; cht++)
	{
		if(M_CheatAllowed(cht->when))
		{
			if(cht->chars_read < cht->sequence_len)
			{
				// still reading characters from the cheat code
				// and verifying.  reset back to the beginning
				// if a key is wrong

				if(char_key == cht->cheat[cht->chars_read])
					++cht->chars_read;
				else if(char_key == cht->cheat[0])
					cht->chars_read = 1;
				else
					cht->chars_read = 0;

				cht->param_chars_read = 0;
			}
			else if(cht->param_chars_read < -cht->arg)
			{
				// we have passed the end of the cheat sequence and are
				// entering parameters now

				cht->parameter_buf[cht->param_chars_read] = char_key;

				++cht->param_chars_read;

				// affirmative response
				rc = 1;
			}

			if(cht->chars_read >= cht->sequence_len &&
				cht->param_chars_read >= -cht->arg)
			{
				if(cht->param_chars_read)
				{
					static char argbuf[CHEAT_ARGS_MAX + 1];

					// process the arg buffer
					memcpy(argbuf, cht->parameter_buf, -cht->arg);

					reinterpret_cast<void (*)(char*)>(cht->func)(argbuf);
				}
				else
				{
					// call cheat handler
					reinterpret_cast<void (*)(int)>(cht->func)(cht->arg);

					if(cht->repeatable)
					{
						--cht->chars_read;
					}
				}

				if(!cht->repeatable)
					cht->chars_read = cht->param_chars_read = 0;
				rc = 1;
			}
		}
	}

	return rc;
}

typedef struct cheat_input_s
{
	InputId input;
	const CheatWhen when;
	void (*const func)();
	const int arg;
} cheat_input_t;

static cheat_input_t cheat_input[] = {
	{InputId::Iddqd, CheatWhen::NotClassicDemo, cheat_god, 0},
	{InputId::Idkfa, CheatWhen::NotDemo, cheat_kfa, 0},
	{InputId::Idfa, CheatWhen::NotDemo, cheat_fa, 0},
	{InputId::Idclip, CheatWhen::NotClassicDemo, cheat_noclip, 0},
	{InputId::Idbeholdh, CheatWhen::NotDemo, cheat_health, 0},
	{InputId::Idbeholdm, CheatWhen::NotDemo, cheat_megaarmour, 0},
	{InputId::Idbeholdv, CheatWhen::NotDemo, reinterpret_cast<void (*)()>(cheat_pw), std::to_underlying(PowerType::Invulnerability)},
	{InputId::Idbeholds, CheatWhen::NotDemo, reinterpret_cast<void (*)()>(cheat_pw), std::to_underlying(PowerType::Strength)},
	{InputId::Idbeholdi, CheatWhen::NotDemo, reinterpret_cast<void (*)()>(cheat_pw), std::to_underlying(PowerType::Invisibility)},
	{InputId::Idbeholdr, CheatWhen::NotDemo, reinterpret_cast<void (*)()>(cheat_pw), std::to_underlying(PowerType::IronFeet)},
	{InputId::Idbeholda, CheatWhen::Always, reinterpret_cast<void (*)()>(cheat_pw), std::to_underlying(PowerType::AllMap)},
	{InputId::Idbeholdl, CheatWhen::Always, reinterpret_cast<void (*)()>(cheat_pw), std::to_underlying(PowerType::Infrared)},
	{InputId::Idmypos, CheatWhen::Always, cheat_mypos, 0},
	{InputId::Idrate, CheatWhen::Always, cheat_rate, 0},
	{InputId::Iddt, CheatWhen::Always, cheat_ddt, 0},
	{InputId::Ponce, CheatWhen::NotDemo, cheat_reset_health, 0},
	{InputId::Shazam, CheatWhen::NotDemo, cheat_tome, 0},
	{InputId::Chicken, CheatWhen::NotDemo, cheat_chicken, 0},
	{InputId::Notarget, CheatWhen::NotDemo, cheat_notarget, 0},
	{InputId::Freeze, CheatWhen::NotDemo, cheat_freeze, 0},
	{InputId::Null}
};

dboolean M_CheatResponder(event_t* ev)
{
	cheat_input_t* cheat_i;

	if(dsda_ProcessCheatCodes() &&
		ev->type == EventType::KeyDown &&
		M_FindCheats(ev->data1.i))
		return true;

	for(cheat_i = cheat_input; cheat_i->input != InputId::Null; cheat_i++)
	{
		if(dsda_InputActivated(cheat_i->input))
		{
			if(M_CheatAllowed(cheat_i->when))
				reinterpret_cast<void (*)(int)>(cheat_i->func)(cheat_i->arg);

			return true;
		}
	}

	if(M_CheatAllowed(CheatWhen::NotDemo) && dsda_InputActivated(InputId::Avj))
	{
		plyr->mo->momz = 1000 * FRACUNIT / plyr->mo->info->mass;

		return true;
	}

	return false;
}

dboolean M_CheatEntered(const char* element, const char* value)
{
	cheatseq_t* cheat_i;

	for(cheat_i = cheat; cheat_i->cheat; cheat_i++)
	{
		if(!strcmp(cheat_i->cheat, element) && M_CheatAllowed(cheat_i->when - CheatWhen::NotMenu))
		{
			if(cheat_i->arg >= 0)
				reinterpret_cast<void (*)(int)>(cheat_i->func)(cheat_i->arg);
			else
				reinterpret_cast<void (*)(const char*)>(cheat_i->func)(value);
			return true;
		}
	}
	return false;
}

// heretic

#include "p_user.hpp"

static void cheat_reset_health()
{
	if(heretic && plyr->chickenTics)
	{
		plyr->health = plyr->mo->health = MAXCHICKENHEALTH;
	}
	else if(hexen && plyr->morphTics)
	{
		plyr->health = plyr->mo->health = MAXMORPHHEALTH;
	}
	else
	{
		plyr->health = plyr->mo->health = MAXHEALTH;
	}
	dsda_AddMessage("FULL HEALTH");
}

static void cheat_artifact(char buf[3])
{
	int i;
	int j;
	int type;
	int count;

	if(!heretic) return;

	type = buf[0] - 'a' + 1;
	count = buf[1] - '0';
	if(type == 26 && count == 0)
	{
		// All artifacts
		for(i = std::to_underlying(ArtiType::None) + 1; i < std::to_underlying(ArtiType::Count); i++)
		{
			if(gamemode == GameMode::Shareware && (i == std::to_underlying(ArtiType::SuperHealth) || i == std::to_underlying(ArtiType::Teleport)))
			{
				continue;
			}
			for(j = 0; j < 16; j++)
			{
				P_GiveArtifact(plyr, static_cast<ArtiType>(i), nullptr);
			}
		}
		dsda_AddMessage("YOU GOT IT");
	}
	else if(type > std::to_underlying(ArtiType::None) && type < std::to_underlying(ArtiType::Count) && count > 0 && count < 10)
	{
		if(gamemode == GameMode::Shareware && (type == std::to_underlying(ArtiType::SuperHealth) || type == std::to_underlying(ArtiType::Teleport)))
		{
			dsda_AddMessage("BAD INPUT");
			return;
		}
		for(i = 0; i < count; i++)
		{
			P_GiveArtifact(plyr, static_cast<ArtiType>(type), nullptr);
		}
		dsda_AddMessage("YOU GOT IT");
	}
	else
	{
		dsda_AddMessage("BAD INPUT");
	}
}

static void cheat_tome()
{
	if(!heretic) return;

	if(plyr->powers[std::to_underlying(PowerType::WeaponLevel2)])
	{
		plyr->powers[std::to_underlying(PowerType::WeaponLevel2)] = 0;
		dsda_AddMessage("POWER OFF");
	}
	else
	{
		P_UseArtifact(plyr, ArtiType::TomeOfPower);
		dsda_AddMessage("POWER ON");
	}
}

static void cheat_chicken()
{
	if(!raven) return;

	P_MapStart();
	if(heretic)
	{
		if(plyr->chickenTics)
		{
			if(P_UndoPlayerChicken(plyr))
			{
				dsda_AddMessage("CHICKEN OFF");
			}
		}
		else if(P_ChickenMorphPlayer(plyr))
		{
			dsda_AddMessage("CHICKEN ON");
		}
	}
	else
	{
		if(plyr->morphTics)
		{
			P_UndoPlayerMorph(plyr);
		}
		else
		{
			P_MorphPlayer(plyr);
		}
		dsda_AddMessage("SQUEAL!!");
	}
	P_MapEnd();
}

// hexen

#include "hexen/p_acs.hpp"

static void cheat_init()
{
	if(dsda_ResolveINIT())
	{
		P_SetMessage(plyr, "LEVEL WARP", true);
	}
}

static void cheat_inventory()
{
	int i, j;
	int start, end;

	if(!raven) return;

	if(heretic)
	{
		start = std::to_underlying(ArtiType::None) + 1;
		end = std::to_underlying(ArtiType::Count);
	}
	else
	{
		start = std::to_underlying(ArtiType::HexenNone) + 1;
		end = std::to_underlying(ArtiType::HexenFirstpuzzitem);
	}

	for(i = start; i < end; i++)
	{
		for(j = 0; j < g_arti_limit; j++)
		{
			P_GiveArtifact(plyr, static_cast<ArtiType>(i), nullptr);
		}
	}
	P_SetMessage(plyr, "ALL ARTIFACTS", true);
}

static void cheat_puzzle()
{
	int i;

	if(!hexen) return;

	for(i = std::to_underlying(ArtiType::HexenFirstpuzzitem); i < std::to_underlying(ArtiType::HexenCount); i++)
	{
		P_GiveArtifact(plyr, static_cast<ArtiType>(i), nullptr);
	}
	P_SetMessage(plyr, "ALL PUZZLE ITEMS", true);
}

static void cheat_class(char buf[2])
{
	int i;
	int new_class;

	if(!hexen) return;

	if(plyr->morphTics)
	{
		// don't change class if the player is morphed
		return;
	}

	new_class = 1 + (buf[0] - '0');
	if(new_class > std::to_underlying(PClass::Mage) || new_class < std::to_underlying(PClass::Fighter))
	{
		P_SetMessage(plyr, "INVALID PLAYER CLASS", true);
		return;
	}
	plyr->pclass = static_cast<PClass>(new_class);
	for(i = 0; i < std::to_underlying(ArmorType::Count); i++)
	{
		plyr->armorpoints[i] = 0;
	}
	PlayerClass[consoleplayer] = static_cast<PClass>(new_class);
	P_PostMorphWeapon(plyr, WeaponType::First);
	SB_SetClassData();
	SB_Start();
	P_SetMessage(plyr, "CLASS CHANGED", true);
}

static void cheat_script(char buf[3])
{
	int script;
	byte script_args[3];
	int tens, ones;
	static char textBuffer[40];

	if(!map_format.acs) return;

	tens = buf[0] - '0';
	ones = buf[1] - '0';
	script = tens * 10 + ones;
	if(script < 1)
		return;
	if(script > 99)
		return;
	script_args[0] = script_args[1] = script_args[2] = 0;

	if(P_StartACS(script, 0, script_args, plyr->mo, nullptr, 0))
	{
		snprintf(textBuffer, sizeof(textBuffer), "RUNNING SCRIPT %.2d", script);
		P_SetMessage(plyr, textBuffer, true);
	}
}
