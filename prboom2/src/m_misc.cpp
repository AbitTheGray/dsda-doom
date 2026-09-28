// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Main loop menu stuff.
 *  Default Config File.
 *  PCX Screenshots.
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include <stdio.h>

#include "doomstat.hpp"
#include "g_game.hpp"
#include "i_system.hpp"
#include "i_sound.hpp"
#include "i_video.hpp"
#include "s_sound.hpp"
#include "lprintf.hpp"
#include "m_file.hpp"
#include "d_main.hpp"

#include "m_misc.hpp"

#include "dsda/args.hpp"
#include "dsda/game_controller.hpp"
#include "dsda/messenger.hpp"
#include "dsda/settings.hpp"

// NSM
#include "i_capture.hpp"

typedef struct
{
	const char* name;
	ConfigId config_id;
} cfg_def_t;

typedef struct
{
	const char* name;
	InputId identifier;
	dsda_input_default_t input;
} cfg_input_def_t;

#define SETTING_HEADING(str) { str, {} }
#define INPUT_SETTING(str, id, k, m, j) { str, id, { k, m, j } }
#define MIGRATED_SETTING(id) { nullptr, id }

cfg_def_t cfg_defs[] =
{
	//e6y
	SETTING_HEADING("System settings"),
	MIGRATED_SETTING(ConfigId::ProcessPriority),

	SETTING_HEADING("Misc settings"),
	MIGRATED_SETTING(ConfigId::VanillaKeymap),
	MIGRATED_SETTING(ConfigId::MenuBackground),
	MIGRATED_SETTING(ConfigId::MaxPlayerCorpse),
	MIGRATED_SETTING(ConfigId::FlashingHom),
	MIGRATED_SETTING(ConfigId::DemoSmoothturns),
	MIGRATED_SETTING(ConfigId::DemoSmoothturnsfactor),
	MIGRATED_SETTING(ConfigId::ScreenshotDir),
	MIGRATED_SETTING(ConfigId::StartupDelayMs),
	MIGRATED_SETTING(ConfigId::ShowEndoom),
	MIGRATED_SETTING(ConfigId::ExportEndoom),
	MIGRATED_SETTING(ConfigId::AnsiEndoom),
	MIGRATED_SETTING(ConfigId::QuitSounds),
	MIGRATED_SETTING(ConfigId::AnnounceMap),

	SETTING_HEADING("Game settings"),
	MIGRATED_SETTING(ConfigId::DefaultComplevel),
	MIGRATED_SETTING(ConfigId::DefaultSkill),
	MIGRATED_SETTING(ConfigId::WeaponAttackAlignment),
	MIGRATED_SETTING(ConfigId::StsColoredNumbers),
	MIGRATED_SETTING(ConfigId::StsPctAlwaysGray),
	MIGRATED_SETTING(ConfigId::StsTraditionalKeys),
	MIGRATED_SETTING(ConfigId::StsSolidBgColor),
	MIGRATED_SETTING(ConfigId::ShowMessages),
	MIGRATED_SETTING(ConfigId::Autorun),
	MIGRATED_SETTING(ConfigId::DehApplyCheats),
	MIGRATED_SETTING(ConfigId::MovementStrafe50),
	MIGRATED_SETTING(ConfigId::MovementStrafe50onturns),
	MIGRATED_SETTING(ConfigId::MovementShorttics),

	SETTING_HEADING("Sound settings"),
	MIGRATED_SETTING(ConfigId::PitchedSounds),
	MIGRATED_SETTING(ConfigId::FullSounds),
	MIGRATED_SETTING(ConfigId::SndSamplerate),
	MIGRATED_SETTING(ConfigId::SndSamplecount),
	MIGRATED_SETTING(ConfigId::SfxVolume),
	MIGRATED_SETTING(ConfigId::MusicVolume),
	MIGRATED_SETTING(ConfigId::MusPauseOpt),
	MIGRATED_SETTING(ConfigId::SndChannels),
	MIGRATED_SETTING(ConfigId::SndMidiplayer),
	MIGRATED_SETTING(ConfigId::SndMididev),
	MIGRATED_SETTING(ConfigId::SndSoundfont),
	MIGRATED_SETTING(ConfigId::MusFluidsynthChorus),
	MIGRATED_SETTING(ConfigId::MusFluidsynthReverb),
	MIGRATED_SETTING(ConfigId::MusFluidsynthGain),
	MIGRATED_SETTING(ConfigId::MusFluidsynthChorusDepth),
	MIGRATED_SETTING(ConfigId::MusFluidsynthChorusLevel),
	MIGRATED_SETTING(ConfigId::MusFluidsynthReverbDamp),
	MIGRATED_SETTING(ConfigId::MusFluidsynthReverbLevel),
	MIGRATED_SETTING(ConfigId::MusFluidsynthReverbWidth),
	MIGRATED_SETTING(ConfigId::MusFluidsynthReverbRoomSize),
	MIGRATED_SETTING(ConfigId::MusOplGain),
	MIGRATED_SETTING(ConfigId::MusOplOpl3mode),
	MIGRATED_SETTING(ConfigId::MusPortmidiResetType),
	MIGRATED_SETTING(ConfigId::MusPortmidiResetDelay),
	MIGRATED_SETTING(ConfigId::MusPortmidiFilterSysex),
	MIGRATED_SETTING(ConfigId::MusPortmidiReverbLevel),
	MIGRATED_SETTING(ConfigId::MusPortmidiChorusLevel),

	SETTING_HEADING("Video settings"),
	MIGRATED_SETTING(ConfigId::Videomode),
	MIGRATED_SETTING(ConfigId::ScreenResolution),
	MIGRATED_SETTING(ConfigId::CustomResolution),
	MIGRATED_SETTING(ConfigId::UseFullscreen),
	MIGRATED_SETTING(ConfigId::ExclusiveFullscreen),
	MIGRATED_SETTING(ConfigId::RenderVsync),
	MIGRATED_SETTING(ConfigId::UncappedFramerate),
	MIGRATED_SETTING(ConfigId::TranslucentSprites),
	MIGRATED_SETTING(ConfigId::TranslucentGhosts),
	MIGRATED_SETTING(ConfigId::Screenblocks),
	MIGRATED_SETTING(ConfigId::Usegamma),
	MIGRATED_SETTING(ConfigId::FpsLimit),
	MIGRATED_SETTING(ConfigId::BackgroundFpsLimit),
	MIGRATED_SETTING(ConfigId::SdlVideoWindowPos),
	MIGRATED_SETTING(ConfigId::SdlVideoDisplayIndex),
	MIGRATED_SETTING(ConfigId::PaletteOndamage),
	MIGRATED_SETTING(ConfigId::PaletteOnbonus),
	MIGRATED_SETTING(ConfigId::PaletteOnpowers),
	MIGRATED_SETTING(ConfigId::RenderWipescreen),
	MIGRATED_SETTING(ConfigId::RenderScreenMultiply),
	MIGRATED_SETTING(ConfigId::IntegerScaling),
	MIGRATED_SETTING(ConfigId::RenderAspect),
	MIGRATED_SETTING(ConfigId::RenderDoomLightmaps),
	MIGRATED_SETTING(ConfigId::FakeContrastMode),
	MIGRATED_SETTING(ConfigId::RenderStretchHud),
	MIGRATED_SETTING(ConfigId::RenderPatchesScalex),
	MIGRATED_SETTING(ConfigId::RenderPatchesScaley),
	MIGRATED_SETTING(ConfigId::RenderStretchsky),
	MIGRATED_SETTING(ConfigId::RenderLinearsky),
	MIGRATED_SETTING(ConfigId::AspectRatioCorrection),
	MIGRATED_SETTING(ConfigId::Freelook),
	MIGRATED_SETTING(ConfigId::ExtraLevelBrightness),

	SETTING_HEADING("OpenGL settings"),
	MIGRATED_SETTING(ConfigId::GlRenderMultisampling),
	MIGRATED_SETTING(ConfigId::GlRenderFov),
	MIGRATED_SETTING(ConfigId::GlSkymode),
	MIGRATED_SETTING(ConfigId::GlHealthBar),
	MIGRATED_SETTING(ConfigId::GlUsevbo),
	MIGRATED_SETTING(ConfigId::GlFadeMode),

	SETTING_HEADING("Mouse settings"),
	MIGRATED_SETTING(ConfigId::UseMouse),
	MIGRATED_SETTING(ConfigId::MouseStutterCorrection),
	MIGRATED_SETTING(ConfigId::MouseSensitivityHoriz),
	MIGRATED_SETTING(ConfigId::FineSensitivity),
	MIGRATED_SETTING(ConfigId::MouseSensitivityVert),
	MIGRATED_SETTING(ConfigId::MouseAcceleration),
	MIGRATED_SETTING(ConfigId::MouseSensitivityMlook),
	MIGRATED_SETTING(ConfigId::MouseDoubleclickAsUse),
	MIGRATED_SETTING(ConfigId::MouseCarrytics),
	MIGRATED_SETTING(ConfigId::Vertmouse),
	MIGRATED_SETTING(ConfigId::MovementMousestrafedivisor),
	MIGRATED_SETTING(ConfigId::MovementMouseinvert),

	SETTING_HEADING("Game controller settings"),
	MIGRATED_SETTING(ConfigId::UseGameController),
	MIGRATED_SETTING(ConfigId::LeftAnalogDeadzone),
	MIGRATED_SETTING(ConfigId::RightAnalogDeadzone),
	MIGRATED_SETTING(ConfigId::LeftTriggerDeadzone),
	MIGRATED_SETTING(ConfigId::RightTriggerDeadzone),
	MIGRATED_SETTING(ConfigId::LeftAnalogSensitivityX),
	MIGRATED_SETTING(ConfigId::LeftAnalogSensitivityY),
	MIGRATED_SETTING(ConfigId::RightAnalogSensitivityX),
	MIGRATED_SETTING(ConfigId::RightAnalogSensitivityY),
	MIGRATED_SETTING(ConfigId::AnalogLookAcceleration),
	MIGRATED_SETTING(ConfigId::SwapAnalogs),
	MIGRATED_SETTING(ConfigId::InvertAnalogLook),

	SETTING_HEADING("Automap settings"),
	MIGRATED_SETTING(ConfigId::MapcolorBack),
	MIGRATED_SETTING(ConfigId::MapcolorGrid),
	MIGRATED_SETTING(ConfigId::MapcolorWall),
	MIGRATED_SETTING(ConfigId::MapcolorFchg),
	MIGRATED_SETTING(ConfigId::MapcolorCchg),
	MIGRATED_SETTING(ConfigId::MapcolorClsd),
	MIGRATED_SETTING(ConfigId::MapcolorRkey),
	MIGRATED_SETTING(ConfigId::MapcolorBkey),
	MIGRATED_SETTING(ConfigId::MapcolorYkey),
	MIGRATED_SETTING(ConfigId::MapcolorRdor),
	MIGRATED_SETTING(ConfigId::MapcolorBdor),
	MIGRATED_SETTING(ConfigId::MapcolorYdor),
	MIGRATED_SETTING(ConfigId::MapcolorTele),
	MIGRATED_SETTING(ConfigId::MapcolorSecr),
	MIGRATED_SETTING(ConfigId::MapcolorRevsecr),
	MIGRATED_SETTING(ConfigId::MapcolorTagfinder),
	MIGRATED_SETTING(ConfigId::MapcolorExit),
	MIGRATED_SETTING(ConfigId::MapcolorExitsecr),
	MIGRATED_SETTING(ConfigId::MapcolorUnsn),
	MIGRATED_SETTING(ConfigId::MapcolorFlat),
	MIGRATED_SETTING(ConfigId::MapcolorSprt),
	MIGRATED_SETTING(ConfigId::MapcolorItem),
	MIGRATED_SETTING(ConfigId::MapcolorHair),
	MIGRATED_SETTING(ConfigId::MapcolorSngl),
	MIGRATED_SETTING(ConfigId::MapcolorMe),
	MIGRATED_SETTING(ConfigId::MapcolorEnemy),
	MIGRATED_SETTING(ConfigId::MapcolorFrnd),
	MIGRATED_SETTING(ConfigId::MapcolorTrail1),
	MIGRATED_SETTING(ConfigId::MapcolorTrail2),
	MIGRATED_SETTING(ConfigId::MapBlinkingLocks),
	MIGRATED_SETTING(ConfigId::MapSecretAfter),
	MIGRATED_SETTING(ConfigId::MapCoordinates),
	MIGRATED_SETTING(ConfigId::MapTotals),
	MIGRATED_SETTING(ConfigId::MapTime),
	MIGRATED_SETTING(ConfigId::MapTitle),
	MIGRATED_SETTING(ConfigId::MapTrail),
	MIGRATED_SETTING(ConfigId::MapTrailCollisions),
	MIGRATED_SETTING(ConfigId::MapTrailSize),
	MIGRATED_SETTING(ConfigId::MapTraces),
	MIGRATED_SETTING(ConfigId::AutomapOverlay),
	MIGRATED_SETTING(ConfigId::AutomapRotate),
	MIGRATED_SETTING(ConfigId::AutomapFollow),
	MIGRATED_SETTING(ConfigId::AutomapGrid),
	MIGRATED_SETTING(ConfigId::MapGridSize),
	MIGRATED_SETTING(ConfigId::MapPanSpeed),
	MIGRATED_SETTING(ConfigId::MapScrollSpeed),
	MIGRATED_SETTING(ConfigId::MapWheelZoom),
	MIGRATED_SETTING(ConfigId::MapUseMultisamling),
	MIGRATED_SETTING(ConfigId::MapTextured),
	MIGRATED_SETTING(ConfigId::MapTexturedTrans),
	MIGRATED_SETTING(ConfigId::MapTexturedOverlayTrans),
	MIGRATED_SETTING(ConfigId::MapLinesOverlayTrans),
	MIGRATED_SETTING(ConfigId::MapThingsAppearance),

	SETTING_HEADING("Heads-up display settings"),
	MIGRATED_SETTING(ConfigId::HudHealthRed),
	MIGRATED_SETTING(ConfigId::HudHealthYellow),
	MIGRATED_SETTING(ConfigId::HudHealthGreen),
	MIGRATED_SETTING(ConfigId::HudAmmoRed),
	MIGRATED_SETTING(ConfigId::HudAmmoYellow),
	MIGRATED_SETTING(ConfigId::HudDisplayed),
	MIGRATED_SETTING(ConfigId::HudaddSecretarea),
	MIGRATED_SETTING(ConfigId::HudaddDemoprogressbar),
	MIGRATED_SETTING(ConfigId::HudaddCrosshair),
	MIGRATED_SETTING(ConfigId::HudaddCrosshairScale),
	MIGRATED_SETTING(ConfigId::HudaddCrosshairColor),
	MIGRATED_SETTING(ConfigId::HudaddCrosshairHealth),
	MIGRATED_SETTING(ConfigId::HudaddCrosshairTarget),
	MIGRATED_SETTING(ConfigId::HudaddCrosshairTargetColor),
	MIGRATED_SETTING(ConfigId::HudaddCrosshairLockTarget),

	SETTING_HEADING("DSDA-Doom settings"),
	// MIGRATED_SETTING(dsda_config_strict_mode), Do not persist
	MIGRATED_SETTING(ConfigId::CycleGhostColors),
	MIGRATED_SETTING(ConfigId::AutoKeyFrameInterval),
	MIGRATED_SETTING(ConfigId::AutoKeyFrameDepth),
	MIGRATED_SETTING(ConfigId::AutoKeyFrameTimeout),
	MIGRATED_SETTING(ConfigId::AutoSave),
	MIGRATED_SETTING(ConfigId::Exhud),
	MIGRATED_SETTING(ConfigId::ExTextScaleX),
	MIGRATED_SETTING(ConfigId::ExTextRatioY),
	MIGRATED_SETTING(ConfigId::FreeText),
	MIGRATED_SETTING(ConfigId::WipeAtFullSpeed),
	MIGRATED_SETTING(ConfigId::ShowDemoAttempts),
	MIGRATED_SETTING(ConfigId::HideHorns),
	MIGRATED_SETTING(ConfigId::HideWeapon),
	MIGRATED_SETTING(ConfigId::OrganizedSaves),
	MIGRATED_SETTING(ConfigId::CommandDisplay),
	MIGRATED_SETTING(ConfigId::CommandHistorySize),
	MIGRATED_SETTING(ConfigId::HideEmptyCommands),
	MIGRATED_SETTING(ConfigId::CoordinateDisplay),
	MIGRATED_SETTING(ConfigId::ShowFps),
	MIGRATED_SETTING(ConfigId::ShowMinimap),
	MIGRATED_SETTING(ConfigId::ShowLevelSplits),
	MIGRATED_SETTING(ConfigId::SkipQuitPrompt),
	MIGRATED_SETTING(ConfigId::ShowSplitData),
	MIGRATED_SETTING(ConfigId::PlayerName),
	MIGRATED_SETTING(ConfigId::QuickstartCacheTics),
	MIGRATED_SETTING(ConfigId::DeathUseAction),
	MIGRATED_SETTING(ConfigId::MuteSfx),
	MIGRATED_SETTING(ConfigId::MuteMusic),
	MIGRATED_SETTING(ConfigId::MuteUnfocusedWindow),
	MIGRATED_SETTING(ConfigId::CheatCodes),
	MIGRATED_SETTING(ConfigId::AllowJumping),
	MIGRATED_SETTING(ConfigId::AlwaysPistolStart),
	MIGRATED_SETTING(ConfigId::ParallelSfxLimit),
	MIGRATED_SETTING(ConfigId::ParallelSfxWindow),
	MIGRATED_SETTING(ConfigId::MovementToggleSfx),
	MIGRATED_SETTING(ConfigId::SwitchWhenAmmoRunsOut),
	MIGRATED_SETTING(ConfigId::SwitchWeaponOnPickup),
	MIGRATED_SETTING(ConfigId::Viewbob),
	MIGRATED_SETTING(ConfigId::Weaponbob),
	MIGRATED_SETTING(ConfigId::FixViewbobFloorJolt),
	MIGRATED_SETTING(ConfigId::QuakeIntensity),
	MIGRATED_SETTING(ConfigId::OrganizeFailedDemos),
	MIGRATED_SETTING(ConfigId::DemoEndQuit),
	MIGRATED_SETTING(ConfigId::PlaybackMouseControls),

	SETTING_HEADING("Scripts"),
	MIGRATED_SETTING(ConfigId::Script0),
	MIGRATED_SETTING(ConfigId::Script1),
	MIGRATED_SETTING(ConfigId::Script2),
	MIGRATED_SETTING(ConfigId::Script3),
	MIGRATED_SETTING(ConfigId::Script4),
	MIGRATED_SETTING(ConfigId::Script5),
	MIGRATED_SETTING(ConfigId::Script6),
	MIGRATED_SETTING(ConfigId::Script7),
	MIGRATED_SETTING(ConfigId::Script8),
	MIGRATED_SETTING(ConfigId::Script9),

	// NSM
	SETTING_HEADING("Video capture encoding settings"),
	MIGRATED_SETTING(ConfigId::CapSoundcommand),
	MIGRATED_SETTING(ConfigId::CapVideocommand),
	MIGRATED_SETTING(ConfigId::CapMuxcommand),
	MIGRATED_SETTING(ConfigId::CapTempfile1),
	MIGRATED_SETTING(ConfigId::CapTempfile2),
	MIGRATED_SETTING(ConfigId::CapRemoveTempfiles),
	MIGRATED_SETTING(ConfigId::CapWipescreen),
	MIGRATED_SETTING(ConfigId::CapFps),

	SETTING_HEADING("Overrun settings"),
	MIGRATED_SETTING(ConfigId::OverrunSpechitWarn),
	MIGRATED_SETTING(ConfigId::OverrunSpechitEmulate),
	MIGRATED_SETTING(ConfigId::OverrunRejectWarn),
	MIGRATED_SETTING(ConfigId::OverrunRejectEmulate),
	MIGRATED_SETTING(ConfigId::OverrunInterceptWarn),
	MIGRATED_SETTING(ConfigId::OverrunInterceptEmulate),
	MIGRATED_SETTING(ConfigId::OverrunPlayeringameWarn),
	MIGRATED_SETTING(ConfigId::OverrunPlayeringameEmulate),
	MIGRATED_SETTING(ConfigId::OverrunDonutWarn),
	MIGRATED_SETTING(ConfigId::OverrunDonutEmulate),
	MIGRATED_SETTING(ConfigId::OverrunMissedbacksideWarn),
	MIGRATED_SETTING(ConfigId::OverrunMissedbacksideEmulate),

	SETTING_HEADING("Mapping error compatibility settings"),
	MIGRATED_SETTING(ConfigId::ComperrPassuse),
	MIGRATED_SETTING(ConfigId::ComperrHangsolid),
	MIGRATED_SETTING(ConfigId::ComperrBlockmap),

	SETTING_HEADING("Weapon preferences"),
	MIGRATED_SETTING(ConfigId::WeaponChoice1),
	MIGRATED_SETTING(ConfigId::WeaponChoice2),
	MIGRATED_SETTING(ConfigId::WeaponChoice3),
	MIGRATED_SETTING(ConfigId::WeaponChoice4),
	MIGRATED_SETTING(ConfigId::WeaponChoice5),
	MIGRATED_SETTING(ConfigId::WeaponChoice6),
	MIGRATED_SETTING(ConfigId::WeaponChoice7),
	MIGRATED_SETTING(ConfigId::WeaponChoice8),
	MIGRATED_SETTING(ConfigId::WeaponChoice9),

	SETTING_HEADING("Input settings"),
	MIGRATED_SETTING(ConfigId::InputProfile),
};

