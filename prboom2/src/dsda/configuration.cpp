// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Config

#include <algorithm>
#include <ranges>
#include <utility>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "cpp/EnumArray.hpp"
#include "cpp/Util.hpp"

#include "am_map.hpp"
#include "d_deh.hpp"
#include "doomdef.hpp"
#include "doomstat.hpp"
#include "hu_stuff.hpp"
#include "g_overflow.hpp"
#include "gl_struct.hpp"
#include "lprintf.hpp"
#include "r_main.hpp"
#include "r_segs.hpp"
#include "s_sound.hpp"
#include "st_stuff.hpp"
#include "smooth.hpp"
#include "v_video.hpp"
#include "z_zone.hpp"

#include "dsda/args.hpp"
#include "dsda/exhud.hpp"
#include "dsda/features.hpp"
#include "dsda/input.hpp"
#include "dsda/stretch.hpp"
#include "dsda/utility.hpp"

#include "configuration.hpp"

typedef union
{
	int v_int;
	char* v_string;
} dsda_config_value_t;

typedef union
{
	int v_int;
	const char* v_string;
} dsda_config_default_t;

enum struct ConfigFlag : uint8_t
{
	Strict = Bit<uint8_t>(0u),
	StrictRange = Bit<uint8_t>(1u),
	Even = Bit<uint8_t>(2u),
	Feature = Bit<uint8_t>(3u),
};
ENUM_FLAGS_FUNC(ConfigFlag)

typedef struct
{
	const char* name;
	ConfigId id;
	ConfigType type;
	int lower_limit;
	int upper_limit;
	dsda_config_default_t default_value;
	int* int_binding;
	ConfigFlag flags;
	int strict_lower_limit;
	int strict_upper_limit;
	void (*onUpdate)();
	dsda_config_value_t transient_value;
	dsda_config_value_t persistent_value;
} dsda_config_t;

#define CONF_BOOL(x) ConfigType::Int, 0, 1, { x }
#define CONF_COLOR(x) ConfigType::Int, 0, 255, { x }
#define CONF_BYTE(x) ConfigType::Int, 0, 255, { x }
#define CONF_STRING(x) ConfigType::String, 0, 0, { .v_string = x }
#define CONF_CR(x) ConfigType::Int, 0, std::to_underlying(ColorRange::HudLimit) - 1, { x }
#define CONF_WEAPON(x) ConfigType::Int, 0, 9, { x }

#define NOT_STRICT ConfigFlag{}, 0, 0
#define STRICT_INT(x) ConfigFlag::Feature | ConfigFlag::Strict, x, x
#define STRICT_RANGE(min, max) ConfigFlag::Feature | ConfigFlag::StrictRange, min, max

extern int dsda_input_profile;
extern int weapon_preferences[2][std::to_underlying(WeaponType::Count) + 1];
extern int demo_smoothturns;
extern int demo_smoothturnsfactor;
extern int sts_colored_numbers;
extern int sts_pct_always_gray;
extern int sts_traditional_keys;
extern int full_sounds;

extern "C" void I_Init2();
extern "C" void M_ChangeDemoSmoothTurns();
extern "C" void M_ChangeSkyMode();
extern "C" void M_ChangeMessages();
extern "C" void S_ResetSfxVolume();
extern "C" void I_ResetMusicVolume();
extern "C" void M_ChangeAllowFog();
extern "C" void gld_ResetShadowParameters();
extern "C" void gld_MultisamplingInit();
extern "C" void M_ChangeFOV();
extern "C" void I_InitMouse();
extern "C" void AccelChanging();
extern "C" void G_UpdateMouseSensitivity();
extern "C" void dsda_InitGameController();
extern "C" void M_ChangeSpeed();
extern "C" void M_ChangeShorttics();
extern "C" void I_InitSoundParams();
extern "C" void S_Init();
extern "C" void M_ChangeMIDIPlayer();
extern "C" void HU_InitCrosshair();
extern "C" void HU_InitThresholds();
extern "C" void dsda_InitAutoKeyFrames();
extern "C" void dsda_UpdateStretchParams();
extern "C" void dsda_InitCommandHistory();
extern "C" void dsda_InitQuickstartCache();
extern "C" void dsda_InitParallelSFXFilter();
extern "C" void M_ChangeMapMultisamling();
extern "C" void M_ChangeMapTextured();
extern "C" void AM_InitParams();
extern "C" void AM_initPlayerTrail();
extern "C" void gld_ResetAutomapTransparency();
extern "C" void M_ChangeVideoMode();
extern "C" void M_ChangeUncappedFrameRate();
extern "C" void M_ChangeFullScreen();
extern "C" void R_SetViewSize();
extern "C" void M_ChangeApplyPalette();
extern "C" void M_ChangeStretch();
extern "C" void M_ChangeAspectRatio();
extern "C" void dsda_RefreshLinearSky();
extern "C" void deh_changeCompTranslucency();
extern "C" void dsda_InitGameControllerParameters();
extern "C" void dsda_InitExHud();
extern "C" void dsda_UpdateFreeText();
extern "C" void dsda_ResetAirControl();
extern "C" void dsda_AlterGameFlags();
extern "C" void dsda_RefreshPistolStart();
extern "C" void dsda_RefreshAlwaysPistolStart();

void dsda_TrackConfigFeatures()
{
	if(!demorecording)
		return;

	if(R_PartialView() && dsda_IntConfig(ConfigId::Exhud))
		dsda_TrackFeature(FeatureFlag::Exhud);

	if(R_FullView() && dsda_IntConfig(ConfigId::HudDisplayed))
		dsda_TrackFeature(FeatureFlag::Advhud);

	if(dsda_IntConfig(ConfigId::GameSpeed) > 100)
		dsda_TrackFeature(FeatureFlag::Speedup);

	if(dsda_IntConfig(ConfigId::GameSpeed) < 100)
		dsda_TrackFeature(FeatureFlag::Slowdown);

	if(dsda_IntConfig(ConfigId::CoordinateDisplay) || dsda_IntConfig(ConfigId::MapCoordinates))
		dsda_TrackFeature(FeatureFlag::Coordinates);

	if(dsda_IntConfig(ConfigId::WeaponAttackAlignment))
		dsda_TrackFeature(FeatureFlag::Weaponalignment);

	if(dsda_IntConfig(ConfigId::CommandDisplay))
		dsda_TrackFeature(FeatureFlag::Commanddisplay);

	if(dsda_IntConfig(ConfigId::HudaddCrosshair))
		dsda_TrackFeature(FeatureFlag::Crosshair);

	if(dsda_IntConfig(ConfigId::HudaddCrosshairTarget))
		dsda_TrackFeature(FeatureFlag::Crosshaircolor);

	if(dsda_IntConfig(ConfigId::HudaddCrosshairLockTarget))
		dsda_TrackFeature(FeatureFlag::Crosshairlock);

	if(!dsda_IntConfig(ConfigId::PaletteOndamage))
		dsda_TrackFeature(FeatureFlag::Painpalette);

	if(!dsda_IntConfig(ConfigId::PaletteOnbonus))
		dsda_TrackFeature(FeatureFlag::Bonuspalette);

	if(!dsda_IntConfig(ConfigId::PaletteOnpowers))
		dsda_TrackFeature(FeatureFlag::Powerpalette);

	if(dsda_IntConfig(ConfigId::GlHealthBar))
		dsda_TrackFeature(FeatureFlag::Healthbar);

	if(dsda_IntConfig(ConfigId::MovementStrafe50))
		dsda_TrackFeature(FeatureFlag::Alwayssr50);

	if(dsda_IntConfig(ConfigId::MaxPlayerCorpse) != 32)
		dsda_TrackFeature(FeatureFlag::Maxplayercorpse);

	if(dsda_IntConfig(ConfigId::HideWeapon))
		dsda_TrackFeature(FeatureFlag::Hideweapon);

	if(dsda_IntConfig(ConfigId::ShowAliveMonsters))
		dsda_TrackFeature(FeatureFlag::Showalive);

	if(dsda_IntConfig(ConfigId::MapTextured) || dsda_IntConfig(ConfigId::ShowMinimap))
		dsda_TrackFeature(FeatureFlag::AdvancedMap);

	if(dsda_IntConfig(ConfigId::TranslucentSprites) > 1)
		dsda_TrackFeature(FeatureFlag::Vanillatrans);

	if(dsda_IntConfig(ConfigId::TranslucentGhosts))
		dsda_TrackFeature(FeatureFlag::Ghosttrans);
}

// TODO: migrate all kinds of stuff from M_Init

// TODO: automatically go through strict list
void dsda_UpdateStrictMode()
{
	I_Init2();       // side effect of realtic clock rate
	M_ChangeSpeed(); // side effect of always sr50
	dsda_InitAutoKeyFrames();
	M_ChangeSkyMode(); // affected by mouselook setting
	HU_InitCrosshair();
	M_ChangeApplyPalette();
	M_ChangeMapTextured();
	dsda_RefreshExHudCoordinateDisplay();
	dsda_RefreshExHudCommandDisplay();
	dsda_RefreshExHudMinimap();
	dsda_TrackConfigFeatures();
}

