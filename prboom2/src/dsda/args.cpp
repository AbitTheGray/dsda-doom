// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA (Command Line) Args

#include <algorithm>
#include <utility>

#include <stdio.h>
#include <string.h>

#include "cpp/EnumArray.hpp"

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

static constinit EnumArray<arg_config_t, EnumCount<ArgId>> arg_config = {
	{At(ArgId::Help), {
		"-help", "--help", nullptr,
		"prints out command line argument information",
		ArgType::Null,
	}},
	{At(ArgId::Iwad), {
		"-iwad", nullptr, nullptr,
		"loads the given iwad file",
		ArgType::String,
	}},
	{At(ArgId::File), {
		"-file", nullptr, nullptr,
		"loads additional wad files",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	}},
	{At(ArgId::Deh), {
		"-deh", "-bex", nullptr,
		"loads additional deh files",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	}},
	{At(ArgId::Loadgame), {
		"-loadgame", nullptr, nullptr,
		"loads the given savegame slot",
		ArgType::Int, 0, 118,
	}},
	{At(ArgId::Playdemo), {
		"-playdemo", nullptr, nullptr,
		"plays the given demo file",
		ArgType::String,
	}},
	{At(ArgId::Playlump), {
		"-playlump", nullptr, nullptr,
		"plays the given internal demo lump (e.g., DEMO1)",
		ArgType::String,
	}},
	{At(ArgId::Timedemo), {
		"-timedemo", nullptr, nullptr,
		"plays the given demo file as fast as possible, timing the process",
		ArgType::String,
	}},
	{At(ArgId::Fastdemo), {
		"-fastdemo", nullptr, nullptr,
		"plays the given demo file as fast as possible, skipping some frames",
		ArgType::String,
	}},
	{At(ArgId::Record), {
		"-record", nullptr, nullptr,
		"records a demo to the given file",
		ArgType::String,
	}},
	{At(ArgId::Recordfromto), {
		"-recordfromto", nullptr, nullptr,
		"plays back the first file while writing to the second",
		ArgType::StringArray, EXACT_ARRAY_LENGTH(2),
	}},
	{At(ArgId::FromKeyFrame), {
		"-from_key_frame", nullptr, nullptr,
		"restores state and demo buffer from a key frame file",
		ArgType::String,
	}},
	{At(ArgId::Warp), {
		"-warp", nullptr, nullptr,
		"warp to the given episode and / or map",
		ArgType::IntArray, 0, 99, 0, 2,
	}},
	{At(ArgId::Skill), {
		"-skill", nullptr, nullptr,
		"sets the skill level",
		ArgType::Int, 1, 255,
	}},
	{At(ArgId::Uv), {
		"-uv", nullptr, nullptr,
		"sets the skill level to 4",
		ArgType::Null,
	}},
	{At(ArgId::Nm), {
		"-nm", nullptr, nullptr,
		"sets the skill level to 5",
		ArgType::Null,
	}},
	{At(ArgId::Episode), {
		"-episode", nullptr, nullptr,
		"warp to the first map in the given episode",
		ArgType::Int, 0, 9,
	}},
	{At(ArgId::Complevel), {
		"-complevel", "-cl", nullptr,
		"sets the compatibility level",
		ArgType::String,
	}},
	{At(ArgId::Fast), {
		"-fast", nullptr, nullptr,
		"turns on fast monsters",
		ArgType::Null,
	}},
	{At(ArgId::Respawn), {
		"-respawn", nullptr, nullptr,
		"turns on monster respawning",
		ArgType::Null,
	}},
	{At(ArgId::Nomonsters), {
		"-nomonsters", "-nomo", nullptr,
		"turns off monster spawning",
		ArgType::Null,
	}},
	{At(ArgId::Longtics), {
		"-longtics", nullptr, nullptr,
		"enables high precision turn angles (in supported formats)",
		ArgType::Null,
	}},
	{At(ArgId::Shorttics), {
		"-shorttics", nullptr, nullptr,
		"restricts turn angles to lower precision",
		ArgType::Null,
	}},
	{At(ArgId::Heretic), {
		"-heretic", nullptr, nullptr,
		"sets the game to heretic",
		ArgType::Null,
	}},
	{At(ArgId::Hexen), {
		"-hexen", nullptr, nullptr,
		"sets the game to hexen",
		ArgType::Null,
	}},
	{At(ArgId::Class), {
		"-class", nullptr, nullptr,
		"sets the player class in hexen",
		ArgType::Int, 0, 2,
	}},
	{At(ArgId::Randclass), {
		"-randclass", nullptr, nullptr,
		"sets a random player class in hexen deathmatch",
		ArgType::Null,
	}},
	{At(ArgId::Dsdademo), {
		"-dsdademo", nullptr, nullptr,
		"turns on extended demo format (for testing)",
		ArgType::Null,
	}},
	{At(ArgId::SoloNet), {
		"-solo-net", nullptr, nullptr,
		"play a net game with one player",
		ArgType::Null,
	}},
	{At(ArgId::CoopSpawns), {
		"-coop_spawns", nullptr, nullptr,
		"play single player with coop thing spawns",
		ArgType::Null,
	}},
	{At(ArgId::PistolStart), {
		"-pistolstart", "-wandstart", nullptr,
		"automatically pistol start each map",
		ArgType::Null,
	}},
	{At(ArgId::ChainEpisodes), {
		"-chain_episodes", nullptr, nullptr,
		"completing one episode leads to the next without interruption",
		ArgType::Null,
	}},
	{At(ArgId::Stroller), {
		"-stroller", nullptr, nullptr,
		"applies stroller category limitations",
		ArgType::Null,
	}},
	{At(ArgId::Turbo), {
		"-turbo", nullptr, "255",
		"sets player speed percent",
		ArgType::Int, 10, 255,
	}},
	{At(ArgId::GameSpeed), {
		"-game_speed", nullptr, nullptr,
		"sets game speed percent",
		ArgType::Int, 10, 10000,
	}},
	{At(ArgId::Tas), {
		"-tas", nullptr, nullptr,
		"lifts strict mode restrictions",
		ArgType::Null,
	}},
	{At(ArgId::Build), {
		"-build", nullptr, nullptr,
		"starts in build mode",
		ArgType::Null,
	}},
	{At(ArgId::QuitAfterBruteForce), {
		"-quit_after_brute_force", nullptr, nullptr,
		"quits the game when brute force ends",
		ArgType::Null,
	}},
	{At(ArgId::FirstInput), {
		"-first_input", nullptr, nullptr,
		"builds the first frame F S T",
		ArgType::IntArray, -128, 127, 3, 3
	}},
	{At(ArgId::Command), {
		"-command", nullptr, nullptr,
		"runs a console command",
		ArgType::String
	}},
	{At(ArgId::Skipsec), {
		"-skipsec", nullptr, nullptr,
		"skip to the given time (mm:ss or ss) - negative times seek from the end",
		ArgType::String,
	}},
	{At(ArgId::Skiptic), {
		"-skiptic", nullptr, nullptr,
		"skip to the given tic - negative tics seek from the end",
		ArgType::Int, INT_MIN, INT_MAX,
	}},
	{At(ArgId::TrackPacifist), {
		"-track_pacifist", nullptr, nullptr,
		"tracks pacifist category restrictions",
		ArgType::Null,
	}},
	{At(ArgId::Track100k), {
		"-track_100k", nullptr, nullptr,
		"tracks when 100% kills is reached",
		ArgType::Null,
	}},
	{At(ArgId::TrackReality), {
		"-track_reality", nullptr, nullptr,
		"tracks reality and almost reality categories restrictions",
		ArgType::Null,
	}},
	{At(ArgId::TimeKeys), {
		"-time_keys", nullptr, "105",
		"announces the time when keys are picked up",
		ArgType::Int, 0, 350
	}},
	{At(ArgId::TimeUse), {
		"-time_use", nullptr, "105",
		"announces the time when the use command is activated",
		ArgType::Int, 0, 350
	}},
	{At(ArgId::TimeSecrets), {
		"-time_secrets", nullptr, "105",
		"announces the time when a secret is collected",
		ArgType::Int, 0, 350
	}},
	{At(ArgId::TimeAll), {
		"-time_all", nullptr, "105",
		"announces the time when any -time_* event happens",
		ArgType::Int, 0, 350
	}},
	{At(ArgId::TrackPlayer), {
		"-track_player", nullptr, nullptr,
		"adds player info to the tracker",
		ArgType::Null,
	}},
	{At(ArgId::TrackLine), {
		"-track_line", nullptr, nullptr,
		"adds at least one line to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	}},
	{At(ArgId::TrackLineDistance), {
		"-track_line_distance", nullptr, nullptr,
		"adds at least one line distance to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	}},
	{At(ArgId::TrackSector), {
		"-track_sector", nullptr, nullptr,
		"adds at least one sector to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	}},
	{At(ArgId::TrackMobj), {
		"-track_mobj", nullptr, nullptr,
		"adds at least one mobj to the tracker",
		ArgType::IntArray, AT_LEAST_ONE_NONNEGATIVE_INT,
	}},
	{At(ArgId::Assign), {
		"-assign", nullptr, nullptr,
		"temporarily assign config variables",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	}},
	{At(ArgId::Update), {
		"-update", nullptr, nullptr,
		"permanently update config variables",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	}},
	{At(ArgId::Analysis), {
		"-analysis", nullptr, nullptr,
		"writes various data to analysis.txt",
		ArgType::Null,
	}},
	{At(ArgId::Levelstat), {
		"-levelstat", nullptr, nullptr,
		"writes level stats to levelstat.txt",
		ArgType::Null,
	}},
	{At(ArgId::ExportTextFile), {
		"-export_text_file", nullptr, nullptr,
		"export a dsda-format text file template",
		ArgType::Null,
	}},
	{At(ArgId::TrackPlayback), {
		"-track_playback", nullptr, nullptr,
		"treat demo playback as an attempt for the given split file base",
		ArgType::String,
	}},
	{At(ArgId::ExportGhost), {
		"-export_ghost", nullptr, nullptr,
		"exports a ghost file",
		ArgType::String,
	}},
	{At(ArgId::ImportGhost), {
		"-import_ghost", nullptr, nullptr,
		"imports at least one ghost file",
		ArgType::StringArray, AT_LEAST_ONE_STRING,
	}},
	{At(ArgId::Consoleplayer), {
		"-consoleplayer", nullptr, nullptr,
		"sets the console player (for coop playback)",
		ArgType::Int, 0, 7,
	}},
	{At(ArgId::Spechit), {
		"-spechit", nullptr, nullptr,
		"sets a magic spechit base address for certain overrun demos",
		ArgType::Int, INT_MIN, INT_MAX,
	}},
	{At(ArgId::Setmem), {
		"-setmem", nullptr, nullptr,
		"sets a magic block of memory for certain overrun demos",
		ArgType::StringArray, 0, 0, 1, 10,
	}},
	{At(ArgId::Data), {
		"-data", nullptr, nullptr,
		"sets the data directory",
		ArgType::String,
	}},
	{At(ArgId::Save), {
		"-save", nullptr, nullptr,
		"sets the save directory",
		ArgType::String,
	}},
	{At(ArgId::Config), {
		"-config", nullptr, nullptr,
		"sets the config file",
		ArgType::String,
	}},
	{At(ArgId::Hud), {
		"-hud", nullptr, nullptr,
		"sets the hud config file",
		ArgType::String,
	}},
	{At(ArgId::Shotdir), {
		"-shotdir", nullptr, nullptr,
		"sets the screenshot directory",
		ArgType::String,
	}},
	{At(ArgId::Movie), {
		"-movie", nullptr, nullptr,
		"sets the target final level for movie demos (for automatic exit detection)",
		ArgType::Int, 0, 99,
	}},
	{At(ArgId::Viddump), {
		"-viddump", nullptr, nullptr,
		"dumps a video to the chosen file name",
		ArgType::String,
	}},
	{At(ArgId::Dehout), {
		"-dehout", "-bexout", nullptr,
		"sets dehacked log file",
		ArgType::String,
	}},
	{At(ArgId::Verbose), {
		"-verbose", nullptr, nullptr,
		"enable all logging",
		ArgType::Null,
	}},
	{At(ArgId::Quiet), {
		"-quiet", nullptr, nullptr,
		"disable all logging",
		ArgType::Null,
	}},
	{At(ArgId::V), {
		"-v", nullptr, nullptr,
		"print the version and exit",
		ArgType::Null,
	}},
	{At(ArgId::Resetgamma), {
		"-resetgamma", nullptr, nullptr,
		"reset gamma and exit",
		ArgType::Null,
	}},
	{At(ArgId::ForceOldZdoomNodes), {
		"-force_old_zdoom_nodes", nullptr, nullptr,
		"force extended (non-gl) zdoom nodes",
		ArgType::Null,
	}},
	{At(ArgId::Sigsegv), {
		"-sigsegv", nullptr, nullptr,
		"disable the SIGSEGV signal handler",
		ArgType::Null,
	}},
	{At(ArgId::Deathmatch), {
		"-deathmatch", nullptr, nullptr,
		"turn on deathmatch mode",
		ArgType::Null,
	}},
	{At(ArgId::Altdeath), {
		"-altdeath", nullptr, nullptr,
		"turn on altdeath mode",
		ArgType::Null,
	}},
	{At(ArgId::Timer), {
		"-timer", nullptr, nullptr,
		"sets the level time limit (in minutes) for deathmatch",
		ArgType::Int, 1, INT_MAX,
	}},
	{At(ArgId::Frags), {
		"-frags", nullptr, "10",
		"sets the level frag limit for deathmatch",
		ArgType::Int, 1, INT_MAX,
	}},
	{At(ArgId::Nosound), {
		"-nosound", nullptr, nullptr,
		"turn off sound",
		ArgType::Null,
	}},
	{At(ArgId::Nomusic), {
		"-nomusic", nullptr, nullptr,
		"turn off music",
		ArgType::Null,
	}},
	{At(ArgId::Nosfx), {
		"-nosfx", nullptr, nullptr,
		"turn off sfx",
		ArgType::Null,
	}},
	{At(ArgId::Nodraw), {
		"-nodraw", nullptr, nullptr,
		"turn off drawing",
		ArgType::Null,
	}},
	{At(ArgId::Nodeh), {
		"-nodeh", nullptr, nullptr,
		"skip dehacked lumps inside wads",
		ArgType::Null,
	}},
	{At(ArgId::Nomapinfo), {
		"-nomapinfo", nullptr, nullptr,
		"skip *MAPINFO lumps",
		ArgType::Null,
	}},
	{At(ArgId::Noautoload), {
		"-noautoload", "-noload", nullptr,
		"ignore autoload files",
		ArgType::Null,
	}},
	{At(ArgId::Nocheats), {
		"-nocheats", nullptr, nullptr,
		"ignore dehacked cheats",
		ArgType::Null,
	}},
	{At(ArgId::Nojoy), {
		"-nojoy", nullptr, nullptr,
		"disable joystick input",
		ArgType::Null,
	}},
	{At(ArgId::Nomouse), {
		"-nomouse", nullptr, nullptr,
		"disable mouse input",
		ArgType::Null,
	}},
	{At(ArgId::NoMessageBox), {
		"-no_message_box", nullptr, nullptr,
		"disable message boxes",
		ArgType::Null,
	}},
	{At(ArgId::Fullscreen), {
		"-fullscreen", nullptr, nullptr,
		"temporarily turns on fullscreen mode",
		ArgType::Null,
	}},
	{At(ArgId::Window), {
		"-window", nullptr, nullptr,
		"temporarily turns on windowed mode",
		ArgType::Null,
	}},
	{At(ArgId::Width), {
		"-width", nullptr, nullptr,
		"temporarily sets the resolution width",
		ArgType::Int, 320, INT_MAX,
	}},
	{At(ArgId::Height), {
		"-height", nullptr, nullptr,
		"temporarily sets the resolution height",
		ArgType::Int, 200, INT_MAX,
	}},
	{At(ArgId::Geometry), {
		"-geometry", "-geom", nullptr,
		"temporarily sets the resolution and, optionally, the window mode WxH[w|f]",
		ArgType::String,
	}},
	{At(ArgId::Vidmode), {
		"-vidmode", nullptr, nullptr,
		"temporarily sets the graphics renderer (sw or gl)",
		ArgType::String,
	}},
	{At(ArgId::Aspect), {
		"-aspect", nullptr, nullptr,
		"sets the fov aspect ratio WxH",
		ArgType::String, 0, 21,
	}},
	{At(ArgId::Emulate), {
		"-emulate", nullptr, nullptr,
		"emulates errors from a version of prboom+ (a.b.c.d)",
		ArgType::String,
	}},
	{At(ArgId::Doom95), {
		"-doom95", nullptr, nullptr,
		"use doom95's adjacent sector limit",
		ArgType::Null,
	}},
	{At(ArgId::Blockmap), {
		"-blockmap", nullptr, nullptr,
		"rebuild the blockmap (ignore BLOCKMAP lump)",
		ArgType::Null,
	}},
	{At(ArgId::ForceMonsterAvoidHazards), {
		"-force_monster_avoid_hazards", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceRemoveSlimeTrails), {
		"-force_remove_slime_trails", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceNoDropoff), {
		"-force_no_dropoff", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceTruncatedSectorSpecials), {
		"-force_truncated_sector_specials", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceBoomBrainawake), {
		"-force_boom_brainawake", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForcePrboomFriction), {
		"-force_prboom_friction", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::RejectPadWithFf), {
		"-reject_pad_with_ff", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceLxdoomDemoCompatibility), {
		"-force_lxdoom_demo_compatibility", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::AllowSsgDirect), {
		"-allow_ssg_direct", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::TreatNoClippingThingsAsNotBlocking), {
		"-treat_no_clipping_things_as_not_blocking", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceIncorrectProcessingOfRespawnFrameEntry), {
		"-force_incorrect_processing_of_respawn_frame_entry", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceCorrectCodeFor3KeysDoorsInMbf), {
		"-force_correct_code_for_3_keys_doors_in_mbf", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::UninitializeCrushFieldForStairs), {
		"-uninitialize_crush_field_for_stairs", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceBoomFindnexthighestfloor), {
		"-force_boom_findnexthighestfloor", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::AllowSkyTransferInBoom), {
		"-allow_sky_transfer_in_boom", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ApplyGreenArmorClassToArmorBonuses), {
		"-apply_green_armor_class_to_armor_bonuses", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ApplyBlueArmorClassToMegasphere), {
		"-apply_blue_armor_class_to_megasphere", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ForceIncorrectBobbingInBoom), {
		"-force_incorrect_bobbing_in_boom", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::BoomDehParser), {
		"-boom_deh_parser", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::MbfRemoveThinkerInKillmobj), {
		"-mbf_remove_thinker_in_killmobj", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::DoNotInheritFriendlynessFlagOnSpawn), {
		"-do_not_inherit_friendlyness_flag_on_spawn", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::DoNotUseMisc12FrameParametersInAMushroom), {
		"-do_not_use_misc12_frame_parameters_in_a_mushroom", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ApplyMbfCodepointersToAnyComplevel), {
		"-apply_mbf_codepointers_to_any_complevel", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
	{At(ArgId::ResetMonsterspawnerParamsAfterLoading), {
		"-reset_monsterspawner_params_after_loading", nullptr, nullptr,
		"sets a special flag to compensate for sync errors in certain demos",
		ArgType::Null,
	}},
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
	int argv_i;

	dsda_argc = argc;
	dsda_argv = argv;

	for(argv_i = dsda_argc - 1; argv_i > 0; --argv_i)
	{
		int x;
		dboolean is_integer;

		is_integer = sscanf(dsda_argv[argv_i], "%d", &x);

		if(dsda_argv[argv_i][0] == '-' && !is_integer)
		{
			const auto id = std::ranges::find_if(arg_config.Keys(), [&](const ArgId key)
			{
				const arg_config_t& config = arg_config[key];
				return !strcasecmp(config.name, dsda_argv[argv_i]) ||
					(config.alias && !strcasecmp(config.alias, dsda_argv[argv_i]));
			});

			if(id == arg_config.Keys().end())
				I_Error("Unknown command line option %s\n", dsda_argv[argv_i]);

			arg_config_t* config = &arg_config[*id];

			if(arg_value[std::to_underlying(*id)].found)
				lprintf(OutputLevels::Warn, "Warning: ignoring duplicate argument %s\n", config->name);
			else
				dsda_ParseArg(config, &arg_value[std::to_underlying(*id)], argv_i);
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
	dsda_ParseIntArg(&arg_config[id], &arg_value[std::to_underlying(id)].value.v_int, param);
}

void dsda_UpdateStringArg(ArgId id, const char* param)
{
	param = Z_Strdup(param);
	arg_value[std::to_underlying(id)].count = 1;
	arg_value[std::to_underlying(id)].found = true;
	dsda_ParseStringArg(&arg_config[id], &arg_value[std::to_underlying(id)].value.v_string, param);
}

void dsda_AppendStringArg(ArgId id, const char* param)
{
	if(arg_config[id].type == ArgType::String)
		return dsda_UpdateStringArg(id, param);

	param = Z_Strdup(param);

	++arg_value[std::to_underlying(id)].count;
	arg_value[std::to_underlying(id)].found = true;
	arg_value[std::to_underlying(id)].value.v_string_array =
		static_cast<const char **>(Z_Realloc(arg_value[std::to_underlying(id)].value.v_string_array, arg_value[std::to_underlying(id)].count * sizeof(char*)));

	dsda_ParseStringArg(
		&arg_config[id],
		&arg_value[std::to_underlying(id)].value.v_string_array[arg_value[std::to_underlying(id)].count - 1],
		param
	);

	dsda_ValidateArrayArg(&arg_config[id], &arg_value[std::to_underlying(id)]);
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
	lprintf(OutputLevels::Info, "\nCommand Line Arguments:\n\n");

	for(const arg_config_t& config_entry : arg_config)
	{
		const arg_config_t* config = &config_entry;

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