cfg_input_def_t input_defs[] = {
	INPUT_SETTING("input_forward", InputId::Forward, KeyCode::W, 2, -1),
	INPUT_SETTING("input_backward", InputId::Backward, KeyCode::S, -1, -1),
	INPUT_SETTING("input_turnleft", InputId::Turnleft, KeyCode::E, -1, -1),
	INPUT_SETTING("input_turnright", InputId::Turnright, KeyCode::Q, -1, -1),
	INPUT_SETTING("input_speed", InputId::Speed, KeyCode::None, -1, -1),
	INPUT_SETTING("input_strafeleft", InputId::Strafeleft, KeyCode::A, -1, -1),
	INPUT_SETTING("input_straferight", InputId::Straferight, KeyCode::D, -1, -1),
	INPUT_SETTING("input_strafe", InputId::Strafe, KeyCode::None, 1, std::to_underlying(GameControllerButton::Leftshoulder)),
	INPUT_SETTING("input_autorun", InputId::Autorun, KeyCode::CapsLock, -1, std::to_underlying(GameControllerButton::Leftstick)),
	INPUT_SETTING("input_reverse", InputId::Reverse, KeyCode::Slash, -1, std::to_underlying(GameControllerButton::Rightstick)),
	INPUT_SETTING("input_use", InputId::Use, KeyCode::Space, -1, std::to_underlying(GameControllerButton::A)),
	INPUT_SETTING("input_flyup", InputId::Flyup, KeyCode::Period, -1, std::to_underlying(GameControllerButton::DpadUp)),
	INPUT_SETTING("input_flydown", InputId::Flydown, KeyCode::Comma, -1, std::to_underlying(GameControllerButton::DpadDown)),
	INPUT_SETTING("input_flycenter", InputId::Flycenter, KeyCode::None, -1, -1),
	INPUT_SETTING("input_mlook", InputId::Mlook, KeyCode::Backslash, -1, -1),
	INPUT_SETTING("input_novert", InputId::Novert, KeyCode::None, -1, -1),

	INPUT_SETTING("input_weapon1", InputId::Weapon1, KeyCode::Digit1, -1, -1),
	INPUT_SETTING("input_weapon2", InputId::Weapon2, KeyCode::Digit2, -1, -1),
	INPUT_SETTING("input_weapon3", InputId::Weapon3, KeyCode::Digit3, -1, -1),
	INPUT_SETTING("input_weapon4", InputId::Weapon4, KeyCode::Digit4, -1, -1),
	INPUT_SETTING("input_weapon5", InputId::Weapon5, KeyCode::Digit5, -1, -1),
	INPUT_SETTING("input_weapon6", InputId::Weapon6, KeyCode::Digit6, -1, -1),
	INPUT_SETTING("input_weapon7", InputId::Weapon7, KeyCode::Digit7, -1, -1),
	INPUT_SETTING("input_weapon8", InputId::Weapon8, KeyCode::Digit8, -1, -1),
	INPUT_SETTING("input_weapon9", InputId::Weapon9, KeyCode::Digit9, -1, -1),
	INPUT_SETTING("input_nextweapon", InputId::Nextweapon, KeyCode::None, -1, std::to_underlying(GameControllerButton::Y)),
	INPUT_SETTING("input_prevweapon", InputId::Prevweapon, KeyCode::None, -1, std::to_underlying(GameControllerButton::X)),
	INPUT_SETTING("input_toggleweapon", InputId::Toggleweapon, KeyCode::Digit0, -1, -1),
	INPUT_SETTING("input_fire", InputId::Fire, KeyCode::Ctrl, 0, std::to_underlying(GameControllerButton::Triggerright)),

	INPUT_SETTING("input_help", InputId::Help, KeyCode::F1, -1, -1),
	INPUT_SETTING("input_pause", InputId::Pause, KeyCode::Pause, -1, -1),
	INPUT_SETTING("input_map", InputId::Map, KeyCode::Tab, -1, std::to_underlying(GameControllerButton::Triggerleft)),
	INPUT_SETTING("input_soundvolume", InputId::Soundvolume, KeyCode::F4, -1, -1),
	INPUT_SETTING("input_hud", InputId::Hud, KeyCode::F5, -1, -1),
	INPUT_SETTING("input_messages", InputId::Messages, KeyCode::F8, -1, -1),
	INPUT_SETTING("input_gamma", InputId::Gamma, KeyCode::F11, -1, -1),
	INPUT_SETTING("input_spy", InputId::Spy, KeyCode::F12, -1, -1),
	INPUT_SETTING("input_zoomin", InputId::Zoomin, KeyCode::Equals, -1, -1),
	INPUT_SETTING("input_zoomout", InputId::Zoomout, KeyCode::Minus, -1, -1),
	INPUT_SETTING("input_screenshot", InputId::Screenshot, KeyCode::Asterisk, -1, -1),
	INPUT_SETTING("input_savegame", InputId::Savegame, KeyCode::F2, -1, -1),
	INPUT_SETTING("input_loadgame", InputId::Loadgame, KeyCode::F3, -1, -1),
	INPUT_SETTING("input_quicksave", InputId::Quicksave, KeyCode::F6, -1, -1),
	INPUT_SETTING("input_quickload", InputId::Quickload, KeyCode::F9, -1, -1),
	INPUT_SETTING("input_level_table", InputId::LevelTable, KeyCode::None, -1, -1),
	INPUT_SETTING("input_endgame", InputId::Endgame, KeyCode::F7, -1, -1),
	INPUT_SETTING("input_quit", InputId::Quit, KeyCode::F10, -1, -1),

	INPUT_SETTING("input_map_follow", InputId::MapFollow, KeyCode::F, -1, -1),
	INPUT_SETTING("input_map_zoomin", InputId::MapZoomin, KeyCode::Equals, -1, -1),
	INPUT_SETTING("input_map_zoomout", InputId::MapZoomout, KeyCode::Minus, -1, -1),
	INPUT_SETTING("input_map_up", InputId::MapUp, KeyCode::UpArrow, -1, -1),
	INPUT_SETTING("input_map_down", InputId::MapDown, KeyCode::DownArrow, -1, -1),
	INPUT_SETTING("input_map_left", InputId::MapLeft, KeyCode::LeftArrow, -1, -1),
	INPUT_SETTING("input_map_right", InputId::MapRight, KeyCode::RightArrow, -1, -1),
	INPUT_SETTING("input_map_mark", InputId::MapMark, KeyCode::M, -1, -1),
	INPUT_SETTING("input_map_clear", InputId::MapClear, KeyCode::C, -1, -1),
	INPUT_SETTING("input_map_gobig", InputId::MapGobig, KeyCode::Digit0, -1, -1),
	INPUT_SETTING("input_map_grid", InputId::MapGrid, KeyCode::G, -1, -1),
	INPUT_SETTING("input_map_rotate", InputId::MapRotate, KeyCode::R, -1, -1),
	INPUT_SETTING("input_map_overlay", InputId::MapOverlay, KeyCode::O, -1, -1),
	INPUT_SETTING("input_map_textured", InputId::MapTextured, KeyCode::None, -1, -1),
	INPUT_SETTING("input_map_highlight_by_tag", InputId::MapHighlightByTag, KeyCode::H, -1, -1),

	INPUT_SETTING("input_repeat_message", InputId::RepeatMessage, KeyCode::None, -1, -1),

	INPUT_SETTING("input_speed_up", InputId::SpeedUp, KeyCode::None, -1, -1),
	INPUT_SETTING("input_speed_down", InputId::SpeedDown, KeyCode::None, -1, -1),
	INPUT_SETTING("input_speed_default", InputId::SpeedDefault, KeyCode::None, -1, -1),
	INPUT_SETTING("input_demo_skip", InputId::DemoSkip, KeyCode::Insert, -1, -1),
	INPUT_SETTING("input_demo_endlevel", InputId::DemoEndlevel, KeyCode::End, -1, -1),
	INPUT_SETTING("input_walkcamera", InputId::Walkcamera, KeyCode::Keypad0, -1, -1),
	INPUT_SETTING("input_join_demo", InputId::JoinDemo, KeyCode::None, -1, -1),
	INPUT_SETTING("input_restart", InputId::Restart, KeyCode::Home, -1, -1),
	INPUT_SETTING("input_nextlevel", InputId::Nextlevel, KeyCode::PageDown, -1, -1),
	INPUT_SETTING("input_prevlevel", InputId::Prevlevel, KeyCode::PageUp, -1, -1),
	INPUT_SETTING("input_showalive", InputId::Showalive, KeyCode::None, -1, -1),

	INPUT_SETTING("input_menu_down", InputId::MenuDown, KeyCode::DownArrow, -1, std::to_underlying(GameControllerButton::DpadDown)),
	INPUT_SETTING("input_menu_up", InputId::MenuUp, KeyCode::UpArrow, -1, std::to_underlying(GameControllerButton::DpadUp)),
	INPUT_SETTING("input_menu_left", InputId::MenuLeft, KeyCode::LeftArrow, -1, std::to_underlying(GameControllerButton::DpadLeft)),
	INPUT_SETTING("input_menu_right", InputId::MenuRight, KeyCode::RightArrow, -1, std::to_underlying(GameControllerButton::DpadRight)),
	INPUT_SETTING("input_menu_backspace", InputId::MenuBackspace, KeyCode::Backspace, -1, std::to_underlying(GameControllerButton::B)),
	INPUT_SETTING("input_menu_enter", InputId::MenuEnter, KeyCode::Enter, -1, std::to_underlying(GameControllerButton::A)),
	INPUT_SETTING("input_menu_escape", InputId::MenuEscape, KeyCode::Escape, -1, std::to_underlying(GameControllerButton::Start)),
	INPUT_SETTING("input_menu_clear", InputId::MenuClear, KeyCode::Delete, -1, std::to_underlying(GameControllerButton::Back)),

	INPUT_SETTING("input_iddqd", InputId::Iddqd, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idkfa", InputId::Idkfa, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idfa", InputId::Idfa, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idclip", InputId::Idclip, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholdh", InputId::Idbeholdh, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholdm", InputId::Idbeholdm, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholdv", InputId::Idbeholdv, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholds", InputId::Idbeholds, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholdi", InputId::Idbeholdi, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholdr", InputId::Idbeholdr, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholda", InputId::Idbeholda, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idbeholdl", InputId::Idbeholdl, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idmypos", InputId::Idmypos, KeyCode::None, -1, -1),
	INPUT_SETTING("input_idrate", InputId::Idrate, KeyCode::None, -1, -1),
	INPUT_SETTING("input_iddt", InputId::Iddt, KeyCode::None, -1, -1),
	INPUT_SETTING("input_ponce", InputId::Ponce, KeyCode::None, -1, -1),
	INPUT_SETTING("input_shazam", InputId::Shazam, KeyCode::None, -1, -1),
	INPUT_SETTING("input_chicken", InputId::Chicken, KeyCode::None, -1, -1),

	INPUT_SETTING("input_lookup", InputId::Lookup, KeyCode::None, -1, -1),
	INPUT_SETTING("input_lookdown", InputId::Lookdown, KeyCode::None, -1, -1),
	INPUT_SETTING("input_lookcenter", InputId::Lookcenter, KeyCode::None, -1, -1),
	INPUT_SETTING("input_use_artifact", InputId::UseArtifact, KeyCode::Shift, -1, std::to_underlying(GameControllerButton::Rightshoulder)),
	INPUT_SETTING("input_arti_tome", InputId::ArtiTome, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_quartz", InputId::ArtiQuartz, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_urn", InputId::ArtiUrn, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_bomb", InputId::ArtiBomb, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_ring", InputId::ArtiRing, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_chaosdevice", InputId::ArtiChaosdevice, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_shadowsphere", InputId::ArtiShadowsphere, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_wings", InputId::ArtiWings, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_torch", InputId::ArtiTorch, KeyCode::None, -1, -1),
	INPUT_SETTING("input_arti_morph", InputId::ArtiMorph, KeyCode::None, -1, -1),
	INPUT_SETTING("input_invleft", InputId::Invleft, KeyCode::MouseWheelDown, -1, std::to_underlying(GameControllerButton::DpadLeft)),
	INPUT_SETTING("input_invright", InputId::Invright, KeyCode::MouseWheelUp, -1, std::to_underlying(GameControllerButton::DpadRight)),
	INPUT_SETTING("input_store_quick_key_frame", InputId::StoreQuickKeyFrame, KeyCode::None, -1, -1),
	INPUT_SETTING("input_restore_quick_key_frame", InputId::RestoreQuickKeyFrame, KeyCode::None, -1, -1),
	INPUT_SETTING("input_rewind", InputId::Rewind, KeyCode::None, -1, -1),
	INPUT_SETTING("input_cycle_profile", InputId::CycleProfile, KeyCode::None, -1, -1),
	INPUT_SETTING("input_cycle_palette", InputId::CyclePalette, KeyCode::None, -1, -1),
	INPUT_SETTING("input_command_display", InputId::CommandDisplay, KeyCode::None, -1, -1),
	INPUT_SETTING("input_strict_mode", InputId::StrictMode, KeyCode::None, -1, -1),
	INPUT_SETTING("input_console", InputId::Console, KeyCode::None, -1, -1),
	INPUT_SETTING("input_coordinate_display", InputId::CoordinateDisplay, KeyCode::None, -1, -1),
	INPUT_SETTING("input_fps", InputId::Fps, KeyCode::None, -1, -1),
	INPUT_SETTING("input_avj", InputId::Avj, KeyCode::None, -1, -1),
	INPUT_SETTING("input_exhud", InputId::Exhud, KeyCode::None, -1, -1),
	INPUT_SETTING("input_mute_sfx", InputId::MuteSfx, KeyCode::None, -1, -1),
	INPUT_SETTING("input_mute_music", InputId::MuteMusic, KeyCode::None, -1, -1),
	INPUT_SETTING("input_cheat_codes", InputId::CheatCodes, KeyCode::None, -1, -1),
	INPUT_SETTING("input_notarget", InputId::Notarget, KeyCode::None, -1, -1),
	INPUT_SETTING("input_freeze", InputId::Freeze, KeyCode::None, -1, -1),

	INPUT_SETTING("input_build", InputId::Build, KeyCode::None, -1, -1),
	INPUT_SETTING("input_build_advance_frame", InputId::BuildAdvanceFrame, KeyCode::RightArrow, -1, -1),
	INPUT_SETTING("input_build_reverse_frame", InputId::BuildReverseFrame, KeyCode::LeftArrow, -1, -1),
	INPUT_SETTING("input_build_reset_command", InputId::BuildResetCommand, KeyCode::Delete, -1, -1),
	INPUT_SETTING("input_build_source", InputId::BuildSource, KeyCode::Shift, -1, -1),
	INPUT_SETTING("input_build_forward", InputId::BuildForward, KeyCode::W, -1, -1),
	INPUT_SETTING("input_build_backward", InputId::BuildBackward, KeyCode::S, -1, -1),
	INPUT_SETTING("input_build_fine_forward", InputId::BuildFineForward, KeyCode::T, -1, -1),
	INPUT_SETTING("input_build_fine_backward", InputId::BuildFineBackward, KeyCode::G, -1, -1),
	INPUT_SETTING("input_build_turn_left", InputId::BuildTurnLeft, KeyCode::Q, -1, -1),
	INPUT_SETTING("input_build_turn_right", InputId::BuildTurnRight, KeyCode::E, -1, -1),
	INPUT_SETTING("input_build_strafe_left", InputId::BuildStrafeLeft, KeyCode::A, -1, -1),
	INPUT_SETTING("input_build_strafe_right", InputId::BuildStrafeRight, KeyCode::D, -1, -1),
	INPUT_SETTING("input_build_fine_strafe_left", InputId::BuildFineStrafeLeft, KeyCode::F, -1, -1),
	INPUT_SETTING("input_build_fine_strafe_right", InputId::BuildFineStrafeRight, KeyCode::H, -1, -1),
	INPUT_SETTING("input_build_use", InputId::BuildUse, KeyCode::Space, -1, -1),
	INPUT_SETTING("input_build_fire", InputId::BuildFire, KeyCode::Ctrl, -1, -1),
	INPUT_SETTING("input_build_weapon1", InputId::BuildWeapon1, KeyCode::Digit1, -1, -1),
	INPUT_SETTING("input_build_weapon2", InputId::BuildWeapon2, KeyCode::Digit2, -1, -1),
	INPUT_SETTING("input_build_weapon3", InputId::BuildWeapon3, KeyCode::Digit3, -1, -1),
	INPUT_SETTING("input_build_weapon4", InputId::BuildWeapon4, KeyCode::Digit4, -1, -1),
	INPUT_SETTING("input_build_weapon5", InputId::BuildWeapon5, KeyCode::Digit5, -1, -1),
	INPUT_SETTING("input_build_weapon6", InputId::BuildWeapon6, KeyCode::Digit6, -1, -1),
	INPUT_SETTING("input_build_weapon7", InputId::BuildWeapon7, KeyCode::Digit7, -1, -1),
	INPUT_SETTING("input_build_weapon8", InputId::BuildWeapon8, KeyCode::Digit8, -1, -1),
	INPUT_SETTING("input_build_weapon9", InputId::BuildWeapon9, KeyCode::Digit9, -1, -1),

	INPUT_SETTING("input_jump", InputId::Jump, KeyCode::Alt, -1, std::to_underlying(GameControllerButton::B)),
	INPUT_SETTING("input_hexen_arti_incant", InputId::HexenArtiIncant, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_summon", InputId::HexenArtiSummon, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_disk", InputId::HexenArtiDisk, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_flechette", InputId::HexenArtiFlechette, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_banishment", InputId::HexenArtiBanishment, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_boots", InputId::HexenArtiBoots, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_krater", InputId::HexenArtiKrater, KeyCode::None, -1, -1),
	INPUT_SETTING("input_hexen_arti_bracers", InputId::HexenArtiBracers, KeyCode::None, -1, -1),

	INPUT_SETTING("input_script_0", InputId::Script0, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_1", InputId::Script1, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_2", InputId::Script2, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_3", InputId::Script3, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_4", InputId::Script4, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_5", InputId::Script5, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_6", InputId::Script6, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_7", InputId::Script7, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_8", InputId::Script8, KeyCode::None, -1, -1),
	INPUT_SETTING("input_script_9", InputId::Script9, KeyCode::None, -1, -1),
};