constinit EnumArray<dsda_config_t, ConfigId> dsda_config = {
	{At(ConfigId::GameSpeed), {
		"game_speed", ConfigId::GameSpeed,
		ConfigType::Int, 3, 10000, {100}, nullptr, STRICT_INT(100), I_Init2
	}},
	{At(ConfigId::DefaultComplevel), {
		"default_compatibility_level", ConfigId::DefaultComplevel,
		ConfigType::Int, 0, std::to_underlying(CompLevel::Mbf21), {std::to_underlying(CompLevel::Mbf21)}
	}},
	{At(ConfigId::DefaultSkill), {
		"default_skill", ConfigId::DefaultSkill,
		ConfigType::Int, 1, 5, {4}
	}},
	{At(ConfigId::VanillaKeymap), {
		"vanilla_keymap", ConfigId::VanillaKeymap,
		CONF_BOOL(0)
	}},
	{At(ConfigId::MenuBackground), {
		"menu_background", ConfigId::MenuBackground,
		ConfigType::Int, 0, 2, {1}
	}},
	{At(ConfigId::ProcessPriority), {
		"process_priority", ConfigId::ProcessPriority,
		ConfigType::Int, 0, 2, {0}
	}},
	{At(ConfigId::MaxPlayerCorpse), {
		"max_player_corpse", ConfigId::MaxPlayerCorpse,
		ConfigType::Int, -1, INT_MAX, {32}, nullptr, STRICT_INT(32)
	}},
	{At(ConfigId::InputProfile), {
		"input_profile", ConfigId::InputProfile,
		ConfigType::Int, 0, DSDA_INPUT_PROFILE_COUNT - 1, {0}, &dsda_input_profile
	}},
	{At(ConfigId::WeaponChoice1), {
		"weapon_choice_1", ConfigId::WeaponChoice1,
		CONF_WEAPON(6), &weapon_preferences[0][0]
	}},
	{At(ConfigId::WeaponChoice2), {
		"weapon_choice_2", ConfigId::WeaponChoice2,
		CONF_WEAPON(9), &weapon_preferences[0][1]
	}},
	{At(ConfigId::WeaponChoice3), {
		"weapon_choice_3", ConfigId::WeaponChoice3,
		CONF_WEAPON(4), &weapon_preferences[0][2]
	}},
	{At(ConfigId::WeaponChoice4), {
		"weapon_choice_4", ConfigId::WeaponChoice4,
		CONF_WEAPON(3), &weapon_preferences[0][3]
	}},
	{At(ConfigId::WeaponChoice5), {
		"weapon_choice_5", ConfigId::WeaponChoice5,
		CONF_WEAPON(2), &weapon_preferences[0][4]
	}},
	{At(ConfigId::WeaponChoice6), {
		"weapon_choice_6", ConfigId::WeaponChoice6,
		CONF_WEAPON(8), &weapon_preferences[0][5]
	}},
	{At(ConfigId::WeaponChoice7), {
		"weapon_choice_7", ConfigId::WeaponChoice7,
		CONF_WEAPON(5), &weapon_preferences[0][6]
	}},
	{At(ConfigId::WeaponChoice8), {
		"weapon_choice_8", ConfigId::WeaponChoice8,
		CONF_WEAPON(7), &weapon_preferences[0][7]
	}},
	{At(ConfigId::WeaponChoice9), {
		"weapon_choice_9", ConfigId::WeaponChoice9,
		CONF_WEAPON(1), &weapon_preferences[0][8]
	}},
	{At(ConfigId::FlashingHom), {
		"flashing_hom", ConfigId::FlashingHom,
		CONF_BOOL(0)
	}},
	{At(ConfigId::DemoSmoothturns), {
		"demo_smoothturns", ConfigId::DemoSmoothturns,
		CONF_BOOL(0), &demo_smoothturns,
		NOT_STRICT, M_ChangeDemoSmoothTurns
	}},
	{At(ConfigId::DemoSmoothturnsfactor), {
		"demo_smoothturnsfactor", ConfigId::DemoSmoothturnsfactor,
		ConfigType::Int, 1, SMOOTH_PLAYING_MAXFACTOR, {6}, &demo_smoothturnsfactor,
		NOT_STRICT, M_ChangeDemoSmoothTurns
	}},
	{At(ConfigId::WeaponAttackAlignment), {
		"weapon_attack_alignment", ConfigId::WeaponAttackAlignment,
		ConfigType::Int, 0, 3, {0}, nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::StsColoredNumbers), {
		"sts_colored_numbers", ConfigId::StsColoredNumbers,
		CONF_BOOL(0), &sts_colored_numbers
	}},
	{At(ConfigId::StsPctAlwaysGray), {
		"sts_pct_always_gray", ConfigId::StsPctAlwaysGray,
		CONF_BOOL(0), &sts_pct_always_gray
	}},
	{At(ConfigId::StsTraditionalKeys), {
		"sts_traditional_keys", ConfigId::StsTraditionalKeys,
		CONF_BOOL(0), &sts_traditional_keys
	}},
	{At(ConfigId::StsSolidBgColor), {
		"sts_solid_bg_color", ConfigId::StsSolidBgColor,
		CONF_BOOL(0), nullptr, NOT_STRICT, ST_SetResolution
	}},
	{At(ConfigId::StrictMode), {
		"dsda_strict_mode", ConfigId::StrictMode,
		CONF_BOOL(1), nullptr, NOT_STRICT, dsda_UpdateStrictMode
	}},
	{At(ConfigId::Vertmouse), {
		"movement_vertmouse", ConfigId::Vertmouse,
		CONF_BOOL(0)
	}},
	{At(ConfigId::Freelook), {
		"allow_freelook", ConfigId::Freelook,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeSkyMode
	}},
	{At(ConfigId::Autorun), {
		"autorun", ConfigId::Autorun,
		CONF_BOOL(1)
	}},
	{At(ConfigId::ShowMessages), {
		"show_messages", ConfigId::ShowMessages,
		CONF_BOOL(1), nullptr, NOT_STRICT, M_ChangeMessages
	}},
	{At(ConfigId::CommandDisplay), {
		"dsda_command_display", ConfigId::CommandDisplay,
		CONF_BOOL(0), nullptr, STRICT_INT(0), dsda_RefreshExHudCommandDisplay
	}},
	{At(ConfigId::CoordinateDisplay), {
		"dsda_coordinate_display", ConfigId::CoordinateDisplay,
		CONF_BOOL(0), nullptr, STRICT_INT(0), dsda_RefreshExHudCoordinateDisplay
	}},
	{At(ConfigId::ShowFps), {
		"dsda_show_fps", ConfigId::ShowFps,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_RefreshExHudFPS
	}},
	{At(ConfigId::ShowMinimap), {
		"dsda_show_minimap", ConfigId::ShowMinimap,
		CONF_BOOL(0), nullptr, STRICT_INT(0), dsda_RefreshExHudMinimap
	}},
	{At(ConfigId::ShowLevelSplits), {
		"dsda_show_level_splits", ConfigId::ShowLevelSplits,
		CONF_BOOL(1), nullptr, NOT_STRICT, dsda_RefreshExHudLevelSplits
	}},
	{At(ConfigId::Exhud), {
		"dsda_exhud", ConfigId::Exhud,
		CONF_BOOL(0), nullptr, ConfigFlag::Feature | NOT_STRICT, dsda_InitExHud
	}},
	{At(ConfigId::FreeText), {
		"dsda_free_text", ConfigId::FreeText,
		CONF_STRING(""), nullptr, NOT_STRICT, dsda_UpdateFreeText
	}},
	{At(ConfigId::MuteSfx), {
		"dsda_mute_sfx", ConfigId::MuteSfx,
		CONF_BOOL(0), nullptr, NOT_STRICT, S_ResetSfxVolume
	}},
	{At(ConfigId::MuteMusic), {
		"dsda_mute_music", ConfigId::MuteMusic,
		CONF_BOOL(0), nullptr, NOT_STRICT, I_ResetMusicVolume
	}},
	{At(ConfigId::MuteUnfocusedWindow), {
		"dsda_mute_unfocused_window", ConfigId::MuteUnfocusedWindow,
		CONF_BOOL(0), nullptr, NOT_STRICT, S_ResetVolume
	}},
	{At(ConfigId::CheatCodes), {
		"dsda_cheat_codes", ConfigId::CheatCodes,
		CONF_BOOL(1), nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::OrganizeFailedDemos), {
		"dsda_organize_failed_demos", ConfigId::OrganizeFailedDemos,
		CONF_BOOL(0)
	}},
	{At(ConfigId::Script0), {
		"dsda_script_0", ConfigId::Script0,
		CONF_STRING("")
	}},
	{At(ConfigId::Script1), {
		"dsda_script_1", ConfigId::Script1,
		CONF_STRING("")
	}},
	{At(ConfigId::Script2), {
		"dsda_script_2", ConfigId::Script2,
		CONF_STRING("")
	}},
	{At(ConfigId::Script3), {
		"dsda_script_3", ConfigId::Script3,
		CONF_STRING("")
	}},
	{At(ConfigId::Script4), {
		"dsda_script_4", ConfigId::Script4,
		CONF_STRING("")
	}},
	{At(ConfigId::Script5), {
		"dsda_script_5", ConfigId::Script5,
		CONF_STRING("")
	}},
	{At(ConfigId::Script6), {
		"dsda_script_6", ConfigId::Script6,
		CONF_STRING("")
	}},
	{At(ConfigId::Script7), {
		"dsda_script_7", ConfigId::Script7,
		CONF_STRING("")
	}},
	{At(ConfigId::Script8), {
		"dsda_script_8", ConfigId::Script8,
		CONF_STRING("")
	}},
	{At(ConfigId::Script9), {
		"dsda_script_9", ConfigId::Script9,
		CONF_STRING("")
	}},
	{At(ConfigId::OverrunSpechitWarn), {
		"overrun_spechit_warn", ConfigId::OverrunSpechitWarn,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Spechit)].warn
	}},
	{At(ConfigId::OverrunSpechitEmulate), {
		"overrun_spechit_emulate", ConfigId::OverrunSpechitEmulate,
		CONF_BOOL(1), &overflows[std::to_underlying(OverrunList::Spechit)].emulate
	}},
	{At(ConfigId::OverrunRejectWarn), {
		"overrun_reject_warn", ConfigId::OverrunRejectWarn,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Reject)].warn
	}},
	{At(ConfigId::OverrunRejectEmulate), {
		"overrun_reject_emulate", ConfigId::OverrunRejectEmulate,
		CONF_BOOL(1), &overflows[std::to_underlying(OverrunList::Reject)].emulate
	}},
	{At(ConfigId::OverrunInterceptWarn), {
		"overrun_intercept_warn", ConfigId::OverrunInterceptWarn,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Intercept)].warn
	}},
	{At(ConfigId::OverrunInterceptEmulate), {
		"overrun_intercept_emulate", ConfigId::OverrunInterceptEmulate,
		CONF_BOOL(1), &overflows[std::to_underlying(OverrunList::Intercept)].emulate
	}},
	{At(ConfigId::OverrunPlayeringameWarn), {
		"overrun_playeringame_warn", ConfigId::OverrunPlayeringameWarn,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Playeringame)].warn
	}},
	{At(ConfigId::OverrunPlayeringameEmulate), {
		"overrun_playeringame_emulate", ConfigId::OverrunPlayeringameEmulate,
		CONF_BOOL(1), &overflows[std::to_underlying(OverrunList::Playeringame)].emulate
	}},
	{At(ConfigId::OverrunDonutWarn), {
		"overrun_donut_warn", ConfigId::OverrunDonutWarn,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Donut)].warn
	}},
	{At(ConfigId::OverrunDonutEmulate), {
		"overrun_donut_emulate", ConfigId::OverrunDonutEmulate,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Donut)].emulate
	}},
	{At(ConfigId::OverrunMissedbacksideWarn), {
		"overrun_missedbackside_warn", ConfigId::OverrunMissedbacksideWarn,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Missedbackside)].warn
	}},
	{At(ConfigId::OverrunMissedbacksideEmulate), {
		"overrun_missedbackside_emulate", ConfigId::OverrunMissedbacksideEmulate,
		CONF_BOOL(0), &overflows[std::to_underlying(OverrunList::Missedbackside)].emulate
	}},
	{At(ConfigId::ComperrPassuse), {
		"comperr_passuse", ConfigId::ComperrPassuse,
		CONF_BOOL(0), &default_comperr[std::to_underlying(CompError::PassUse)]
	}},
	{At(ConfigId::ComperrHangsolid), {
		"comperr_hangsolid", ConfigId::ComperrHangsolid,
		CONF_BOOL(0), &default_comperr[std::to_underlying(CompError::HangSolid)]
	}},
	{At(ConfigId::ComperrBlockmap), {
		"comperr_blockmap", ConfigId::ComperrBlockmap,
		CONF_BOOL(0), &default_comperr[std::to_underlying(CompError::BlockMap)]
	}},
	{At(ConfigId::MapcolorBack), {
		"mapcolor_back", ConfigId::MapcolorBack,
		CONF_COLOR(247), &mapcolor.back
	}},
	{At(ConfigId::MapcolorGrid), {
		"mapcolor_grid", ConfigId::MapcolorGrid,
		CONF_COLOR(104), &mapcolor.grid
	}},
	{At(ConfigId::MapcolorWall), {
		"mapcolor_wall", ConfigId::MapcolorWall,
		CONF_COLOR(23), &mapcolor.wall
	}},
	{At(ConfigId::MapcolorFchg), {
		"mapcolor_fchg", ConfigId::MapcolorFchg,
		CONF_COLOR(55), &mapcolor.fchg
	}},
	{At(ConfigId::MapcolorCchg), {
		"mapcolor_cchg", ConfigId::MapcolorCchg,
		CONF_COLOR(215), &mapcolor.cchg
	}},
	{At(ConfigId::MapcolorClsd), {
		"mapcolor_clsd", ConfigId::MapcolorClsd,
		CONF_COLOR(208), &mapcolor.clsd
	}},
	{At(ConfigId::MapcolorRkey), {
		"mapcolor_rkey", ConfigId::MapcolorRkey,
		CONF_COLOR(175), &mapcolor.rkey
	}},
	{At(ConfigId::MapcolorBkey), {
		"mapcolor_bkey", ConfigId::MapcolorBkey,
		CONF_COLOR(204), &mapcolor.bkey
	}},
	{At(ConfigId::MapcolorYkey), {
		"mapcolor_ykey", ConfigId::MapcolorYkey,
		CONF_COLOR(231), &mapcolor.ykey
	}},
	{At(ConfigId::MapcolorRdor), {
		"mapcolor_rdor", ConfigId::MapcolorRdor,
		CONF_COLOR(175), &mapcolor.rdor
	}},
	{At(ConfigId::MapcolorBdor), {
		"mapcolor_bdor", ConfigId::MapcolorBdor,
		CONF_COLOR(204), &mapcolor.bdor
	}},
	{At(ConfigId::MapcolorYdor), {
		"mapcolor_ydor", ConfigId::MapcolorYdor,
		CONF_COLOR(231), &mapcolor.ydor
	}},
	{At(ConfigId::MapcolorTele), {
		"mapcolor_tele", ConfigId::MapcolorTele,
		CONF_COLOR(119), &mapcolor.tele
	}},
	{At(ConfigId::MapcolorSecr), {
		"mapcolor_secr", ConfigId::MapcolorSecr,
		CONF_COLOR(252), &mapcolor.secr
	}},
	{At(ConfigId::MapcolorRevsecr), {
		"mapcolor_revsecr", ConfigId::MapcolorRevsecr,
		CONF_COLOR(112), &mapcolor.revsecr
	}},
	{At(ConfigId::MapcolorTagfinder), {
		"mapcolor_tagfinder", ConfigId::MapcolorTagfinder,
		CONF_COLOR(252), &mapcolor.tagfinder
	}},
	{At(ConfigId::MapcolorExit), {
		"mapcolor_exit", ConfigId::MapcolorExit,
		CONF_COLOR(0), &mapcolor.exit
	}},
	{At(ConfigId::MapcolorExitsecr), {
		"mapcolor_exitsecr", ConfigId::MapcolorExitsecr,
		CONF_COLOR(0), &mapcolor.exitsecr
	}},
	{At(ConfigId::MapcolorUnsn), {
		"mapcolor_unsn", ConfigId::MapcolorUnsn,
		CONF_COLOR(104), &mapcolor.unsn
	}},
	{At(ConfigId::MapcolorFlat), {
		"mapcolor_flat", ConfigId::MapcolorFlat,
		CONF_COLOR(88), &mapcolor.flat
	}},
	{At(ConfigId::MapcolorSprt), {
		"mapcolor_sprt", ConfigId::MapcolorSprt,
		CONF_COLOR(88), &mapcolor.sprt
	}},
	{At(ConfigId::MapcolorItem), {
		"mapcolor_item", ConfigId::MapcolorItem,
		CONF_COLOR(231), &mapcolor.item
	}},
	{At(ConfigId::MapcolorHair), {
		"mapcolor_hair", ConfigId::MapcolorHair,
		CONF_COLOR(208), &mapcolor.hair
	}},
	{At(ConfigId::MapcolorSngl), {
		"mapcolor_sngl", ConfigId::MapcolorSngl,
		CONF_COLOR(208), &mapcolor.sngl
	}},
	{At(ConfigId::MapcolorMe), {
		"mapcolor_me", ConfigId::MapcolorMe,
		CONF_COLOR(112), &mapcolor.me
	}},
	{At(ConfigId::MapcolorEnemy), {
		"mapcolor_enemy", ConfigId::MapcolorEnemy,
		CONF_COLOR(177), &mapcolor.enemy
	}},
	{At(ConfigId::MapcolorFrnd), {
		"mapcolor_frnd", ConfigId::MapcolorFrnd,
		CONF_COLOR(112), &mapcolor.frnd
	}},
	{At(ConfigId::MapcolorTrail1), {
		"mapcolor_trail_1", ConfigId::MapcolorTrail1,
		CONF_COLOR(80), &mapcolor.trail_1
	}},
	{At(ConfigId::MapcolorTrail2), {
		"mapcolor_trail_2", ConfigId::MapcolorTrail2,
		CONF_COLOR(100), &mapcolor.trail_2
	}},
	{At(ConfigId::MapcolorPickup), {
		"mapcolor_pickup", ConfigId::MapcolorPickup,
		CONF_COLOR(112), &mapcolor.pickup
	}},
	{At(ConfigId::GlSkymode), {
		"gl_skymode", ConfigId::GlSkymode,
		ConfigType::Int, std::to_underlying(SkyType::Auto), std::to_underlying(SkyType::Count) - 1, {std::to_underlying(SkyType::Auto)}, nullptr,
		NOT_STRICT, M_ChangeSkyMode
	}},
	{At(ConfigId::GlRenderMultisampling), {
		"gl_render_multisampling", ConfigId::GlRenderMultisampling,
		ConfigType::Int, 0, 8, {0}, nullptr, ConfigFlag::Even, 0, 0, gld_MultisamplingInit
	}},
	{At(ConfigId::GlRenderFov), {
		"gl_render_fov", ConfigId::GlRenderFov,
		ConfigType::Int, 20, 160, {90}, &gl_render_fov, NOT_STRICT, M_ChangeFOV
	}},
	{At(ConfigId::GlHealthBar), {
		"gl_health_bar", ConfigId::GlHealthBar,
		CONF_BOOL(0), nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::GlUsevbo), {
		"gl_usevbo", ConfigId::GlUsevbo,
		CONF_BOOL(1), nullptr, NOT_STRICT
	}},
	{At(ConfigId::UseMouse), {
		"use_mouse", ConfigId::UseMouse,
		CONF_BOOL(1), nullptr, NOT_STRICT, I_InitMouse
	}},
	{At(ConfigId::MouseSensitivityHoriz), {
		"mouse_sensitivity_horiz", ConfigId::MouseSensitivityHoriz,
		ConfigType::Int, 0, INT_MAX, {10}, nullptr, NOT_STRICT, G_UpdateMouseSensitivity
	}},
	{At(ConfigId::MouseSensitivityVert), {
		"mouse_sensitivity_vert", ConfigId::MouseSensitivityVert,
		ConfigType::Int, 0, INT_MAX, {1}, nullptr, NOT_STRICT, G_UpdateMouseSensitivity
	}},
	{At(ConfigId::MouseAcceleration), {
		"dsda_mouse_acceleration", ConfigId::MouseAcceleration,
		ConfigType::Int, 0, INT_MAX, {0}, nullptr, NOT_STRICT, AccelChanging
	}},
	{At(ConfigId::MouseSensitivityMlook), {
		"mouse_sensitivity_mlook", ConfigId::MouseSensitivityMlook,
		ConfigType::Int, 0, INT_MAX, {10}, nullptr, NOT_STRICT, G_UpdateMouseSensitivity
	}},
	{At(ConfigId::MouseStutterCorrection), {
		"mouse_stutter_correction", ConfigId::MouseStutterCorrection,
		CONF_BOOL(1)
	}},
	{At(ConfigId::MouseDoubleclickAsUse), {
		"mouse_doubleclick_as_use", ConfigId::MouseDoubleclickAsUse,
		CONF_BOOL(0)
	}},
	{At(ConfigId::MouseCarrytics), {
		"mouse_carrytics", ConfigId::MouseCarrytics,
		CONF_BOOL(1)
	}},
	{At(ConfigId::MovementMouseinvert), {
		"movement_mouseinvert", ConfigId::MovementMouseinvert,
		CONF_BOOL(0)
	}},
	{At(ConfigId::MovementMousestrafedivisor), {
		"movement_mousestrafedivisor", ConfigId::MovementMousestrafedivisor,
		ConfigType::Int, 1, INT_MAX, {4}, nullptr, NOT_STRICT, G_UpdateMouseSensitivity
	}},
	{At(ConfigId::FineSensitivity), {
		"dsda_fine_sensitivity", ConfigId::FineSensitivity,
		ConfigType::Int, 0, 99, {0}, nullptr, NOT_STRICT, G_UpdateMouseSensitivity
	}},
	{At(ConfigId::UseGameController), {
		"use_game_controller", ConfigId::UseGameController,
		ConfigType::Int, 0, 2, {0}, nullptr, NOT_STRICT, dsda_InitGameController
	}},
	{At(ConfigId::DehApplyCheats), {
		"deh_apply_cheats", ConfigId::DehApplyCheats,
		CONF_BOOL(1)
	}},
	{At(ConfigId::MovementStrafe50), {
		"movement_strafe50", ConfigId::MovementStrafe50,
		CONF_BOOL(0), nullptr, STRICT_INT(0), M_ChangeSpeed
	}},
	{At(ConfigId::MovementStrafe50onturns), {
		"movement_strafe50onturns", ConfigId::MovementStrafe50onturns,
		CONF_BOOL(0), nullptr, STRICT_INT(0), M_ChangeSpeed
	}},
	{At(ConfigId::MovementShorttics), {
		"movement_shorttics", ConfigId::MovementShorttics,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeShorttics
	}},
	{At(ConfigId::ScreenshotDir), {
		"screenshot_dir", ConfigId::ScreenshotDir,
		CONF_STRING("")
	}},
	{At(ConfigId::StartupDelayMs), {
		"startup_delay_ms", ConfigId::StartupDelayMs,
		ConfigType::Int, 0, 1000, {0}
	}},
	{At(ConfigId::PitchedSounds), {
		"pitched_sounds", ConfigId::PitchedSounds,
		CONF_BOOL(0), nullptr, NOT_STRICT, I_InitSoundParams
	}},
	{At(ConfigId::FullSounds), {
		"full_sounds", ConfigId::FullSounds,
		CONF_BOOL(0), &full_sounds
	}},
	{At(ConfigId::SndSamplerate), {
		"snd_samplerate", ConfigId::SndSamplerate,
		ConfigType::Int, 11025, 48000, {44100}, nullptr, NOT_STRICT, I_InitSoundParams
	}},
	{At(ConfigId::SndSamplecount), {
		"snd_samplecount", ConfigId::SndSamplecount,
		ConfigType::Int, 0, 8192, {0}, nullptr, NOT_STRICT, I_InitSoundParams
	}},
	{At(ConfigId::SfxVolume), {
		"sfx_volume", ConfigId::SfxVolume,
		ConfigType::Int, 0, 15, {8}, nullptr, NOT_STRICT, S_ResetSfxVolume
	}},
	{At(ConfigId::MusicVolume), {
		"music_volume", ConfigId::MusicVolume,
		ConfigType::Int, 0, 15, {8}, nullptr, NOT_STRICT, I_ResetMusicVolume
	}},
	{At(ConfigId::MusPauseOpt), {
		"mus_pause_opt", ConfigId::MusPauseOpt,
		ConfigType::Int, 0, 2, {1}
	}},
	{At(ConfigId::SndChannels), {
		"snd_channels", ConfigId::SndChannels,
		ConfigType::Int, 1, MAX_CHANNELS, {32}, nullptr, NOT_STRICT, S_Init
	}},
	{At(ConfigId::SndMidiplayer), {
		"snd_midiplayer", ConfigId::SndMidiplayer,
		CONF_STRING("fluidsynth"), nullptr, NOT_STRICT, M_ChangeMIDIPlayer
	}},
	{At(ConfigId::SndMididev), {
		"snd_mididev", ConfigId::SndMididev,
		CONF_STRING("")
	}},
	{At(ConfigId::SndSoundfont), {
		"snd_soundfont", ConfigId::SndSoundfont,
		CONF_STRING("")
	}},
	{At(ConfigId::MusFluidsynthChorus), {
		"mus_fluidsynth_chorus", ConfigId::MusFluidsynthChorus,
		CONF_BOOL(0)
	}},
	{At(ConfigId::MusFluidsynthReverb), {
		"mus_fluidsynth_reverb", ConfigId::MusFluidsynthReverb,
		CONF_BOOL(0)
	}},
	{At(ConfigId::MusFluidsynthGain), {
		"mus_fluidsynth_gain", ConfigId::MusFluidsynthGain,
		ConfigType::Int, 0, 1000, {50}
	}},
	{At(ConfigId::MusFluidsynthChorusDepth), {
		"mus_fluidsynth_chorus_depth", ConfigId::MusFluidsynthChorusDepth,
		ConfigType::Int, 0, 10000, {500}
	}},
	{At(ConfigId::MusFluidsynthChorusLevel), {
		"mus_fluidsynth_chorus_level", ConfigId::MusFluidsynthChorusLevel,
		ConfigType::Int, 0, 1000, {35}
	}},
	{At(ConfigId::MusFluidsynthReverbDamp), {
		"mus_fluidsynth_reverb_damp", ConfigId::MusFluidsynthReverbDamp,
		ConfigType::Int, 0, 1000, {40}
	}},
	{At(ConfigId::MusFluidsynthReverbLevel), {
		"mus_fluidsynth_reverb_level", ConfigId::MusFluidsynthReverbLevel,
		ConfigType::Int, 0, 1000, {15}
	}},
	{At(ConfigId::MusFluidsynthReverbWidth), {
		"mus_fluidsynth_reverb_width", ConfigId::MusFluidsynthReverbWidth,
		ConfigType::Int, 0, 10000, {400}
	}},
	{At(ConfigId::MusFluidsynthReverbRoomSize), {
		"mus_fluidsynth_reverb_room_size", ConfigId::MusFluidsynthReverbRoomSize,
		ConfigType::Int, 0, 1000, {60}
	}},
	{At(ConfigId::MusOplGain), {
		"mus_opl_gain", ConfigId::MusOplGain,
		ConfigType::Int, 0, 1000, {50}
	}},
	{At(ConfigId::MusOplOpl3mode), {
		"mus_opl_opl3mode", ConfigId::MusOplOpl3mode,
		CONF_BOOL(1)
	}},
	{At(ConfigId::MusPortmidiResetType), {
		"mus_portmidi_reset_type", ConfigId::MusPortmidiResetType,
		CONF_STRING("gm") // none, gs, gm, gm2, xg
	}},
	{At(ConfigId::MusPortmidiResetDelay), {
		"mus_portmidi_reset_delay", ConfigId::MusPortmidiResetDelay,
		ConfigType::Int, 0, 2000, {0}
	}},
	{At(ConfigId::MusPortmidiFilterSysex), {
		"mus_portmidi_filter_sysex", ConfigId::MusPortmidiFilterSysex,
		ConfigType::Int, 0, 1, {1}
	}},
	{At(ConfigId::MusPortmidiReverbLevel), {
		"mus_portmidi_reverb_level", ConfigId::MusPortmidiReverbLevel,
		ConfigType::Int, -1, 127, {-1}
	}},
	{At(ConfigId::MusPortmidiChorusLevel), {
		"mus_portmidi_chorus_level", ConfigId::MusPortmidiChorusLevel,
		ConfigType::Int, -1, 127, {-1}
	}},
	{At(ConfigId::CapSoundcommand), {
		"cap_soundcommand", ConfigId::CapSoundcommand,
		CONF_STRING("ffmpeg -f s16le -ar %s -ac 2 -i - -c:a libopus -y temp_a.nut")
	}},
	{At(ConfigId::CapVideocommand), {
		"cap_videocommand", ConfigId::CapVideocommand,
		CONF_STRING("ffmpeg -f rawvideo -pix_fmt rgb24 -r %r -s %wx%h -i - -c:v libx264 -y temp_v.nut")
	}},
	{At(ConfigId::CapMuxcommand), {
		"cap_muxcommand", ConfigId::CapMuxcommand,
		CONF_STRING("ffmpeg -i temp_v.nut -i temp_a.nut -r %r -c copy -y %f")
	}},
	{At(ConfigId::CapTempfile1), {
		"cap_tempfile1", ConfigId::CapTempfile1,
		CONF_STRING("temp_a.nut")
	}},
	{At(ConfigId::CapTempfile2), {
		"cap_tempfile2", ConfigId::CapTempfile2,
		CONF_STRING("temp_v.nut")
	}},
	{At(ConfigId::CapRemoveTempfiles), {
		"cap_remove_tempfiles", ConfigId::CapRemoveTempfiles,
		CONF_BOOL(1)
	}},
	{At(ConfigId::CapWipescreen), {
		"cap_wipescreen", ConfigId::CapWipescreen,
		CONF_BOOL(0)
	}},
	{At(ConfigId::CapFps), {
		"cap_fps", ConfigId::CapFps,
		ConfigType::Int, 16, 300, {60}
	}},
	{At(ConfigId::HudaddCrosshairColor), {
		"hudadd_crosshair_color", ConfigId::HudaddCrosshairColor,
		CONF_CR(3)
	}},
	{At(ConfigId::HudaddCrosshairTargetColor), {
		"hudadd_crosshair_target_color", ConfigId::HudaddCrosshairTargetColor,
		CONF_CR(9), nullptr, STRICT_INT(9)
	}},
	{At(ConfigId::HudDisplayed), {
		"hud_displayed", ConfigId::HudDisplayed,
		CONF_BOOL(0), nullptr, ConfigFlag::Feature | NOT_STRICT, R_SetViewSize
	}},
	{At(ConfigId::HudaddSecretarea), {
		"hudadd_secretarea", ConfigId::HudaddSecretarea,
		CONF_BOOL(1)
	}},
	{At(ConfigId::HudaddDemoprogressbar), {
		"hudadd_demoprogressbar", ConfigId::HudaddDemoprogressbar,
		CONF_BOOL(1)
	}},
	{At(ConfigId::HudaddCrosshairScale), {
		"hudadd_crosshair_scale", ConfigId::HudaddCrosshairScale,
		CONF_BOOL(0), nullptr, NOT_STRICT, HU_InitCrosshair
	}},
	{At(ConfigId::HudaddCrosshairHealth), {
		"hudadd_crosshair_health", ConfigId::HudaddCrosshairHealth,
		CONF_BOOL(0), nullptr, NOT_STRICT, HU_InitCrosshair
	}},
	{At(ConfigId::HudaddCrosshairTarget), {
		"hudadd_crosshair_target", ConfigId::HudaddCrosshairTarget,
		CONF_BOOL(0), nullptr, STRICT_INT(0), HU_InitCrosshair
	}},
	{At(ConfigId::HudaddCrosshairLockTarget), {
		"hudadd_crosshair_lock_target", ConfigId::HudaddCrosshairLockTarget,
		CONF_BOOL(0), nullptr, STRICT_INT(0), HU_InitCrosshair
	}},
	{At(ConfigId::HudaddCrosshair), {
		"hudadd_crosshair", ConfigId::HudaddCrosshair,
		ConfigType::Int, 0, HU_CROSSHAIRS - 1, {0}, nullptr, ConfigFlag::Feature | NOT_STRICT, HU_InitCrosshair
	}},
	{At(ConfigId::HudHealthRed), {
		"hud_health_red", ConfigId::HudHealthRed,
		ConfigType::Int, 0, 200, {25}, nullptr, NOT_STRICT, HU_InitThresholds
	}},
	{At(ConfigId::HudHealthYellow), {
		"hud_health_yellow", ConfigId::HudHealthYellow,
		ConfigType::Int, 0, 200, {50}, nullptr, NOT_STRICT, HU_InitThresholds
	}},
	{At(ConfigId::HudHealthGreen), {
		"hud_health_green", ConfigId::HudHealthGreen,
		ConfigType::Int, 0, 200, {100}, nullptr, NOT_STRICT, HU_InitThresholds
	}},
	{At(ConfigId::HudAmmoRed), {
		"hud_ammo_red", ConfigId::HudAmmoRed,
		ConfigType::Int, 0, 100, {25}, nullptr, NOT_STRICT, HU_InitThresholds
	}},
	{At(ConfigId::HudAmmoYellow), {
		"hud_ammo_yellow", ConfigId::HudAmmoYellow,
		ConfigType::Int, 0, 100, {50}, nullptr, NOT_STRICT, HU_InitThresholds
	}},
	{At(ConfigId::CycleGhostColors), {
		"dsda_cycle_ghost_colors", ConfigId::CycleGhostColors,
		CONF_BOOL(0)
	}},
	{At(ConfigId::AutoKeyFrameInterval), {
		"dsda_auto_key_frame_interval", ConfigId::AutoKeyFrameInterval,
		ConfigType::Int, 1, 600, {1}, nullptr, STRICT_INT(1), dsda_InitAutoKeyFrames
	}},
	{At(ConfigId::AutoKeyFrameDepth), {
		"dsda_auto_key_frame_depth", ConfigId::AutoKeyFrameDepth,
		ConfigType::Int, 0, 600, {60}, nullptr, STRICT_INT(0), dsda_InitAutoKeyFrames
	}},
	{At(ConfigId::AutoKeyFrameTimeout), {
		"dsda_auto_key_frame_timeout", ConfigId::AutoKeyFrameTimeout,
		ConfigType::Int, 0, 25, {10}, nullptr, STRICT_INT(0), dsda_InitAutoKeyFrames
	}},
	{At(ConfigId::AutoSave), {
		"dsda_config_auto_save", ConfigId::AutoSave,
		CONF_BOOL(0), nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::ExTextScaleX), {
		"ex_text_scale_x", ConfigId::ExTextScaleX,
		ConfigType::Int, 0, 4000, {0}, nullptr, NOT_STRICT, dsda_UpdateStretchParams
	}},
	{At(ConfigId::ExTextRatioY), {
		"ex_text_ratio_y", ConfigId::ExTextRatioY,
		ConfigType::Int, 0, 200, {0}, nullptr, NOT_STRICT, dsda_UpdateStretchParams
	}},
	{At(ConfigId::WipeAtFullSpeed), {
		"dsda_wipe_at_full_speed", ConfigId::WipeAtFullSpeed,
		CONF_BOOL(1), nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::ShowDemoAttempts), {
		"dsda_show_demo_attempts", ConfigId::ShowDemoAttempts,
		CONF_BOOL(1)
	}},
	{At(ConfigId::HideHorns), {
		"dsda_hide_horns", ConfigId::HideHorns,
		CONF_BOOL(0)
	}},
	{At(ConfigId::HideWeapon), {
		"dsda_hide_weapon", ConfigId::HideWeapon,
		CONF_BOOL(0), nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::OrganizedSaves), {
		"dsda_organized_saves", ConfigId::OrganizedSaves,
		CONF_BOOL(1)
	}},
	{At(ConfigId::CommandHistorySize), {
		"dsda_command_history_size", ConfigId::CommandHistorySize,
		ConfigType::Int, 1, 20, {10}, nullptr, STRICT_INT(0), dsda_InitCommandHistory
	}},
	{At(ConfigId::HideEmptyCommands), {
		"dsda_hide_empty_commands", ConfigId::HideEmptyCommands,
		CONF_BOOL(1), nullptr, STRICT_INT(0), dsda_InitCommandHistory
	}},
	{At(ConfigId::SkipQuitPrompt), {
		"dsda_skip_quit_prompt", ConfigId::SkipQuitPrompt,
		CONF_BOOL(0)
	}},
	{At(ConfigId::ShowSplitData), {
		"dsda_show_split_data", ConfigId::ShowSplitData,
		CONF_BOOL(1)
	}},
	{At(ConfigId::PlayerName), {
		"dsda_player_name", ConfigId::PlayerName,
		CONF_STRING("Anonymous")
	}},
	{At(ConfigId::QuickstartCacheTics), {
		"dsda_quickstart_cache_tics", ConfigId::QuickstartCacheTics,
		ConfigType::Int, 0, 35, {0}, nullptr, NOT_STRICT, dsda_InitQuickstartCache
	}},
	{At(ConfigId::DeathUseAction), {
		"dsda_death_use_action", ConfigId::DeathUseAction,
		ConfigType::Int, 0, 2, {0}
	}},
	{At(ConfigId::AllowJumping), {
		"dsda_allow_jumping", ConfigId::AllowJumping,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_ResetAirControl
	}},
	{At(ConfigId::PistolStart), {
		"dsda_pistol_start", ConfigId::PistolStart,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_RefreshPistolStart
	}},
	{At(ConfigId::AlwaysPistolStart), {
		"dsda_always_pistol_start", ConfigId::AlwaysPistolStart,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_RefreshAlwaysPistolStart
	}},
	{At(ConfigId::RespawnMonsters), {
		"dsda_respawn_monsters", ConfigId::RespawnMonsters,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_AlterGameFlags
	}},
	{At(ConfigId::FastMonsters), {
		"dsda_fast_monsters", ConfigId::FastMonsters,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_AlterGameFlags
	}},
	{At(ConfigId::NoMonsters), {
		"dsda_no_monsters", ConfigId::NoMonsters,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_AlterGameFlags
	}},
	{At(ConfigId::CoopSpawns), {
		"dsda_coop_spawns", ConfigId::CoopSpawns,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_AlterGameFlags
	}},
	{At(ConfigId::ParallelSfxLimit), {
		"dsda_parallel_sfx_limit", ConfigId::ParallelSfxLimit,
		ConfigType::Int, 0, 32, {0}, nullptr, NOT_STRICT, dsda_InitParallelSFXFilter
	}},
	{At(ConfigId::ParallelSfxWindow), {
		"dsda_parallel_sfx_window", ConfigId::ParallelSfxWindow,
		ConfigType::Int, 1, 32, {1}, nullptr, NOT_STRICT, dsda_InitParallelSFXFilter
	}},
	{At(ConfigId::MovementToggleSfx), {
		"dsda_movement_toggle_sfx", ConfigId::MovementToggleSfx,
		CONF_BOOL(0)
	}},
	{At(ConfigId::SwitchWhenAmmoRunsOut), {
		"dsda_switch_when_ammo_runs_out", ConfigId::SwitchWhenAmmoRunsOut,
		CONF_BOOL(1)
	}},
	{At(ConfigId::SwitchWeaponOnPickup), {
		"dsda_switch_weapon_on_pickup", ConfigId::SwitchWeaponOnPickup,
		CONF_BOOL(1), nullptr, STRICT_INT(1)
	}},
	{At(ConfigId::Viewbob), {
		"dsda_viewbob_pct", ConfigId::Viewbob,
		ConfigType::Int, 0, 4, {4}
	}},
	{At(ConfigId::Weaponbob), {
		"dsda_weaponbob_pct", ConfigId::Weaponbob,
		ConfigType::Int, 0, 4, {4}
	}},
	{At(ConfigId::FixViewbobFloorJolt), {
		"dsda_fix_viewbob_floor_jolt", ConfigId::FixViewbobFloorJolt,
		CONF_BOOL(1)
	}},
	{At(ConfigId::QuakeIntensity), {
		"dsda_quake_intensity", ConfigId::QuakeIntensity,
		ConfigType::Int, 0, 100, {100}
	}},
	{At(ConfigId::DemoEndQuit), {
		"dsda_demo_end_quit", ConfigId::DemoEndQuit,
		CONF_BOOL(1)
	}},
	{At(ConfigId::MapBlinkingLocks), {
		"map_blinking_locks", ConfigId::MapBlinkingLocks,
		CONF_BOOL(0), nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::MapSecretAfter), {
		"map_secret_after", ConfigId::MapSecretAfter,
		CONF_BOOL(0), nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::MapCoordinates), {
		"map_coordinates", ConfigId::MapCoordinates,
		CONF_BOOL(1), nullptr, STRICT_INT(0), dsda_RefreshMapCoordinates
	}},
	{At(ConfigId::MapTotals), {
		"map_totals", ConfigId::MapTotals,
		CONF_BOOL(1), nullptr, NOT_STRICT, dsda_RefreshMapTotals
	}},
	{At(ConfigId::MapTime), {
		"map_time", ConfigId::MapTime,
		CONF_BOOL(1), nullptr, NOT_STRICT, dsda_RefreshMapTime
	}},
	{At(ConfigId::MapTitle), {
		"map_title", ConfigId::MapTitle,
		CONF_BOOL(1), nullptr, NOT_STRICT, dsda_RefreshMapTitle
	}},
	{At(ConfigId::MapTrail), {
		"map_trail", ConfigId::MapTrail,
		CONF_BOOL(0), nullptr, STRICT_INT(0), AM_initPlayerTrail
	}},
	{At(ConfigId::MapTrailCollisions), {
		"map_trail_collisions", ConfigId::MapTrailCollisions,
		CONF_BOOL(0), nullptr, STRICT_INT(0), AM_initPlayerTrail
	}},
	{At(ConfigId::MapTrailSize), {
		"map_trail_size", ConfigId::MapTrailSize,
		ConfigType::Int, 0, 350, {105}, nullptr, STRICT_INT(0), AM_initPlayerTrail
	}},
	{At(ConfigId::MapTraces), {
		"map_traces", ConfigId::MapTraces,
		CONF_BOOL(0)
	}},
	{At(ConfigId::AutomapOverlay), {
		"automap_overlay", ConfigId::AutomapOverlay,
		ConfigType::Int, 0, 2, {0}, &automap_overlay
	}},
	{At(ConfigId::AutomapRotate), {
		"automap_rotate", ConfigId::AutomapRotate,
		CONF_BOOL(0), &automap_rotate
	}},
	{At(ConfigId::AutomapFollow), {
		"automap_follow", ConfigId::AutomapFollow,
		CONF_BOOL(1), &automap_follow
	}},
	{At(ConfigId::AutomapGrid), {
		"automap_grid", ConfigId::AutomapGrid,
		CONF_BOOL(0), &automap_grid
	}},
	{At(ConfigId::MapGridSize), {
		"map_grid_size", ConfigId::MapGridSize,
		ConfigType::Int, 8, 256, {128}, nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::MapPanSpeed), {
		"map_pan_speed", ConfigId::MapPanSpeed,
		ConfigType::Int, 1, 32, {16}, nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::MapScrollSpeed), {
		"map_scroll_speed", ConfigId::MapScrollSpeed,
		ConfigType::Int, 1, 32, {32}, nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::MapWheelZoom), {
		"map_wheel_zoom", ConfigId::MapWheelZoom,
		CONF_BOOL(1), nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::MapUseMultisamling), {
		"map_use_multisampling", ConfigId::MapUseMultisamling,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeMapMultisamling
	}},
	{At(ConfigId::MapTextured), {
		"map_textured", ConfigId::MapTextured,
		CONF_BOOL(1), nullptr, STRICT_INT(0), M_ChangeMapTextured
	}},
	{At(ConfigId::MapTexturedTrans), {
		"map_textured_trans", ConfigId::MapTexturedTrans,
		ConfigType::Int, 0, 100, {100}, nullptr, STRICT_INT(0), gld_ResetAutomapTransparency
	}},
	{At(ConfigId::MapTexturedOverlayTrans), {
		"map_textured_overlay_trans", ConfigId::MapTexturedOverlayTrans,
		ConfigType::Int, 0, 100, {66}, nullptr, STRICT_INT(0), gld_ResetAutomapTransparency
	}},
	{At(ConfigId::MapLinesOverlayTrans), {
		"map_lines_overlay_trans", ConfigId::MapLinesOverlayTrans,
		ConfigType::Int, 0, 100, {100}, nullptr, NOT_STRICT, gld_ResetAutomapTransparency
	}},
	{At(ConfigId::MapThingsAppearance), {
		"map_things_appearance", ConfigId::MapThingsAppearance,
		ConfigType::Int, 0, std::to_underlying(MapThingsAppearance::Count) - 1, {std::to_underlying(MapThingsAppearance::Count) - 1},
		nullptr, NOT_STRICT, AM_InitParams
	}},
	{At(ConfigId::Videomode), {
		"videomode", ConfigId::Videomode,
		CONF_STRING("Software"), nullptr, NOT_STRICT, M_ChangeVideoMode
	}},
	{At(ConfigId::ScreenResolution), {
		"screen_resolution", ConfigId::ScreenResolution,
		CONF_STRING("640x480"), nullptr, NOT_STRICT, M_ChangeVideoMode
	}},
	{At(ConfigId::CustomResolution), {
		"custom_resolution", ConfigId::CustomResolution,
		CONF_STRING("")
	}},
	{At(ConfigId::UseFullscreen), {
		"use_fullscreen", ConfigId::UseFullscreen,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeFullScreen
	}},
	{At(ConfigId::ExclusiveFullscreen), {
		"exclusive_fullscreen", ConfigId::ExclusiveFullscreen,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeVideoMode
	}},
	{At(ConfigId::RenderVsync), {
		"render_vsync", ConfigId::RenderVsync,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeVideoMode
	}},
	{At(ConfigId::UncappedFramerate), {
		"uncapped_framerate", ConfigId::UncappedFramerate,
		CONF_BOOL(1), nullptr, NOT_STRICT, M_ChangeUncappedFrameRate
	}},
	{At(ConfigId::FpsLimit), {
		"dsda_fps_limit", ConfigId::FpsLimit,
		ConfigType::Int, 0, 1000, {0}
	}},
	{At(ConfigId::BackgroundFpsLimit), {
		"dsda_background_fps_limit", ConfigId::BackgroundFpsLimit,
		ConfigType::Int, 0, 1000, {35}
	}},
	{At(ConfigId::Usegamma), {
		"usegamma", ConfigId::Usegamma,
		ConfigType::Int, 0, 4, {0}, &usegamma, NOT_STRICT, M_ChangeApplyPalette
	}},
	{At(ConfigId::Screenblocks), {
		"screenblocks", ConfigId::Screenblocks,
		ConfigType::Int, 10, 11, {10}, nullptr, ConfigFlag::Feature | NOT_STRICT, R_SetViewSize
	}},
	{At(ConfigId::SdlVideoWindowPos), {
		"sdl_video_window_pos", ConfigId::SdlVideoWindowPos,
		CONF_STRING("")
	}},
	{At(ConfigId::SdlVideoDisplayIndex), {
		"sdl_video_display_index", ConfigId::SdlVideoDisplayIndex,
		ConfigType::Int, 0, 10, {0}, nullptr, NOT_STRICT
	}},
	{At(ConfigId::PaletteOndamage), {
		"palette_ondamage", ConfigId::PaletteOndamage,
		CONF_BOOL(1), nullptr, STRICT_INT(1), M_ChangeApplyPalette
	}},
	{At(ConfigId::PaletteOnbonus), {
		"palette_onbonus", ConfigId::PaletteOnbonus,
		CONF_BOOL(1), nullptr, STRICT_INT(1), M_ChangeApplyPalette
	}},
	{At(ConfigId::PaletteOnpowers), {
		"palette_onpowers", ConfigId::PaletteOnpowers,
		CONF_BOOL(1), nullptr, STRICT_INT(1), M_ChangeApplyPalette
	}},
	{At(ConfigId::RenderWipescreen), {
		"render_wipescreen", ConfigId::RenderWipescreen,
		CONF_BOOL(1), nullptr, STRICT_INT(1)
	}},
	{At(ConfigId::RenderScreenMultiply), {
		"render_screen_multiply", ConfigId::RenderScreenMultiply,
		ConfigType::Int, 1, 5, {1}, nullptr, NOT_STRICT, M_ChangeVideoMode
	}},
	{At(ConfigId::IntegerScaling), {
		"integer_scaling", ConfigId::IntegerScaling,
		CONF_BOOL(0), nullptr, NOT_STRICT, M_ChangeVideoMode
	}},
	{At(ConfigId::RenderAspect), {
		"render_aspect", ConfigId::RenderAspect,
		ConfigType::Int, 0, 4, {0}, nullptr, NOT_STRICT, M_ChangeAspectRatio
	}},
	{At(ConfigId::RenderDoomLightmaps), {
		"render_doom_lightmaps", ConfigId::RenderDoomLightmaps,
		CONF_BOOL(0)
	}},
	{At(ConfigId::FakeContrastMode), {
		"fake_contrast_mode", ConfigId::FakeContrastMode,
		ConfigType::Int, std::to_underlying(FakeContrastMode::Off), std::to_underlying(FakeContrastMode::Smooth),
		{std::to_underlying(FakeContrastMode::On)}, &fake_contrast_mode
	}},
	{At(ConfigId::RenderStretchHud), {
		"render_stretch_hud", ConfigId::RenderStretchHud,
		ConfigType::Int, std::to_underlying(PatchStretch::NotAdjusted), std::to_underlying(PatchStretch::FitToWidth), {std::to_underlying(PatchStretch::DoomFormat)},
		nullptr, NOT_STRICT, M_ChangeStretch
	}},
	{At(ConfigId::RenderPatchesScalex), {
		"render_patches_scalex", ConfigId::RenderPatchesScalex,
		ConfigType::Int, 0, 16, {0}
	}},
	{At(ConfigId::RenderPatchesScaley), {
		"render_patches_scaley", ConfigId::RenderPatchesScaley,
		ConfigType::Int, 0, 16, {0}
	}},
	{At(ConfigId::RenderStretchsky), {
		"render_stretchsky", ConfigId::RenderStretchsky,
		CONF_BOOL(1)
	}},
	{At(ConfigId::RenderLinearsky), {
		"render_linearsky", ConfigId::RenderLinearsky,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_RefreshLinearSky
	}},
	{At(ConfigId::AspectRatioCorrection), {
		"aspect_ratio_correction", ConfigId::AspectRatioCorrection,
		CONF_BOOL(1), nullptr, NOT_STRICT
	}},
	{At(ConfigId::GlFadeMode), {
		"gl_fade_mode", ConfigId::GlFadeMode,
		ConfigType::Int, 0, 2, {0}
	}},
	{At(ConfigId::TranslucentSprites), {
		"boom_translucent_sprites", ConfigId::TranslucentSprites,
		ConfigType::Int, 0, 2, {1}, nullptr, STRICT_RANGE(0, 1), deh_changeCompTranslucency
	}},
	{At(ConfigId::TranslucentGhosts), {
		"translucent_ghosts", ConfigId::TranslucentGhosts,
		CONF_BOOL(0), nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::ShowAliveMonsters), {
		// never persisted
		"show_alive_monsters", ConfigId::ShowAliveMonsters,
		ConfigType::Int, 0, 2, {0}, nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::LeftAnalogDeadzone), {
		"left_analog_deadzone", ConfigId::LeftAnalogDeadzone,
		ConfigType::Int, 0, 16384, {6556}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::RightAnalogDeadzone), {
		"right_analog_deadzone", ConfigId::RightAnalogDeadzone,
		ConfigType::Int, 0, 16384, {6556}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::LeftTriggerDeadzone), {
		"left_trigger_deadzone", ConfigId::LeftTriggerDeadzone,
		ConfigType::Int, 0, 16384, {6556}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::RightTriggerDeadzone), {
		"right_trigger_deadzone", ConfigId::RightTriggerDeadzone,
		ConfigType::Int, 0, 16384, {6556}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::LeftAnalogSensitivityX), {
		"left_analog_sensitivity_x", ConfigId::LeftAnalogSensitivityX,
		ConfigType::Int, 0, 16384, {100}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::LeftAnalogSensitivityY), {
		"left_analog_sensitivity_y", ConfigId::LeftAnalogSensitivityY,
		ConfigType::Int, 0, 16384, {100}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::RightAnalogSensitivityX), {
		"right_analog_sensitivity_x", ConfigId::RightAnalogSensitivityX,
		ConfigType::Int, 0, 16384, {1536}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::RightAnalogSensitivityY), {
		"right_analog_sensitivity_y", ConfigId::RightAnalogSensitivityY,
		ConfigType::Int, 0, 16384, {768}, nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::AnalogLookAcceleration), {
		"analog_look_acceleration", ConfigId::AnalogLookAcceleration,
		ConfigType::Int, 0, INT_MAX, {0}, nullptr, NOT_STRICT, AccelChanging
	}},
	{At(ConfigId::SwapAnalogs), {
		"swap_analogs", ConfigId::SwapAnalogs,
		CONF_BOOL(0), nullptr, NOT_STRICT, dsda_InitGameControllerParameters
	}},
	{At(ConfigId::InvertAnalogLook), {
		"invert_analog_look", ConfigId::InvertAnalogLook,
		CONF_BOOL(0),
	}},
	{At(ConfigId::ShowEndoom), {
		"show_endoom", ConfigId::ShowEndoom,
		ConfigType::Int, 0, 2, {0}
	}},
	{At(ConfigId::ExportEndoom), {
		"export_endoom", ConfigId::ExportEndoom,
		CONF_BOOL(0),
	}},
	{At(ConfigId::AnsiEndoom), {
		"ansi_endoom", ConfigId::AnsiEndoom,
		ConfigType::Int, 0, 2, {0}
	}},
	{At(ConfigId::QuitSounds), {
		"quit_sounds", ConfigId::QuitSounds,
		CONF_BOOL(0),
	}},
	{At(ConfigId::AnnounceMap), {
		"announce_map", ConfigId::AnnounceMap,
		CONF_BOOL(0),
	}},
	{At(ConfigId::ExtraLevelBrightness), {
		"extra_level_brightness", ConfigId::ExtraLevelBrightness,
		ConfigType::Int, 0, 4, {0}, nullptr, STRICT_INT(0)
	}},
	{At(ConfigId::PlaybackMouseControls), {
		"playback_mouse_controls", ConfigId::PlaybackMouseControls,
		CONF_BOOL(1)
	}},
};

