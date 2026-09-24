// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA (Command Line) Args

#include <utility>

#include <stdio.h>
#include <string.h>

#include "lprintf.hpp"
#include "z_zone.hpp"

#include "dsda/args.hpp"

int dsda_argc;
char** dsda_argv;

enum struct ArgType : int32_t
{
	Null,
	Int,
	String,
	IntArray,
	StringArray,
};

typedef struct
{
	const char* name;
	const char* alias;
	const char* default_value;
	const char* description;

	ArgType type;

	int lower_limit;
	int upper_limit;
	int min_count;
	int max_count;
} arg_config_t;

#define AT_LEAST_ONE_STRING 0, 0, 1, INT_MAX
#define AT_LEAST_ONE_NONNEGATIVE_INT 0, INT_MAX, 1, INT_MAX
#define EXACT_ARRAY_LENGTH(x) 0, 0, x, x

static arg_config_t arg_config[std::to_underlying(ArgId::Count)] = {
	[std::to_underlying(ArgId::Help)] = {
		"-help", "--help", nullptr,
		"prints out command line argument information",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Iwad)] = {
		"-iwad", nullptr, nullptr,
		"loads the given iwad file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::File)] = {
		"-file", nullptr, nullptr,
		"loads additional wad files",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	},
	[std::to_underlying(ArgId::Deh)] = {
		"-deh", "-bex", nullptr,
		"loads additional deh files",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	},
	[std::to_underlying(ArgId::Loadgame)] = {
		"-loadgame", nullptr, nullptr,
		"loads the given savegame slot",
		ArgType::Int, 0, 118,
	},
	[std::to_underlying(ArgId::Playdemo)] = {
		"-playdemo", nullptr, nullptr,
		"plays the given demo file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Playlump)] = {
		"-playlump", nullptr, nullptr,
		"plays the given internal demo lump (e.g., DEMO1)",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Timedemo)] = {
		"-timedemo", nullptr, nullptr,
		"plays the given demo file as fast as possible, timing the process",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Fastdemo)] = {
		"-fastdemo", nullptr, nullptr,
		"plays the given demo file as fast as possible, skipping some frames",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Record)] = {
		"-record", nullptr, nullptr,
		"records a demo to the given file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Recordfromto)] = {
		"-recordfromto", nullptr, nullptr,
		"plays back the first file while writing to the second",
		ArgType::StringArray, EXACT_ARRAY_LENGTH(2),
	},
	[std::to_underlying(ArgId::FromKeyFrame)] = {
		"-from_key_frame", nullptr, nullptr,
		"restores state and demo buffer from a key frame file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Warp)] = {
		"-warp", nullptr, nullptr,
		"warp to the given episode and / or map",
		ArgType::IntArray, 0, 99, 0, 2,
	},
	[std::to_underlying(ArgId::Skill)] = {
		"-skill", nullptr, nullptr,
		"sets the skill level",
		ArgType::Int, 1, 255,
	},
	[std::to_underlying(ArgId::Uv)] = {
		"-uv", nullptr, nullptr,
		"sets the skill level to 4",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nm)] = {
		"-nm", nullptr, nullptr,
		"sets the skill level to 5",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Episode)] = {
		"-episode", nullptr, nullptr,
		"warp to the first map in the given episode",
		ArgType::Int, 0, 9,
	},
	[std::to_underlying(ArgId::Complevel)] = {
		"-complevel", "-cl", nullptr,
		"sets the compatibility level",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Fast)] = {
		"-fast", nullptr, nullptr,
		"turns on fast monsters",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Respawn)] = {
		"-respawn", nullptr, nullptr,
		"turns on monster respawning",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nomonsters)] = {
		"-nomonsters", "-nomo", nullptr,
		"turns off monster spawning",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Longtics)] = {
		"-longtics", nullptr, nullptr,
		"enables high precision turn angles (in supported formats)",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Shorttics)] = {
		"-shorttics", nullptr, nullptr,
		"restricts turn angles to lower precision",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Heretic)] = {
		"-heretic", nullptr, nullptr,
		"sets the game to heretic",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Hexen)] = {
		"-hexen", nullptr, nullptr,
		"sets the game to hexen",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Class)] = {
		"-class", nullptr, nullptr,
		"sets the player class in hexen",
		ArgType::Int, 0, 2,
	},
	[std::to_underlying(ArgId::Randclass)] = {
		"-randclass", nullptr, nullptr,
		"sets a random player class in hexen deathmatch",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Dsdademo)] = {
		"-dsdademo", nullptr, nullptr,
		"turns on extended demo format (for testing)",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::SoloNet)] = {
		"-solo-net", nullptr, nullptr,
		"play a net game with one player",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::CoopSpawns)] = {
		"-coop_spawns", nullptr, nullptr,
		"play single player with coop thing spawns",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::PistolStart)] = {
		"-pistolstart", "-wandstart", nullptr,
		"automatically pistol start each map",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ChainEpisodes)] = {
		"-chain_episodes", nullptr, nullptr,
		"completing one episode leads to the next without interruption",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Stroller)] = {
		"-stroller", nullptr, nullptr,
		"applies stroller category limitations",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Turbo)] = {
		"-turbo", nullptr, "255",
		"sets player speed percent",
		ArgType::Int, 10, 255,
	},
	[std::to_underlying(ArgId::GameSpeed)] = {
		"-game_speed", nullptr, nullptr,
		"sets game speed percent",
		ArgType::Int, 10, 10000,
	},
	[std::to_underlying(ArgId::Tas)] = {
		"-tas", nullptr, nullptr,
		"lifts strict mode restrictions",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Build)] = {
		"-build", nullptr, nullptr,
		"starts in build mode",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::QuitAfterBruteForce)] = {
		"-quit_after_brute_force", nullptr, nullptr,
		"quits the game when brute force ends",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::FirstInput)] = {
		"-first_input", nullptr, nullptr,
		"builds the first frame F S T",
		ArgType::IntArray, -128, 127, 3, 3
	},
	[std::to_underlying(ArgId::Command)] = {
		"-command", nullptr, nullptr,
		"runs a console command",
		ArgType::String
	},
	[std::to_underlying(ArgId::Skipsec)] = {
		"-skipsec", nullptr, nullptr,
		"skip to the given time (mm:ss or ss) - negative times seek from the end",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Skiptic)] = {
		"-skiptic", nullptr, nullptr,
		"skip to the given tic - negative tics seek from the end",
		ArgType::Int, INT_MIN, INT_MAX,
	},
	[std::to_underlying(ArgId::TrackPacifist)] = {
		"-track_pacifist", nullptr, nullptr,
		"tracks pacifist category restrictions",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Track100k)] = {
		"-track_100k", nullptr, nullptr,
		"tracks when 100% kills is reached",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::TrackReality)] = {
		"-track_reality", nullptr, nullptr,
		"tracks reality and almost reality categories restrictions",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::TimeKeys)] = {
		"-time_keys", nullptr, "105",
		"announces the time when keys are picked up",
		ArgType::Int, 0, 350
	},
	[std::to_underlying(ArgId::TimeUse)] = {
		"-time_use", nullptr, "105",
		"announces the time when the use command is activated",
		ArgType::Int, 0, 350
	},
	[std::to_underlying(ArgId::TimeSecrets)] = {
		"-time_secrets", nullptr, "105",
		"announces the time when a secret is collected",
		ArgType::Int, 0, 350
	},
	[std::to_underlying(ArgId::TimeAll)] = {
		"-time_all", nullptr, "105",
		"announces the time when any -time_* event happens",
		ArgType::Int, 0, 350
	},
	[std::to_underlying(ArgId::TrackPlayer)] = {
		"-track_player", nullptr, nullptr,
		"adds player info to the tracker",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::TrackLine)] = {
		"-track_line", nullptr, nullptr,
		"adds at least one line to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	},
	[std::to_underlying(ArgId::TrackLineDistance)] = {
		"-track_line_distance", nullptr, nullptr,
		"adds at least one line distance to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	},
	[std::to_underlying(ArgId::TrackSector)] = {
		"-track_sector", nullptr, nullptr,
		"adds at least one sector to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	},
	[std::to_underlying(ArgId::TrackMobj)] = {
		"-track_mobj", nullptr, nullptr,
		"adds at least one mobj to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	},
	[std::to_underlying(ArgId::Assign)] = {
		"-assign", nullptr, nullptr,
		"temporarily assign config variables",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	},
	[std::to_underlying(ArgId::Update)] = {
		"-update", nullptr, nullptr,
		"permanently update config variables",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	},
	[std::to_underlying(ArgId::Analysis)] = {
		"-analysis", nullptr, nullptr,
		"writes various data to analysis.txt",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Levelstat)] = {
		"-levelstat", nullptr, nullptr,
		"writes level stats to levelstat.txt",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ExportTextFile)] = {
		"-export_text_file", nullptr, nullptr,
		"export a dsda-format text file template",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::TrackPlayback)] = {
		"-track_playback", nullptr, nullptr,
		"treat demo playback as an attempt for the given split file base",
		ArgType::String,
	},
	[std::to_underlying(ArgId::ExportGhost)] = {
		"-export_ghost", nullptr, nullptr,
		"exports a ghost file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::ImportGhost)] = {
		"-import_ghost", nullptr, nullptr,
		"imports at least one ghost file",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	},
	[std::to_underlying(ArgId::Consoleplayer)] = {
		"-consoleplayer", nullptr, nullptr,
		"sets the console player (for coop playback)",
		ArgType::Int, 0, 7,
	},
	[std::to_underlying(ArgId::Spechit)] = {
		"-spechit", nullptr, nullptr,
		"sets a magic spechit base address for certain overrun demos",
		ArgType::Int, INT_MIN, INT_MAX,
	},
	[std::to_underlying(ArgId::Setmem)] = {
		"-setmem", nullptr, nullptr,
		"sets a magic block of memory for certain overrun demos",
		ArgType::StringArray, 0, 0, 1, 10,
	},
	[std::to_underlying(ArgId::Data)] = {
		"-data", nullptr, nullptr,
		"sets the data directory",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Save)] = {
		"-save", nullptr, nullptr,
		"sets the save directory",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Config)] = {
		"-config", nullptr, nullptr,
		"sets the config file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Hud)] = {
		"-hud", nullptr, nullptr,
		"sets the hud config file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Shotdir)] = {
		"-shotdir", nullptr, nullptr,
		"sets the screenshot directory",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Movie)] = {
		"-movie", nullptr, nullptr,
		"sets the target final level for movie demos (for automatic exit detection)",
		ArgType::Int, 0, 99,
	},
	[std::to_underlying(ArgId::Viddump)] = {
		"-viddump", nullptr, nullptr,
		"dumps a video to the chosen file name",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Dehout)] = {
		"-dehout", "-bexout", nullptr,
		"sets dehacked log file",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Verbose)] = {
		"-verbose", nullptr, nullptr,
		"enable all logging",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Quiet)] = {
		"-quiet", nullptr, nullptr,
		"disable all logging",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::V)] = {
		"-v", nullptr, nullptr,
		"print the version and exit",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Resetgamma)] = {
		"-resetgamma", nullptr, nullptr,
		"reset gamma and exit",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceOldZdoomNodes)] = {
		"-force_old_zdoom_nodes", nullptr, nullptr,
		"force extended (non-gl) zdoom nodes",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Sigsegv)] = {
		"-sigsegv", nullptr, nullptr,
		"disable the SIGSEGV signal handler",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Deathmatch)] = {
		"-deathmatch", nullptr, nullptr,
		"turn on deathmatch mode",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Altdeath)] = {
		"-altdeath", nullptr, nullptr,
		"turn on altdeath mode",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Timer)] = {
		"-timer", nullptr, nullptr,
		"sets the level time limit (in minutes) for deathmatch",
		ArgType::Int, 1, INT_MAX,
	},
	[std::to_underlying(ArgId::Frags)] = {
		"-frags", nullptr, "10",
		"sets the level frag limit for deathmatch",
		ArgType::Int, 1, INT_MAX,
	},
	[std::to_underlying(ArgId::Nosound)] = {
		"-nosound", nullptr, nullptr,
		"turn off sound",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nomusic)] = {
		"-nomusic", nullptr, nullptr,
		"turn off music",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nosfx)] = {
		"-nosfx", nullptr, nullptr,
		"turn off sfx",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nodraw)] = {
		"-nodraw", nullptr, nullptr,
		"turn off drawing",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nodeh)] = {
		"-nodeh", nullptr, nullptr,
		"skip dehacked lumps inside wads",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nomapinfo)] = {
		"-nomapinfo", nullptr, nullptr,
		"skip *MAPINFO lumps",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Noautoload)] = {
		"-noautoload", "-noload", nullptr,
		"ignore autoload files",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nocheats)] = {
		"-nocheats", nullptr, nullptr,
		"ignore dehacked cheats",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nojoy)] = {
		"-nojoy", nullptr, nullptr,
		"disable joystick input",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Nomouse)] = {
		"-nomouse", nullptr, nullptr,
		"disable mouse input",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::NoMessageBox)] = {
		"-no_message_box", nullptr, nullptr,
		"disable message boxes",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Fullscreen)] = {
		"-fullscreen", nullptr, nullptr,
		"temporarily turns on fullscreen mode",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Window)] = {
		"-window", nullptr, nullptr,
		"temporarily turns on windowed mode",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Width)] = {
		"-width", nullptr, nullptr,
		"temporarily sets the resolution width",
		ArgType::Int, 320, INT_MAX,
	},
	[std::to_underlying(ArgId::Height)] = {
		"-height", nullptr, nullptr,
		"temporarily sets the resolution height",
		ArgType::Int, 200, INT_MAX,
	},
	[std::to_underlying(ArgId::Geometry)] = {
		"-geometry", "-geom", nullptr,
		"temporarily sets the resolution and, optionally, the window mode WxH[w|f]",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Vidmode)] = {
		"-vidmode", nullptr, nullptr,
		"temporarily sets the graphics renderer (sw or gl)",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Aspect)] = {
		"-aspect", nullptr, nullptr,
		"sets the fov aspect ratio WxH",
		ArgType::String, 0, 21,
	},
	[std::to_underlying(ArgId::Emulate)] = {
		"-emulate", nullptr, nullptr,
		"emulates errors from a version of prboom+ (a.b.c.d)",
		ArgType::String,
	},
	[std::to_underlying(ArgId::Doom95)] = {
		"-doom95", nullptr, nullptr,
		"use doom95's adjacent sector limit",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::Blockmap)] = {
		"-blockmap", nullptr, nullptr,
		"rebuild the blockmap (ignore BLOCKMAP lump)",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceMonsterAvoidHazards)] = {
		"-force_monster_avoid_hazards", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceRemoveSlimeTrails)] = {
		"-force_remove_slime_trails", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceNoDropoff)] = {
		"-force_no_dropoff", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceTruncatedSectorSpecials)] = {
		"-force_truncated_sector_specials", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceBoomBrainawake)] = {
		"-force_boom_brainawake", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForcePrboomFriction)] = {
		"-force_prboom_friction", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::RejectPadWithFf)] = {
		"-reject_pad_with_ff", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceLxdoomDemoCompatibility)] = {
		"-force_lxdoom_demo_compatibility", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::AllowSsgDirect)] = {
		"-allow_ssg_direct", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::TreatNoClippingThingsAsNotBlocking)] = {
		"-treat_no_clipping_things_as_not_blocking", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceIncorrectProcessingOfRespawnFrameEntry)] = {
		"-force_incorrect_processing_of_respawn_frame_entry", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceCorrectCodeFor3KeysDoorsInMbf)] = {
		"-force_correct_code_for_3_keys_doors_in_mbf", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::UninitializeCrushFieldForStairs)] = {
		"-uninitialize_crush_field_for_stairs", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceBoomFindnexthighestfloor)] = {
		"-force_boom_findnexthighestfloor", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::AllowSkyTransferInBoom)] = {
		"-allow_sky_transfer_in_boom", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ApplyGreenArmorClassToArmorBonuses)] = {
		"-apply_green_armor_class_to_armor_bonuses", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ApplyBlueArmorClassToMegasphere)] = {
		"-apply_blue_armor_class_to_megasphere", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ForceIncorrectBobbingInBoom)] = {
		"-force_incorrect_bobbing_in_boom", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::BoomDehParser)] = {
		"-boom_deh_parser", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::MbfRemoveThinkerInKillmobj)] = {
		"-mbf_remove_thinker_in_killmobj", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::DoNotInheritFriendlynessFlagOnSpawn)] = {
		"-do_not_inherit_friendlyness_flag_on_spawn", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::DoNotUseMisc12FrameParametersInAMushroom)] = {
		"-do_not_use_misc12_frame_parameters_in_a_mushroom", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ApplyMbfCodepointersToAnyComplevel)] = {
		"-apply_mbf_codepointers_to_any_complevel", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
	[std::to_underlying(ArgId::ResetMonsterspawnerParamsAfterLoading)] = {
		"-reset_monsterspawner_params_after_loading", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	},
};

