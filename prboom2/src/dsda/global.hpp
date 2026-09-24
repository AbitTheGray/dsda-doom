// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Global - define top level globals for doom vs heretic

#pragma once

#include "doomtype.hpp"

enum struct SfxId : int32_t;
enum struct MobjType : int32_t;
enum struct StateId : int32_t;

#ifdef __cplusplus
extern "C"
{
#endif

extern int g_maxplayers;
extern int g_viewheight;
extern int g_numammo;

extern MobjType g_mt_player;
extern MobjType g_mt_tfog;
extern MobjType g_mt_blood;
extern MobjType g_skullpop_mt;
extern StateId g_s_bloodyskullx1;
extern StateId g_s_bloodyskullx2;
extern StateId g_s_play_fdth20;

extern int g_wp_fist;
extern int g_wp_chainsaw;
extern int g_wp_pistol;

extern int g_telefog_height;
extern int g_thrust_factor;
extern int g_fuzzy_aim_shift;
extern int g_jump;

extern StateId g_s_null;

extern MobjType g_mt_bloodsplatter;
extern int g_bloodsplatter_shift;
extern int g_bloodsplatter_weight;
extern int g_mons_look_range;
extern StateId g_hide_state;
extern MobjType g_lava_type;

extern int g_mntr_charge_speed;
extern SfxId g_mntr_atk1_sfx;
extern int g_mntr_decide_range;
extern int g_mntr_charge_rng;
extern int g_mntr_fire_rng;
extern StateId g_mntr_charge_state;
extern StateId g_mntr_fire_state;
extern MobjType g_mntr_charge_puff;
extern SfxId g_mntr_atk2_sfx;
extern int g_mntr_atk2_dice;
extern MobjType g_mntr_atk2_missile;
extern SfxId g_mntr_atk3_sfx;
extern int g_mntr_atk3_dice;
extern MobjType g_mntr_atk3_missile;
extern StateId g_mntr_atk3_state;
extern MobjType g_mntr_fire;

extern int g_arti_health;
extern int g_arti_superhealth;
extern int g_arti_fly;
extern int g_arti_limit;

extern SfxId g_sfx_telept;
extern SfxId g_sfx_sawup;
extern SfxId g_sfx_stnmov;
extern SfxId g_sfx_stnmov_plats;
extern SfxId g_sfx_swtchn;
extern SfxId g_sfx_swtchx;
extern SfxId g_sfx_dorcls;
extern SfxId g_sfx_doropn;
extern SfxId g_sfx_dorlnd;
extern SfxId g_sfx_pstart;
extern SfxId g_sfx_pstop;
extern SfxId g_sfx_itemup;
extern SfxId g_sfx_pistol;
extern SfxId g_sfx_oof;
extern SfxId g_sfx_menu;
extern SfxId g_sfx_respawn;
extern SfxId g_sfx_secret;
extern SfxId g_sfx_revive;
extern SfxId g_sfx_console;

// Optional menu/intermission sounds
extern SfxId g_sfx_mnuopn;
extern SfxId g_sfx_mnucls;
extern SfxId g_sfx_mnuact;
extern SfxId g_sfx_mnubak;
extern SfxId g_sfx_mnumov;
extern SfxId g_sfx_mnusli;
extern SfxId g_sfx_mnusel;
extern SfxId g_sfx_mnuerr;
extern SfxId g_sfx_inttic;
extern SfxId g_sfx_inttot;
extern SfxId g_sfx_intnex;
extern SfxId g_sfx_intnet;
extern SfxId g_sfx_intdms;

extern int g_door_normal;
extern int g_door_raise_in_5_mins;
extern int g_door_open;

extern int g_st_height;
extern int g_border_offset;
extern int g_mf_translucent;
extern int g_mf_shadow;

extern const char* g_skyflatname;

void dsda_InitGlobal();

#ifdef __cplusplus
}
#endif