static void dsda_PersistIntConfig(dsda_config_t* conf)
{
	conf->persistent_value.v_int = conf->transient_value.v_int;
}

static void dsda_PersistStringConfig(dsda_config_t* conf)
{
	if(conf->persistent_value.v_string)
		Z_Free(conf->persistent_value.v_string);

	conf->persistent_value.v_string = Z_Strdup(conf->transient_value.v_string);
}

static void dsda_ConstrainIntConfig(dsda_config_t* conf)
{
	if(conf->transient_value.v_int > conf->upper_limit)
		conf->transient_value.v_int = conf->upper_limit;
	else if(conf->transient_value.v_int < conf->lower_limit)
	{
		if(conf->transient_value.v_int == -1)
			conf->transient_value.v_int = conf->default_value.v_int;
		else
			conf->transient_value.v_int = conf->lower_limit;
	}

	if((conf->flags & ConfigFlag::Even) != ConfigFlag{} && (conf->transient_value.v_int % 2))
		conf->transient_value.v_int = conf->default_value.v_int;
}

static void dsda_PropagateIntConfig(dsda_config_t* conf)
{
	if(conf->int_binding)
		*conf->int_binding = dsda_IntConfig(conf->id);
}

// No side effects
static void dsda_InitIntConfig(dsda_config_t* conf, int value, dboolean persist)
{
	conf->transient_value.v_int = value;

	dsda_ConstrainIntConfig(conf);
	if(persist)
		dsda_PersistIntConfig(conf);
	dsda_PropagateIntConfig(conf);
}