static int input_def_count = sizeof(input_defs) / sizeof(input_defs[0]);
static int def_count = sizeof(cfg_defs) / sizeof(cfg_defs[0]);

static char* defaultfile; // CPhipps - static, const

static dboolean forget_config_file;

extern "C" void M_ForgetCurrentConfig()
{
	forget_config_file = true;
}

extern "C" void M_RememberCurrentConfig()
{
	forget_config_file = false;
}

void M_SaveDefaults()
{
	int i;
	FILE* f;
	int maxlen;

	if(forget_config_file)
		return;

	f = M_OpenFile(defaultfile, "w");
	if(!f)
		return; // can't write the file, but don't complain

	maxlen = dsda_MaxConfigLength();

	for(i = 0; i < input_def_count; i++)
	{
		int len;

		len = strlen(input_defs[i].name);
		if(len > maxlen && len < 80)
			maxlen = len;
	}

	// 3/3/98 explain format of file

	fprintf(f, "# Doom config file\n");
	fprintf(f, "# Format:\n");
	fprintf(f, "# variable   value\n");

	for(i = 0; i < def_count; i++)
	{
		if(cfg_defs[i].config_id != ConfigId::None)
		{
			dsda_WriteConfig(cfg_defs[i].config_id, maxlen, f);
		}
		else if(cfg_defs[i].name)
		{
			fprintf(f, "\n# %s\n", cfg_defs[i].name);
		}
	}

	fprintf(f, "\n");

	for(i = 0; i < input_def_count; ++i)
	{
		int a, j;
		dsda_input_t* input[DSDA_INPUT_PROFILE_COUNT];
		dsda_InputCopy(input_defs[i].identifier, input);

		fprintf(f, "%-*s", maxlen, input_defs[i].name);

		for(a = 0; a < DSDA_INPUT_PROFILE_COUNT; ++a)
		{
			if(input[a]->num_keys)
			{
				fprintf(f, " %i", std::to_underlying(input[a]->key[0]));
				for(j = 1; j < input[a]->num_keys; ++j)
				{
					fprintf(f, ",%i", std::to_underlying(input[a]->key[j]));
				}
			}
			else
				fprintf(f, " 0");

			fprintf(f, " %i %i", input[a]->mouseb, input[a]->joyb);

			if(a != DSDA_INPUT_PROFILE_COUNT - 1)
				fprintf(f, " |");
		}

		fprintf(f, "\n");
	}

	fclose(f);
}

