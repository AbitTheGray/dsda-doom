// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Global - define top level globals for doom vs heretic

#include <utility>

#include <stdlib.h>
#include <string.h>

#include "info.hpp"
#include "d_items.hpp"
#include "p_inter.hpp"
#include "p_spec.hpp"
#include "p_map.hpp"
#include "sounds.hpp"
#include "d_main.hpp"
#include "v_video.hpp"
#include "hu_stuff.hpp"
#include "heretic/def.hpp"

#include "global.hpp"

#include "dsda/args.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mobjinfo.hpp"
#include "dsda/music.hpp"
#include "dsda/sfx.hpp"
#include "dsda/sprite.hpp"
#include "dsda/state.hpp"

#define IGNORE_VALUE -1

const demostate_t (*demostates)[4];
extern const demostate_t doom_demostates[][4];
extern const demostate_t heretic_demostates[][4];
extern const demostate_t hexen_demostates[][4];

weaponinfo_t* weaponinfo;

int g_maxplayers = 4;
int g_viewheight = 41 * FRACUNIT;
int g_numammo;

MobjType g_mt_player;
MobjType g_mt_tfog;
MobjType g_mt_blood;
MobjType g_skullpop_mt;
StateId g_s_bloodyskullx1;
StateId g_s_bloodyskullx2;
StateId g_s_play_fdth20;

int g_wp_fist;
int g_wp_chainsaw;
int g_wp_pistol;

int g_telefog_height;
int g_thrust_factor;
int g_fuzzy_aim_shift;
int g_jump;

StateId g_s_null;

MobjType g_mt_bloodsplatter;
int g_bloodsplatter_shift;
int g_bloodsplatter_weight;
int g_mons_look_range;
StateId g_hide_state;
MobjType g_lava_type;

int g_mntr_charge_speed;
SfxId g_mntr_atk1_sfx;
int g_mntr_decide_range;
int g_mntr_charge_rng;
int g_mntr_fire_rng;
StateId g_mntr_charge_state;
StateId g_mntr_fire_state;
MobjType g_mntr_charge_puff;
SfxId g_mntr_atk2_sfx;
int g_mntr_atk2_dice;
MobjType g_mntr_atk2_missile;
SfxId g_mntr_atk3_sfx;
int g_mntr_atk3_dice;
MobjType g_mntr_atk3_missile;
StateId g_mntr_atk3_state;
MobjType g_mntr_fire;

int g_arti_health;
int g_arti_superhealth;
int g_arti_fly;
int g_arti_limit;

SfxId g_sfx_sawup;
SfxId g_sfx_telept;
SfxId g_sfx_stnmov;
SfxId g_sfx_stnmov_plats;
SfxId g_sfx_swtchn;
SfxId g_sfx_swtchx;
SfxId g_sfx_dorcls;
SfxId g_sfx_doropn;
SfxId g_sfx_dorlnd;
SfxId g_sfx_pstart;
SfxId g_sfx_pstop;
SfxId g_sfx_itemup;
SfxId g_sfx_pistol;
SfxId g_sfx_oof;
SfxId g_sfx_menu;
SfxId g_sfx_respawn;
SfxId g_sfx_secret;
SfxId g_sfx_revive;
SfxId g_sfx_console;

// Optional menu/intermission sounds
SfxId g_sfx_mnuopn;
SfxId g_sfx_mnucls;
SfxId g_sfx_mnuact;
SfxId g_sfx_mnubak;
SfxId g_sfx_mnumov;
SfxId g_sfx_mnusli;
SfxId g_sfx_mnusel;
SfxId g_sfx_mnuerr;
SfxId g_sfx_inttic;
SfxId g_sfx_inttot;
SfxId g_sfx_intnex;
SfxId g_sfx_intnet;
SfxId g_sfx_intdms;

int g_door_normal;
int g_door_raise_in_5_mins;
int g_door_open;

int g_st_height;
int g_border_offset;
MobjFlag g_mf_translucent;
MobjFlag g_mf_shadow;

const char* g_menu_flat;
int g_menu_save_page_size;
int g_menu_font_spacing;

const char* g_skyflatname;