// No side effects
static void dsda_InitStringConfig(dsda_config_t* conf, const char* value, dboolean persist)
{
	if(conf->transient_value.v_string)
		Z_Free(conf->transient_value.v_string);

	conf->transient_value.v_string = Z_Strdup(value);
	if(persist)
		dsda_PersistStringConfig(conf);
}

// No side effects
void dsda_RevertIntConfig(ConfigId id)
{
	dsda_config[id].transient_value.v_int = dsda_config[id].persistent_value.v_int;
}

int dsda_MaxConfigLength()
{
	int length = 0;

	// Entry 0 is ConfigId::None, which has no config
	for(dsda_config_t& config_entry : dsda_config | std::views::drop(1))
	{
		dsda_config_t* conf = &config_entry;

		if(strlen(conf->name) > length)
			length = strlen(conf->name);
	}

	return length;
}

void dsda_InitConfig()
{
	// Entry 0 is ConfigId::None, which has no config
	for(dsda_config_t& config_entry : dsda_config | std::views::drop(1))
	{
		dsda_config_t* conf = &config_entry;

		if(conf->type == ConfigType::Int)
			dsda_InitIntConfig(conf, conf->default_value.v_int, true);
		else if(conf->type == ConfigType::String)
			dsda_InitStringConfig(conf, conf->default_value.v_string, true);
	}
}