//
// M_LoadDefaults
//

#define CFG_BUFFERMAX 32000

void M_LoadDefaults()
{
	int i;
	int len;
	FILE* f;
	char def[80];
	char* strparm = static_cast<char*>(Z_Malloc(CFG_BUFFERMAX));
	char* cfgline = static_cast<char*>(Z_Malloc(CFG_BUFFERMAX));
	char* newstring = nullptr; // killough
	int parm;
	dsda_arg_t* arg;

	// set everything to base values

	dsda_InitConfig();

	for(i = 0; i < input_def_count; i++)
	{
		int c;

		for(c = 0; c < DSDA_INPUT_PROFILE_COUNT; ++c)
			dsda_InputSetSpecific(c, input_defs[i].identifier, input_defs[i].input);
	}

	// check for a custom default file

	arg = dsda_Arg(ArgId::Config);
	if(arg->found)
	{
		defaultfile = Z_Strdup(arg->value.v_string);
	}
	else
	{
		const char* configdir = I_ConfigDir();
		int len = snprintf(nullptr, 0, "%s/dsda-doom.cfg", configdir);
		defaultfile = static_cast<char*>(Z_Malloc(len + 1));
		snprintf(defaultfile, len + 1, "%s/dsda-doom.cfg", configdir);
	}

	Log::Debug(" default file: {}\n", defaultfile);

	// read the file in, overriding any set defaults

	f = M_OpenFile(defaultfile, "r");
	if(f)
	{
		while(!feof(f))
		{
			parm = 0;

			cfgline = fgets(cfgline, CFG_BUFFERMAX, f);
			if(!cfgline)
				break;

			if(sscanf(cfgline, "%79s %[^\n]\n", def, strparm) == 2)
			{
				newstring = nullptr;

				//jff 3/3/98 skip lines not starting with an alphanum
				if(!isalnum(def[0]))
					continue;

				if(strparm[0] == '"')
				{
					// get a string
					len = strlen(strparm);
					newstring = static_cast<char*>(Z_Malloc(len));
					strparm[len - 1] = 0;           // clears trailing double-quote mark
					strcpy(newstring, strparm + 1); // clears leading double-quote mark
				}
				else if((strparm[0] == '0') && (strparm[1] == 'x'))
				{
					// CPhipps - allow ints to be specified in hex
					sscanf(strparm + 2, "%x", &parm);
				}
				else
				{
					sscanf(strparm, "%i", &parm);
				}

				if(dsda_ReadConfig(def, newstring, parm))
				{
					Z_Free(newstring);
				}
				else
				{
					for(i = 0; i < input_def_count; i++)
						if(!strcmp(def, input_defs[i].name))
						{
							int count;
							char keys[80];
							int key, mouseb, joyb;
							int index = 0;
							char* key_scan_p;
							char* config_scan_p;

							config_scan_p = strparm;
							do
							{
								count = sscanf(config_scan_p, "%79s %d %d", keys, &mouseb, &joyb);

								if(count != 3)
									break;

								dsda_InputResetSpecific(index, input_defs[i].identifier);

								dsda_InputAddSpecificMouseB(index, input_defs[i].identifier, mouseb);
								dsda_InputAddSpecificJoyB(index, input_defs[i].identifier, joyb);

								key_scan_p = strtok(keys, ",");
								do
								{
									count = sscanf(key_scan_p, "%d,", &key);

									if(count != 1)
										break;

									dsda_InputAddSpecificKey(index, input_defs[i].identifier, static_cast<KeyCode>(key));

									key_scan_p = strtok(nullptr, ",");
								}
								while(key_scan_p);

								index++;
								config_scan_p = strchr(config_scan_p, '|');
								if(config_scan_p)
									config_scan_p++;
							}
							while(config_scan_p && index < DSDA_INPUT_PROFILE_COUNT);

							break;
						}
				}
			}
		}

		fclose(f);
	}

	Z_Free(strparm);
	Z_Free(cfgline);

	dsda_ApplyAdHocConfiguration();

	dsda_InitSettings();

	//e6y: Check on existence of dsda-doom.wad
	port_wad_file = I_RequireFile(WAD_DATA, "");
}