dboolean started_demo = false;
dboolean hexen = false;
dboolean heretic = false;
dboolean raven = false;

static void dsda_InitDoom()
{
	int i;
	doom_mobjinfo_t* mobjinfo_p;

	dsda_InitializeMobjInfo(std::to_underlying(MobjType::DoomZero), std::to_underlying(MobjType::DoomCount), std::to_underlying(MobjType::DoomCount));
	dsda_InitializeStates(doom_states, std::to_underlying(StateId::DoomCount));
	dsda_InitializeSprites(doom_sprnames, std::to_underlying(SpriteId::DoomNumsprites));
	dsda_InitializeSFX(doom_S_sfx, std::to_underlying(SfxId::DoomCount));
	dsda_InitializeMusic(doom_S_music, std::to_underlying(MusicId::DoomNummusic));

	demostates = doom_demostates;

	weaponinfo = doom_weaponinfo;

	g_maxplayers = 4;
	g_viewheight = 41 * FRACUNIT;
	g_numammo = std::to_underlying(AmmoType::DoomCount);

	g_mt_player = MobjType::Player;
	g_mt_tfog = MobjType::Tfog;
	g_mt_blood = MobjType::Blood;
	g_skullpop_mt = MobjType::Null;

	g_wp_fist = std::to_underlying(WeaponType::Fist);
	g_wp_chainsaw = std::to_underlying(WeaponType::Chainsaw);
	g_wp_pistol = std::to_underlying(WeaponType::Pistol);

	g_telefog_height = 0;
	g_thrust_factor = 100;
	g_fuzzy_aim_shift = 20;
	g_jump = 8;

	g_s_null = StateId::Null;

	g_sfx_sawup = SfxId::Sawup;
	g_sfx_telept = SfxId::Telept;
	g_sfx_stnmov = SfxId::Stnmov;
	g_sfx_stnmov_plats = SfxId::Stnmov;
	g_sfx_swtchn = SfxId::Swtchn;
	g_sfx_swtchx = SfxId::Swtchx;
	g_sfx_dorcls = SfxId::Dorcls;
	g_sfx_doropn = SfxId::Doropn;
	g_sfx_dorlnd = SfxId::Dorcls;
	g_sfx_pstart = SfxId::Pstart;
	g_sfx_pstop = SfxId::Pstop;
	g_sfx_itemup = SfxId::Itemup;
	g_sfx_pistol = SfxId::Pistol;
	g_sfx_oof = SfxId::Oof;
	g_sfx_menu = SfxId::Pstop;
	g_sfx_secret = SfxId::Secret;
	g_sfx_revive = SfxId::Slop;
	g_sfx_console = SfxId::Radio;

	// Optional menu/intermission sounds
	g_sfx_mnuopn = SfxId::Mnuopn;
	g_sfx_mnucls = SfxId::Mnucls;
	g_sfx_mnuact = SfxId::Mnuact;
	g_sfx_mnubak = SfxId::Mnubak;
	g_sfx_mnumov = SfxId::Mnumov;
	g_sfx_mnusli = SfxId::Mnusli;
	g_sfx_mnusel = SfxId::Mnusel;
	g_sfx_mnuerr = SfxId::Mnuerr;
	g_sfx_inttic = SfxId::Inttic;
	g_sfx_inttot = SfxId::Inttot;
	g_sfx_intnex = SfxId::Intnex;
	g_sfx_intnet = SfxId::Intnet;
	g_sfx_intdms = SfxId::Intdms;

	g_door_normal = std::to_underlying(VerticalDoorType::Normal);
	g_door_raise_in_5_mins = std::to_underlying(VerticalDoorType::WaitRaiseDoor);
	g_door_open = std::to_underlying(VerticalDoorType::OpenDoor);

	g_st_height = 32;
	g_border_offset = 8;
	g_mf_translucent = MobjFlag::Translucent;
	g_mf_shadow = MobjFlag::Shadow;

	g_menu_flat = "FLOOR4_6";
	g_menu_save_page_size = 7;
	g_menu_font_spacing = -1;

	g_skyflatname = "F_SKY1";

	// convert doom mobj types to shared type
	for(i = 0; i < std::to_underlying(MobjType::DoomCount); ++i)
	{
		mobjinfo_p = &doom_mobjinfo[i];

		mobjinfo[i].doomednum = mobjinfo_p->doomednum;
		mobjinfo[i].spawnstate = mobjinfo_p->spawnstate;
		mobjinfo[i].spawnhealth = mobjinfo_p->spawnhealth;
		mobjinfo[i].seestate = mobjinfo_p->seestate;
		mobjinfo[i].seesound = mobjinfo_p->seesound;
		mobjinfo[i].reactiontime = mobjinfo_p->reactiontime;
		mobjinfo[i].attacksound = mobjinfo_p->attacksound;
		mobjinfo[i].painstate = mobjinfo_p->painstate;
		mobjinfo[i].painchance = mobjinfo_p->painchance;
		mobjinfo[i].painsound = mobjinfo_p->painsound;
		mobjinfo[i].meleestate = mobjinfo_p->meleestate;
		mobjinfo[i].missilestate = mobjinfo_p->missilestate;
		mobjinfo[i].deathstate = mobjinfo_p->deathstate;
		mobjinfo[i].xdeathstate = mobjinfo_p->xdeathstate;
		mobjinfo[i].deathsound = mobjinfo_p->deathsound;
		mobjinfo[i].speed = mobjinfo_p->speed;
		mobjinfo[i].radius = mobjinfo_p->radius;
		mobjinfo[i].height = mobjinfo_p->height;
		mobjinfo[i].mass = mobjinfo_p->mass;
		mobjinfo[i].damage = mobjinfo_p->damage;
		mobjinfo[i].activesound = mobjinfo_p->activesound;
		mobjinfo[i].flags = mobjinfo_p->flags;
		mobjinfo[i].raisestate = mobjinfo_p->raisestate;
		mobjinfo[i].droppeditem = MobjType::Null;
		mobjinfo[i].crashstate = StateId::Null; // not in doom
		mobjinfo[i].flags2 = MobjFlag2{};     // not in doom

		// mbf21
		mobjinfo[i].infighting_group = std::to_underlying(InfightingGroup::Default);
		mobjinfo[i].projectile_group = std::to_underlying(ProjectileGroup::Default);
		mobjinfo[i].splash_group = std::to_underlying(SplashGroup::Default);
		mobjinfo[i].ripsound = SfxId::None;
		mobjinfo[i].altspeed = NO_ALTSPEED;
		mobjinfo[i].meleerange = MELEERANGE;

		// misc
		mobjinfo[i].bloodcolor = 0; // default
		mobjinfo[i].visibility = VF_DOOM;
	}

	// don't want to reorganize info.c structure for a few tweaks...
	mobjinfo[std::to_underlying(MobjType::Wolfss)].droppeditem = MobjType::Clip;
	mobjinfo[std::to_underlying(MobjType::Possessed)].droppeditem = MobjType::Clip;
	mobjinfo[std::to_underlying(MobjType::Shotguy)].droppeditem = MobjType::Shotgun;
	mobjinfo[std::to_underlying(MobjType::Chainguy)].droppeditem = MobjType::Chaingun;

	mobjinfo[std::to_underlying(MobjType::Vile)].flags2 = MobjFlag2::ShortMRange | MobjFlag2::DmgIgnored | MobjFlag2::NoThreshold;
	mobjinfo[std::to_underlying(MobjType::Cyborg)].flags2 = MobjFlag2::NoRadiusDmg | MobjFlag2::HigherMProb | MobjFlag2::RangeHalf |
		MobjFlag2::FullVolSounds | MobjFlag2::E2M8Boss | MobjFlag2::E4M6Boss;
	mobjinfo[std::to_underlying(MobjType::Spider)].flags2 = MobjFlag2::NoRadiusDmg | MobjFlag2::RangeHalf | MobjFlag2::FullVolSounds |
		MobjFlag2::E3M8Boss | MobjFlag2::E4M8Boss;
	mobjinfo[std::to_underlying(MobjType::Skull)].flags2 = MobjFlag2::RangeHalf;
	mobjinfo[std::to_underlying(MobjType::Fatso)].flags2 = MobjFlag2::Map07Boss1;
	mobjinfo[std::to_underlying(MobjType::Baby)].flags2 = MobjFlag2::Map07Boss2;
	mobjinfo[std::to_underlying(MobjType::Bruiser)].flags2 = MobjFlag2::E1M8Boss;
	mobjinfo[std::to_underlying(MobjType::Undead)].flags2 = MobjFlag2::LongMelee | MobjFlag2::RangeHalf;

	mobjinfo[std::to_underlying(MobjType::Bruiser)].projectile_group = std::to_underlying(ProjectileGroup::Baron);
	mobjinfo[std::to_underlying(MobjType::Knight)].projectile_group = std::to_underlying(ProjectileGroup::Baron);

	mobjinfo[std::to_underlying(MobjType::Bruisershot)].altspeed = 20 * FRACUNIT;
	mobjinfo[std::to_underlying(MobjType::Headshot)].altspeed = 20 * FRACUNIT;
	mobjinfo[std::to_underlying(MobjType::Troopshot)].altspeed = 20 * FRACUNIT;

	for(i = std::to_underlying(StateId::SargRun1); i <= std::to_underlying(StateId::SargPain2); ++i)
		states[i].flags |= STATEF_SKILL5FAST;
}