dboolean dsda_ReadConfig(const char* name, const char* string_param, int int_param)
{
	ConfigId id;

	id = dsda_ConfigIDByName(name);

	if(id != ConfigId::None)
	{
		dsda_config_t* conf;

		conf = &dsda_config[id];

		if(conf->type == ConfigType::Int && !string_param)
			dsda_InitIntConfig(conf, int_param, true);
		else if(conf->type == ConfigType::String && string_param)
			dsda_InitStringConfig(conf, string_param, true);

		return true;
	}

	return false;
}

void dsda_WriteConfig(ConfigId id, int key_length, FILE* file)
{
	dsda_config_t* conf;

	conf = &dsda_config[id];

	if(conf->type == ConfigType::Int)
		fprintf(file, "%-*s %i\n", key_length, conf->name, conf->persistent_value.v_int);
	else if(conf->type == ConfigType::String)
		fprintf(file, "%-*s \"%s\"\n", key_length, conf->name, conf->persistent_value.v_string);
}

static void dsda_ParseConfigArg(ArgId arg_id, dboolean persist)
{
	dsda_arg_t* arg;

	arg = dsda_Arg(static_cast<ArgId>(arg_id));
	if(arg->found)
	{
		int i;

		for(i = 0; i < arg->count; ++i)
		{
			ConfigId id;
			dsda_config_t* conf;
			char* pair;
			char** key_value;

			pair = Z_Strdup(arg->value.v_string_array[i]);
			key_value = dsda_SplitString(pair, "=");
			if(!key_value[0] || !key_value[1])
				Log::Fatal("Invalid config variable assignment \"{}\" (use key=value)", pair);

			id = dsda_ConfigIDByName(key_value[0]);
			if(id == ConfigId::None)
				Log::Fatal("Unknown config variable \"{}\"", key_value[0]);

			conf = &dsda_config[id];
			if(conf->type == ConfigType::Int)
			{
				int value;
				char* str_end;

				errno = 0;
				value = strtol(key_value[1], &str_end, 0);
				if(errno != 0)
				{
					Log::Fatal("Config variable \"{}\" requires an integer value (was \"{}\", err \"{}\")", key_value[0], key_value[1], strerror(errno));
				}
				if(*str_end != '\0')
				{
					Log::Fatal("Value for config variable \"{}\" was not converted into an integer in its entirety (was \"{}\")", key_value[0], key_value[1]);
				}

				dsda_InitIntConfig(conf, value, persist);
			}
			else
			{
				dsda_InitStringConfig(conf, key_value[1], persist);
			}

			Z_Free(pair);
			Z_Free(key_value);
		}
	}
}