static dsda_arg_t arg_value[std::to_underlying(ArgId::Count)];

static void dsda_ParseIntArg(arg_config_t* config, int* value, const char* param)
{
	if(sscanf(param, "%d", value) != 1)
	{
		if(config->type == ArgType::Int)
			I_Error("%s requires an integer argument", config->name);
		else
			I_Error("%s requires integer arguments", config->name);
	}
	if(*value < config->lower_limit)
		I_Error("%s argument too low (min is %d)", config->name, config->lower_limit);
	if(*value > config->upper_limit)
		I_Error("%s argument too high (max is %d)", config->name, config->upper_limit);
}

static void dsda_ParseStringArg(arg_config_t* config, const char** value, const char* param)
{
	if(config->upper_limit && strlen(param) > config->upper_limit)
		I_Error("%s argument too long (max is %d)", config->name, config->upper_limit);

	*value = param;
}

static void dsda_ValidateArrayArg(arg_config_t* config, dsda_arg_t* arg)
{
	if(config->min_count == config->max_count)
	{
		if(arg->count != config->min_count)
			I_Error("%s requires exactly %d arguments", config->name, config->min_count);
	}
	else
	{
		if(arg->count < config->min_count || arg->count > config->max_count)
		{
			if(config->max_count == INT_MAX)
				I_Error("%s requires at least %d argument(s)", config->name, config->min_count);
			else
				I_Error("%s requires %d to %d argument(s)", config->name, config->min_count, config->max_count);
		}
	}
}