//
// M_ScreenShot
//
// Modified by Lee Killough so that any number of shots can be taken,
// the code is faster, and no annoying "screenshot" message appears.

// CPhipps - modified to use its own buffer for the image
//         - checks for the case where no file can be created (doesn't occur on POSIX systems, would on DOS)
//         - track errors better
//         - split into 2 functions

//
// M_DoScreenShot
// Takes a screenshot into the names file

void M_DoScreenShot(const char* fname)
{
	if(I_ScreenShot(fname) != 0)
		Message::Add("M_ScreenShot: Error writing screenshot\n");
}

#ifndef SCREENSHOT_DIR
#define SCREENSHOT_DIR "."
#endif

#ifdef HAVE_LIBSDL2_IMAGE
#define SCREENSHOT_EXT ".png"
#else
#define SCREENSHOT_EXT ".bmp"
#endif

const char* M_CheckWritableDir(const char* dir)
{
	static char* base = nullptr;
	static int base_len = 0;

	const char* result = nullptr;
	int len;

	if(!dir || !(len = strlen(dir)))
	{
		return nullptr;
	}

	if(len + 1 > base_len)
	{
		base_len = len + 1;
		base = static_cast<char*>(Z_Malloc(len + 1));
	}

	if(base)
	{
		strcpy(base, dir);

		if(base[len - 1] != '\\' && base[len - 1] != '/')
			strcat(base, "/");

		if(M_ReadWriteAccess(base))
		{
			base[strlen(base) - 1] = 0;
			result = base;
		}
	}

	return result;
}