void dsda_ApplyAdHocConfiguration()
{
	dsda_arg_t* arg;

	dsda_ParseConfigArg(ArgId::Update, true);
	dsda_ParseConfigArg(ArgId::Assign, false);

	arg = dsda_Arg(ArgId::GameSpeed);
	if(arg->found)
		dsda_ReadConfig("game_speed", nullptr, arg->value.v_int);
}

int dsda_ToggleConfig(ConfigId id, dboolean persist)
{
	return dsda_UpdateIntConfig(id, !dsda_config[id].transient_value.v_int, persist);
}

int dsda_IncrementIntConfig(ConfigId id, dboolean persist)
{
	return dsda_UpdateIntConfig(id, dsda_config[id].transient_value.v_int + 1, persist);
}

int dsda_DecrementIntConfig(ConfigId id, dboolean persist)
{
	return dsda_UpdateIntConfig(id, dsda_config[id].transient_value.v_int - 1, persist);
}

int dsda_CycleConfig(ConfigId id, dboolean persist)
{
	int value;

	value = dsda_config[id].transient_value.v_int + 1;

	if(value > dsda_config[id].upper_limit)
		value = dsda_config[id].lower_limit;

	return dsda_UpdateIntConfig(id, value, persist);
}

int dsda_UpdateIntConfig(ConfigId id, int value, dboolean persist)
{
	dsda_config[id].transient_value.v_int = value;

	dsda_ConstrainIntConfig(&dsda_config[id]);

	if(persist)
		dsda_PersistIntConfig(&dsda_config[id]);

	dsda_PropagateIntConfig(&dsda_config[id]);

	if(dsda_config[id].onUpdate)
		dsda_config[id].onUpdate();

	if((dsda_config[id].flags & ConfigFlag::Feature) != ConfigFlag{})
		dsda_TrackConfigFeatures();

	return dsda_IntConfig(id);
}