static void dsda_ParseArg(arg_config_t* config, dsda_arg_t* arg, int argv_i)
{
	for(arg->count = 0; argv_i + arg->count < dsda_argc - 1; ++arg->count)
	{
		int x;
		dboolean is_integer;

		is_integer = sscanf(dsda_argv[argv_i + arg->count + 1], "%d", &x);

		if(dsda_argv[argv_i + arg->count + 1][0] == '-' && !is_integer)
			break;

		if(config->type == ArgType::Int || config->type == ArgType::IntArray)
		{
			// only valid integers should be interpreted as arguments
			if(!is_integer)
				I_Error("%s does not accept string arguments", config->name);
		}
	}

	arg->found = true;

	switch(config->type)
	{
		case ArgType::Null:
			if(arg->count)
				I_Error("%s does not take an argument", config->name);

			break;
		case ArgType::Int:
			if(arg->count == 0)
			{
				if(config->default_value)
				{
					dsda_ParseIntArg(config, &arg->value.v_int, config->default_value);

					break;
				}

				I_Error("%s requires an integer argument", config->name);
			}
			if(arg->count > 1)
				I_Error("%s takes only one argument", config->name);

			dsda_ParseIntArg(config, &arg->value.v_int, dsda_argv[argv_i + 1]);

			break;
		case ArgType::String:
			if(arg->count == 0)
			{
				if(config->default_value)
				{
					arg->value.v_string = config->default_value;

					break;
				}

				I_Error("%s requires a string argument", config->name);
			}
			if(arg->count > 1)
				I_Error("%s takes only one argument", config->name);

			dsda_ParseStringArg(config, &arg->value.v_string, dsda_argv[argv_i + 1]);

			break;
		case ArgType::IntArray:
			dsda_ValidateArrayArg(config, arg);

			{
				int i;

				arg->value.v_int_array = static_cast<int *>(Z_Malloc(arg->count * sizeof(int)));

				for(i = argv_i; i < argv_i + arg->count; ++i)
					dsda_ParseIntArg(config, &arg->value.v_int_array[i - argv_i], dsda_argv[i + 1]);
			}

			break;

		case ArgType::StringArray:
			dsda_ValidateArrayArg(config, arg);

			{
				int i;

				arg->value.v_string_array = static_cast<const char **>(Z_Malloc(arg->count * sizeof(char*)));

				for(i = argv_i; i < argv_i + arg->count; ++i)
					dsda_ParseStringArg(config, &arg->value.v_string_array[i - argv_i], dsda_argv[i + 1]);
			}

			break;
	}
}