static void dsda_InitHeretic()
{
	int i, j;
	raven_mobjinfo_t* mobjinfo_p;

	dsda_InitializeMobjInfo(std::to_underlying(MobjType::HereticZero), std::to_underlying(MobjType::HereticCount), std::to_underlying(MobjType::HereticCount));
	dsda_InitializeStates(heretic_states, std::to_underlying(StateId::HereticCount));
	dsda_InitializeSprites(heretic_sprnames, std::to_underlying(SpriteId::HereticCount));
	dsda_InitializeSFX(heretic_S_sfx, std::to_underlying(SfxId::HereticCount));
	dsda_InitializeMusic(heretic_S_music, std::to_underlying(MusicId::HereticCount));

	demostates = heretic_demostates;

	weaponinfo = wpnlev1info;

	g_maxplayers = 4;
	g_viewheight = 41 * FRACUNIT;
	g_numammo = std::to_underlying(AmmoType::HereticCount);

	g_mt_player = MobjType::HereticPlayer;
	g_mt_tfog = MobjType::HereticTfog;
	g_mt_blood = MobjType::HereticBlood;
	g_skullpop_mt = MobjType::HereticBloodyskull;
	g_s_bloodyskullx1 = StateId::HereticBloodyskullx1;
	g_s_bloodyskullx2 = StateId::HereticBloodyskullx2;
	g_s_play_fdth20 = StateId::HereticPlayFdth20;

	g_wp_fist = std::to_underlying(WeaponType::Staff);
	g_wp_chainsaw = std::to_underlying(WeaponType::Gauntlets);
	g_wp_pistol = std::to_underlying(WeaponType::GoldWand);

	g_telefog_height = TELEFOGHEIGHT;
	g_thrust_factor = 150;
	g_fuzzy_aim_shift = 21;
	g_jump = 8;

	g_s_null = StateId::HereticNull;

	g_mt_bloodsplatter = MobjType::HereticBloodsplatter;
	g_bloodsplatter_shift = 9;
	g_bloodsplatter_weight = 2;
	g_mons_look_range = 20 * 64 * FRACUNIT;
	g_hide_state = StateId::HereticHidespecial1;
	g_lava_type = MobjType::HereticPhoenixfx2;

	g_mntr_atk1_sfx = SfxId::HereticStfpow;
	g_mntr_charge_speed = 13 * FRACUNIT;
	g_mntr_decide_range = 8;
	g_mntr_charge_rng = 150;
	g_mntr_charge_state = StateId::HereticMntrAtk41;
	g_mntr_fire_rng = 220;
	g_mntr_fire_state = StateId::HereticMntrAtk31;
	g_mntr_charge_puff = MobjType::HereticPhoenixpuff;
	g_mntr_atk2_sfx = SfxId::HereticMinat2;
	g_mntr_atk2_dice = 5;
	g_mntr_atk2_missile = MobjType::HereticMntrfx1;
	g_mntr_atk3_sfx = SfxId::HereticMinat1;
	g_mntr_atk3_dice = 5;
	g_mntr_atk3_missile = MobjType::HereticMntrfx2;
	g_mntr_atk3_state = StateId::HereticMntrAtk34;
	g_mntr_fire = MobjType::HereticMntrfx3;

	g_arti_health = std::to_underlying(ArtiType::Health);
	g_arti_superhealth = std::to_underlying(ArtiType::SuperHealth);
	g_arti_fly = std::to_underlying(ArtiType::Fly);
	g_arti_limit = 16;

	g_sfx_sawup = SfxId::HereticGntact;
	g_sfx_telept = SfxId::HereticTelept;
	g_sfx_stnmov = SfxId::HereticDormov;
	g_sfx_stnmov_plats = SfxId::HereticStnmov;
	g_sfx_swtchn = SfxId::HereticSwitch;
	g_sfx_swtchx = SfxId::HereticSwitch;
	g_sfx_dorcls = SfxId::HereticDoropn;
	g_sfx_doropn = SfxId::HereticDoropn;
	g_sfx_dorlnd = SfxId::HereticDorcls;
	g_sfx_pstart = SfxId::HereticPstart;
	g_sfx_pstop = SfxId::HereticPstop;
	g_sfx_itemup = SfxId::HereticItemup;
	g_sfx_pistol = SfxId::HereticGldhit;
	g_sfx_oof = SfxId::HereticPlroof;
	g_sfx_menu = SfxId::HereticDorcls;
	g_sfx_secret = SfxId::HereticSecret;
	g_sfx_respawn = SfxId::HereticRespawn;
	g_sfx_revive = SfxId::HereticTelept;
	g_sfx_console = SfxId::HereticChat;

	// Optional menu/intermission sounds
	g_sfx_mnuopn = SfxId::HereticMnuopn;
	g_sfx_mnucls = SfxId::HereticMnucls;
	g_sfx_mnuact = SfxId::HereticMnuact;
	g_sfx_mnubak = SfxId::HereticMnubak;
	g_sfx_mnumov = SfxId::HereticMnumov;
	g_sfx_mnusli = SfxId::HereticMnusli;
	g_sfx_mnusel = SfxId::HereticMnusel;
	g_sfx_mnuerr = SfxId::HereticMnuerr;
	g_sfx_inttic = SfxId::HereticInttic;
	g_sfx_inttot = SfxId::HereticInttot;
	g_sfx_intnex = SfxId::HereticIntnex;
	g_sfx_intnet = SfxId::HereticIntnet;
	g_sfx_intdms = SfxId::HereticIntdms;

	g_door_normal = std::to_underlying(VerticalDoorType::VldNormal);
	g_door_raise_in_5_mins = std::to_underlying(VerticalDoorType::VldRaiseIn5Mins);
	g_door_open = std::to_underlying(VerticalDoorType::VldOpen);

	g_st_height = 42;
	g_border_offset = 4;
	g_mf_translucent = MobjFlag::Shadow;
	g_mf_shadow = MobjFlag{}; // doesn't exist in heretic

	g_menu_flat = "FLOOR30";
	g_menu_save_page_size = 5;
	g_menu_font_spacing = -1;

	g_skyflatname = "F_SKY1";

	// convert heretic mobj types to shared type
	for(i = 0; i < std::to_underlying(MobjType::HereticCount) - std::to_underlying(MobjType::HereticZero); ++i)
	{
		mobjinfo_p = &heretic_mobjinfo[i];

		j = i + std::to_underlying(MobjType::HereticZero);
		mobjinfo[j].doomednum = mobjinfo_p->doomednum;
		mobjinfo[j].spawnstate = mobjinfo_p->spawnstate;
		mobjinfo[j].spawnhealth = mobjinfo_p->spawnhealth;
		mobjinfo[j].seestate = mobjinfo_p->seestate;
		mobjinfo[j].seesound = mobjinfo_p->seesound;
		mobjinfo[j].reactiontime = mobjinfo_p->reactiontime;
		mobjinfo[j].attacksound = mobjinfo_p->attacksound;
		mobjinfo[j].painstate = mobjinfo_p->painstate;
		mobjinfo[j].painchance = mobjinfo_p->painchance;
		mobjinfo[j].painsound = mobjinfo_p->painsound;
		mobjinfo[j].meleestate = mobjinfo_p->meleestate;
		mobjinfo[j].missilestate = mobjinfo_p->missilestate;
		mobjinfo[j].deathstate = mobjinfo_p->deathstate;
		mobjinfo[j].xdeathstate = mobjinfo_p->xdeathstate;
		mobjinfo[j].deathsound = mobjinfo_p->deathsound;
		mobjinfo[j].speed = mobjinfo_p->speed;
		mobjinfo[j].radius = mobjinfo_p->radius;
		mobjinfo[j].height = mobjinfo_p->height;
		mobjinfo[j].mass = mobjinfo_p->mass;
		mobjinfo[j].damage = mobjinfo_p->damage;
		mobjinfo[j].activesound = mobjinfo_p->activesound;
		mobjinfo[j].flags = mobjinfo_p->flags;
		mobjinfo[j].raisestate = StateId::Null;  // not in heretic
		mobjinfo[j].droppeditem = {}; // not in heretic
		mobjinfo[j].crashstate = mobjinfo_p->crashstate;
		mobjinfo[j].flags2 = mobjinfo_p->flags2;

		// mbf21
		mobjinfo[j].infighting_group = std::to_underlying(InfightingGroup::Default);
		mobjinfo[j].projectile_group = std::to_underlying(ProjectileGroup::Default);
		mobjinfo[j].splash_group = std::to_underlying(SplashGroup::Default);
		mobjinfo[j].ripsound = SfxId::None;
		mobjinfo[j].altspeed = NO_ALTSPEED;
		mobjinfo[j].meleerange = MELEERANGE;

		// misc
		mobjinfo[j].bloodcolor = 0; // default
		mobjinfo[j].visibility = VF_HERETIC;
	}

	// heretic doesn't use "clip" concept
	for(i = 0; i < std::to_underlying(AmmoType::Count); ++i) clipammo[i] = 1;

	// so few it's not worth implementing a pointer swap
	maxammo[0] = 100; // gold wand
	maxammo[1] = 50;  // crossbow
	maxammo[2] = 200; // blaster
	maxammo[3] = 200; // skull rod
	maxammo[4] = 20;  // phoenix rod
	maxammo[5] = 150; // mace
}