const char* dsda_UpdateStringConfig(ConfigId id, const char* value, dboolean persist)
{
	if(dsda_config[id].transient_value.v_string)
		Z_Free(dsda_config[id].transient_value.v_string);

	dsda_config[id].transient_value.v_string = Z_Strdup(value);

	if(persist)
		dsda_PersistStringConfig(&dsda_config[id]);

	if(dsda_config[id].onUpdate)
		dsda_config[id].onUpdate();

	return dsda_StringConfig(id);
}

// No callbacks, to avoid recursion cases
extern "C" const char* dsda_HackStringConfig(ConfigId id, const char* value, dboolean persist)
{
	if(dsda_config[id].transient_value.v_string)
		Z_Free(dsda_config[id].transient_value.v_string);

	dsda_config[id].transient_value.v_string = Z_Strdup(value);

	if(persist)
		dsda_PersistStringConfig(&dsda_config[id]);

	return dsda_StringConfig(id);
}

extern "C" dboolean dsda_StrictMode();
int dsda_IntConfig(ConfigId id)
{

	if((dsda_config[id].flags & ConfigFlag::Strict) != ConfigFlag{} && dsda_StrictMode())
		return dsda_config[id].strict_lower_limit;

	if((dsda_config[id].flags & ConfigFlag::StrictRange) != ConfigFlag{} && dsda_StrictMode())
	{
		if(dsda_config[id].transient_value.v_int < dsda_config[id].strict_lower_limit)
			return dsda_config[id].strict_lower_limit;

		if(dsda_config[id].transient_value.v_int > dsda_config[id].strict_upper_limit)
			return dsda_config[id].strict_upper_limit;
	}

	return dsda_config[id].transient_value.v_int;
}