void dsda_ParseCommandLineArgs(int argc, char** argv)
{
	int i;
	int argv_i;
	arg_config_t* config;

	dsda_argc = argc;
	dsda_argv = argv;

	for(argv_i = dsda_argc - 1; argv_i > 0; --argv_i)
	{
		int x;
		dboolean is_integer;

		is_integer = sscanf(dsda_argv[argv_i], "%d", &x);

		if(dsda_argv[argv_i][0] == '-' && !is_integer)
		{
			for(i = 0; i < std::to_underlying(ArgId::Count); ++i)
			{
				config = &arg_config[i];

				if(
					!strcasecmp(config->name, dsda_argv[argv_i]) ||
					(config->alias && !strcasecmp(config->alias, dsda_argv[argv_i]))
				)
				{
					if(arg_value[i].found)
						lprintf(OutputLevels::Warn, "Warning: ignoring duplicate argument %s\n", config->name);
					else
						dsda_ParseArg(config, &arg_value[i], argv_i);
					break;
				}
			}

			if(i == std::to_underlying(ArgId::Count))
				I_Error("Unknown command line option %s\n", dsda_argv[argv_i]);
		}
	}

	if(dsda_Flag(ArgId::Uv))
		dsda_UpdateIntArg(ArgId::Skill, "4");

	if(dsda_Flag(ArgId::Nm))
		dsda_UpdateIntArg(ArgId::Skill, "5");
}