extern "C" void P_UseHexenRNG();
static void dsda_InitHexen()
{
	int i, j;
	raven_mobjinfo_t* mobjinfo_p;

	dsda_InitializeMobjInfo(std::to_underlying(MobjType::HexenZero), std::to_underlying(MobjType::HexenCount), std::to_underlying(MobjType::TotalCount));
	dsda_InitializeStates(hexen_states, std::to_underlying(StateId::HexenCount));
	dsda_InitializeSprites(hexen_sprnames, std::to_underlying(SpriteId::HexenCount));
	dsda_InitializeSFX(hexen_S_sfx, std::to_underlying(SfxId::HexenCount));
	dsda_InitializeMusic(hexen_S_music.data(), std::to_underlying(MusicId::HexenCount));

	demostates = hexen_demostates;

	// weaponinfo = wpnlev1info;

	g_maxplayers = 8;
	g_viewheight = 48 * FRACUNIT;
	g_numammo = std::to_underlying(AmmoType::ManaCount);

	// g_mt_player = HERETIC_MT_PLAYER;
	g_mt_tfog = MobjType::HexenTfog;
	g_mt_blood = MobjType::HexenBlood;
	g_skullpop_mt = MobjType::HexenBloodyskull;
	g_s_bloodyskullx1 = StateId::HexenBloodyskullx1;
	g_s_bloodyskullx2 = StateId::HexenBloodyskullx2;
	g_s_play_fdth20 = StateId::HexenPlayFdth20;

	// g_wp_fist = wp_staff;
	// g_wp_chainsaw = wp_gauntlets;
	// g_wp_pistol = wp_goldwand;

	g_telefog_height = TELEFOGHEIGHT;
	g_thrust_factor = 150;
	g_fuzzy_aim_shift = 21;
	g_jump = 9;

	g_s_null = StateId::HexenNull;

	g_mt_bloodsplatter = MobjType::HexenBloodsplatter;
	g_bloodsplatter_shift = 10;
	g_bloodsplatter_weight = 3;
	g_mons_look_range = 16 * 64 * FRACUNIT;
	g_hide_state = StateId::HexenHidespecial1;
	g_lava_type = MobjType::HexenCircleflame;

	g_mntr_atk1_sfx = SfxId::HexenMaulatorHammerSwing;
	g_mntr_charge_speed = 23 * FRACUNIT;
	g_mntr_decide_range = 16;
	g_mntr_charge_rng = 230;
	g_mntr_charge_state = StateId::HexenMntrAtk41;
	g_mntr_fire_rng = 100;
	g_mntr_fire_state = StateId::HexenMntrAtk31;
	g_mntr_charge_puff = MobjType::HexenPunchpuff;
	g_mntr_atk2_sfx = SfxId::HexenMaulatorHammerSwing;
	g_mntr_atk2_dice = 3;
	g_mntr_atk2_missile = MobjType::HexenMntrfx1;
	g_mntr_atk3_sfx = SfxId::HexenMaulatorHammerHit;
	g_mntr_atk3_dice = 3;
	g_mntr_atk3_missile = MobjType::HexenMntrfx2;
	g_mntr_atk3_state = StateId::HexenMntrAtk34;
	g_mntr_fire = MobjType::HexenMntrfx3;

	g_arti_health = std::to_underlying(ArtiType::HexenHealth);
	g_arti_superhealth = std::to_underlying(ArtiType::HexenSuperhealth);
	g_arti_fly = std::to_underlying(ArtiType::HexenFly);
	g_arti_limit = 25;

	g_sfx_telept = SfxId::HexenTeleport;
	g_sfx_stnmov = SfxId::HexenDoorLightClose;
	g_sfx_swtchn = SfxId::HexenFighterHammerHitwall;
	g_sfx_swtchx = SfxId::HexenFighterHammerHitwall;
	g_sfx_dorcls = SfxId::HexenDoorLightClose;
	g_sfx_doropn = SfxId::HexenDoorOpen;
	g_sfx_dorlnd = SfxId::HexenDoorLightClose;
	g_sfx_itemup = SfxId::HexenPickupKey;
	g_sfx_pistol = SfxId::HexenFighterHammerHitwall;
	g_sfx_oof = SfxId::HexenPlayerFighterGrunt;
	g_sfx_menu = SfxId::HexenDoorLightClose;
	g_sfx_secret = SfxId::HexenSecret;
	g_sfx_respawn = SfxId::HexenRespawn;
	g_sfx_revive = SfxId::HexenTeleport;
	g_sfx_console = SfxId::HexenChat;

	// Optional menu/intermission sounds
	g_sfx_mnuopn = SfxId::HexenMnuopn;
	g_sfx_mnucls = SfxId::HexenMnucls;
	g_sfx_mnuact = SfxId::HexenMnuact;
	g_sfx_mnubak = SfxId::HexenMnubak;
	g_sfx_mnumov = SfxId::HexenMnumov;
	g_sfx_mnusli = SfxId::HexenMnusli;
	g_sfx_mnusel = SfxId::HexenMnusel;
	g_sfx_mnuerr = SfxId::HexenMnuerr;
	g_sfx_inttic = SfxId::HexenInttic;
	g_sfx_inttot = SfxId::HexenInttot;
	g_sfx_intnex = SfxId::HexenIntnex;
	g_sfx_intnet = SfxId::HexenIntnet;
	g_sfx_intdms = SfxId::HexenIntdms;

	g_st_height = 39;
	g_border_offset = 4;
	g_mf_translucent = MobjFlag::Shadow; // hexen_note: how does ALTSHADOW fit in?
	g_mf_shadow = MobjFlag{};     // doesn't exist in hexen

	g_menu_flat = "F_032";
	g_menu_save_page_size = 5;
	g_menu_font_spacing = -1;

	g_skyflatname = "F_SKY";

	// convert hexen mobj types to shared type
	for(i = 0; i < std::to_underlying(MobjType::HexenCount) - std::to_underlying(MobjType::HexenZero); ++i)
	{
		mobjinfo_p = &hexen_mobjinfo[i];

		j = i + std::to_underlying(MobjType::HexenZero);
		mobjinfo[j].doomednum = mobjinfo_p->doomednum;
		mobjinfo[j].spawnstate = mobjinfo_p->spawnstate;
		mobjinfo[j].spawnhealth = mobjinfo_p->spawnhealth;
		mobjinfo[j].seestate = mobjinfo_p->seestate;
		mobjinfo[j].seesound = mobjinfo_p->seesound;
		mobjinfo[j].reactiontime = mobjinfo_p->reactiontime;
		mobjinfo[j].attacksound = mobjinfo_p->attacksound;
		mobjinfo[j].painstate = mobjinfo_p->painstate;
		mobjinfo[j].painchance = mobjinfo_p->painchance;
		mobjinfo[j].painsound = mobjinfo_p->painsound;
		mobjinfo[j].meleestate = mobjinfo_p->meleestate;
		mobjinfo[j].missilestate = mobjinfo_p->missilestate;
		mobjinfo[j].deathstate = mobjinfo_p->deathstate;
		mobjinfo[j].xdeathstate = mobjinfo_p->xdeathstate;
		mobjinfo[j].deathsound = mobjinfo_p->deathsound;
		mobjinfo[j].speed = mobjinfo_p->speed;
		mobjinfo[j].radius = mobjinfo_p->radius;
		mobjinfo[j].height = mobjinfo_p->height;
		mobjinfo[j].mass = mobjinfo_p->mass;
		mobjinfo[j].damage = mobjinfo_p->damage;
		mobjinfo[j].activesound = mobjinfo_p->activesound;
		mobjinfo[j].flags = mobjinfo_p->flags;
		mobjinfo[j].raisestate = StateId::Null;  // not in hexen
		mobjinfo[j].droppeditem = {}; // not in hexen
		mobjinfo[j].crashstate = mobjinfo_p->crashstate;
		mobjinfo[j].flags2 = mobjinfo_p->flags2;

		// mbf21
		mobjinfo[j].infighting_group = std::to_underlying(InfightingGroup::Default);
		mobjinfo[j].projectile_group = std::to_underlying(ProjectileGroup::Default);
		mobjinfo[j].splash_group = std::to_underlying(SplashGroup::Default);
		mobjinfo[j].ripsound = SfxId::None;
		mobjinfo[j].altspeed = NO_ALTSPEED;
		mobjinfo[j].meleerange = MELEERANGE;

		// misc
		mobjinfo[j].bloodcolor = 0; // default
		mobjinfo[j].visibility = VF_HEXEN;
	}

	{

		P_UseHexenRNG();
	}
}