void M_ScreenShot()
{
	static int shot;
	char* lbmname = nullptr;
	int startshot;
	const char* shot_dir = nullptr;
	dsda_arg_t* arg;
	int success = 0;

	arg = dsda_Arg(ArgId::Shotdir);
	if(arg->found)
		shot_dir = M_CheckWritableDir(arg->value.v_string);
	if(!shot_dir)
		shot_dir = M_CheckWritableDir(dsda_StringConfig(ConfigId::ScreenshotDir));
	if(!shot_dir)
#ifdef _WIN32
	shot_dir = M_CheckWritableDir(I_ConfigDir());
#else
		shot_dir = (M_WriteAccess(SCREENSHOT_DIR) ? SCREENSHOT_DIR : nullptr);
#endif

	if(shot_dir)
	{
		startshot = shot; // CPhipps - prevent infinite loop

		do
		{
			int size = snprintf(nullptr, 0, "%s/dsda%04d" SCREENSHOT_EXT, shot_dir, shot);
			lbmname = static_cast<char*>(Z_Realloc(lbmname, size + 1));
			snprintf(lbmname, size + 1, "%s/dsda%04d" SCREENSHOT_EXT, shot_dir, shot);
			shot++;
		}
		while(M_FileExists(lbmname) && (shot != startshot) && (shot < 10000));

		if(!M_FileExists(lbmname))
		{
			S_StartVoidSound(gamemode == GameMode::Commercial ? SfxId::Radio : SfxId::Tink);
			M_DoScreenShot(lbmname); // cph
			success = 1;
		}
		Z_Free(lbmname);
		if(success) return;
	}

	Message::Add("M_ScreenShot: Couldn't create screenshot");
	return;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

dboolean M_StringCopy(char* dest, const char* src, size_t dest_size)
{
	size_t len;

	if(dest_size >= 1)
	{
		dest[dest_size - 1] = '\0';
		strncpy(dest, src, dest_size - 1);
	}
	else
	{
		return false;
	}

	len = strlen(dest);
	return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

dboolean M_StringConcat(char* dest, const char* src, size_t dest_size)
{
	size_t offset;

	offset = strlen(dest);
	if(offset > dest_size)
	{
		offset = dest_size;
	}

	return M_StringCopy(dest + offset, src, dest_size - offset);
}

int M_StrToInt(const char* s, int* l)
{
	return (
		(sscanf(s, " 0x%x", l) == 1) ||
		(sscanf(s, " 0X%x", l) == 1) ||
		(sscanf(s, " 0%o", l) == 1) ||
		(sscanf(s, " %d", l) == 1)
	);
}

int M_StrToFloat(const char* s, float* f)
{
	return (
		(sscanf(s, " %f", f) == 1)
	);
}

int M_DoubleToInt(double x)
{
#ifdef __GNUC__
	double tmp = x;
	return (int)tmp;
#else
	return (int)x;
#endif
}

char* M_Strlwr(char* str)
{
	char* p;
	for(p = str; *p; p++) *p = tolower(*p);
	return str;
}

char* M_Strupr(char* str)
{
	char* p;
	for(p = str; *p; p++) *p = toupper(*p);
	return str;
}

char* M_StrRTrim(char* str)
{
	char* end;

	if(str)
	{
		// Trim trailing space
		end = str + strlen(str) - 1;
		while(end > str && isspace(*end))
		{
			end--;
		}

		// Write new null terminator
		*(end + 1) = 0;
	}

	return str;
}

void M_ArrayClear(array_t* data)
{
	data->count = 0;
}

void* M_ArrayGetNewItem(array_t* data, int itemsize)
{
	if(data->count + 1 >= data->capacity)
	{
		data->capacity = (data->capacity ? data->capacity * 2 : 128);
		data->data = static_cast<decltype(data->data)>(Z_Realloc(data->data, data->capacity * itemsize));
	}

	data->count++;

	return (unsigned char*)data->data + (data->count - 1) * itemsize;
}