void dsda_UpdateIntArg(ArgId id, const char* param)
{
	arg_value[std::to_underlying(id)].count = 1;
	arg_value[std::to_underlying(id)].found = true;
	dsda_ParseIntArg(&arg_config[std::to_underlying(id)], &arg_value[std::to_underlying(id)].value.v_int, param);
}

void dsda_UpdateStringArg(ArgId id, const char* param)
{
	param = Z_Strdup(param);
	arg_value[std::to_underlying(id)].count = 1;
	arg_value[std::to_underlying(id)].found = true;
	dsda_ParseStringArg(&arg_config[std::to_underlying(id)], &arg_value[std::to_underlying(id)].value.v_string, param);
}

void dsda_AppendStringArg(ArgId id, const char* param)
{
	if(arg_config[std::to_underlying(id)].type == ArgType::String)
		return dsda_UpdateStringArg(id, param);

	param = Z_Strdup(param);

	++arg_value[std::to_underlying(id)].count;
	arg_value[std::to_underlying(id)].found = true;
	arg_value[std::to_underlying(id)].value.v_string_array =
		static_cast<const char **>(Z_Realloc(arg_value[std::to_underlying(id)].value.v_string_array, arg_value[std::to_underlying(id)].count * sizeof(char*)));

	dsda_ParseStringArg(
		&arg_config[std::to_underlying(id)],
		&arg_value[std::to_underlying(id)].value.v_string_array[arg_value[std::to_underlying(id)].count - 1],
		param
	);

	dsda_ValidateArrayArg(&arg_config[std::to_underlying(id)], &arg_value[std::to_underlying(id)]);
}