dboolean dsda_IsStrictConfig(ConfigId id)
{
	return (dsda_config[id].flags & ConfigFlag::Strict) != ConfigFlag{};
}

int dsda_TransientIntConfig(ConfigId id)
{
	return dsda_config[id].transient_value.v_int;
}

const char* dsda_StringConfig(ConfigId id)
{
	return dsda_config[id].transient_value.v_string;
}

char* dsda_ConfigSummary(const char* name)
{
	ConfigId id;
	char* summary = nullptr;
	size_t length;

	id = dsda_ConfigIDByName(name);

	if(id != ConfigId::None)
	{
		dsda_config_t* conf;

		conf = &dsda_config[id];

		if(conf->type == ConfigType::Int)
		{
			length = snprintf(nullptr, 0,
				"%s: %d (transient), %d (persistent)", conf->name,
				conf->transient_value.v_int, conf->persistent_value.v_int);
			summary = static_cast<char *>(Z_Malloc(length + 1));
			snprintf(summary, length + 1,
				"%s: %d (transient), %d (persistent)", conf->name,
				conf->transient_value.v_int, conf->persistent_value.v_int);
		}
		else if(conf->type == ConfigType::String)
		{
			length = snprintf(nullptr, 0,
				"%s: %s (transient), %s (persistent)", conf->name,
				conf->transient_value.v_string, conf->persistent_value.v_string);
			summary = static_cast<char *>(Z_Malloc(length + 1));
			snprintf(summary, length + 1,
				"%s: %s (transient), %s (persistent)", conf->name,
				conf->transient_value.v_string, conf->persistent_value.v_string);
		}

		return summary;
	}

	return nullptr;
}

ConfigId dsda_ConfigIDByName(const char* name)
{
	// Entry 0 is ConfigId::None, which has no config and also means "not found"
	const auto keys = dsda_config.Keys() | std::views::drop(1);
	const auto id = std::ranges::find_if(keys, [&](const ConfigId key)
	{
		return !strcmp(name, dsda_config[key].name);
	});

	return id == keys.end() ? ConfigId::None : *id;
}

ConfigType dsda_ConfigType(ConfigId id)
{
	return dsda_config[id].type;
}