static dboolean dsda_AutoDetectHeretic()
{
	dsda_arg_t* arg;
	int length;
	arg = dsda_Arg(ArgId::Iwad);
	if(arg->found)
	{
		length = strlen(arg->value.v_string);
		if(length >= 11 && !strnicmp(arg->value.v_string + length - 11, "heretic.wad", 11))
			return true;
		else if(length >= 12 && !strnicmp(arg->value.v_string + length - 12, "heretic1.wad", 12))
			return true;
	}

	return false;
}

static dboolean dsda_AutoDetectHexen()
{
	dsda_arg_t* arg;
	int length;
	arg = dsda_Arg(ArgId::Iwad);
	if(arg->found)
	{
		length = strlen(arg->value.v_string);
		if(length >= 9 && !strnicmp(arg->value.v_string + length - 9, "hexen.wad", 9))
			return true;
	}

	return false;
}

extern "C" void dsda_ResetNullPClass();

void dsda_InitGlobal()
{
	heretic = dsda_Flag(ArgId::Heretic) || dsda_AutoDetectHeretic();
	hexen = dsda_Flag(ArgId::Hexen) || dsda_AutoDetectHexen();
	raven = heretic || hexen;

	if(hexen)
		dsda_InitHexen();
	else if(heretic)
		dsda_InitHeretic();
	else
		dsda_InitDoom();

	dsda_ResetNullPClass();
}