dsda_arg_t* dsda_Arg(ArgId id)
{
	return &arg_value[std::to_underlying(id)];
}

void dsda_UpdateFlag(ArgId id, dboolean found)
{
	arg_value[std::to_underlying(id)].found = found;
}

dboolean dsda_Flag(ArgId id)
{
	return arg_value[std::to_underlying(id)].found;
}

int dsda_SimpleIntArg(ArgId id)
{
	if(arg_value[std::to_underlying(id)].found)
		return arg_value[std::to_underlying(id)].value.v_int;

	return 0;
}

void dsda_PrintArgHelp()
{
	int i;

	lprintf(OutputLevels::Info, "\nCommand Line Arguments:\n\n");

	for(i = 0; i < std::to_underlying(ArgId::Count); ++i)
	{
		arg_config_t* config;

		config = &arg_config[i];

		lprintf(OutputLevels::Info, "  %s", config->name);
		if(config->alias)
			lprintf(OutputLevels::Info, " / %s", config->alias);
		lprintf(OutputLevels::Info, ":\n");
		lprintf(OutputLevels::Info, "    %s\n", config->description);
		if(config->default_value)
			lprintf(OutputLevels::Info, "    Default: %s\n", config->default_value);
		lprintf(OutputLevels::Info, "\n");
	}
}
