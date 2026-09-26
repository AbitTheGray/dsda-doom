// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended HUD

#include <algorithm>
#include <utility>

#include <stdio.h>

#include "cpp/EnumArray.hpp"

#include "am_map.hpp"
#include "doomstat.hpp"
#include "hu_stuff.hpp"
#include "lprintf.hpp"
#include "m_file.hpp"
#include "r_main.hpp"
#include "v_video.hpp"
#include "w_wad.hpp"

#include "dsda/args.hpp"
#include "dsda/console.hpp"
#include "dsda/global.hpp"
#include "dsda/hud_components.hpp"
#include "dsda/render_stats.hpp"
#include "dsda/settings.hpp"
#include "dsda/utility.hpp"

#include "exhud.hpp"

typedef struct
{
	void (*init)(int x_offset, int y_offset, PatchTranslation vpt_flags, int* args, int arg_count, void** data);
	void (*update)(void* data);
	void (*draw)(void* data);
	const char* name;
	PatchTranslation default_vpt;
	dboolean strict;
	dboolean off_by_default;
	dboolean intermission;
	dboolean not_level;
	dboolean on;
	dboolean initialized;
	void* data;
} exhud_component_t;

enum struct ExHudComponentId : int32_t
{
	AmmoText,
	ArmorText,
	BigAmmo,
	BigArmor,
	BigArmorText,
	BigArtifact,
	BigHealth,
	BigHealthText,
	CompositeTime,
	HealthText,
	Keys,
	ReadyAmmoText,
	SpeedText,
	StatTotals,
	Tracker,
	WeaponText,
	RenderStats,
	Fps,
	Attempts,
	LocalTime,
	CoordinateDisplay,
	LineDisplay,
	CommandDisplay,
	EventSplit,
	LevelSplits,
	ColorTest,
	FreeText,
	Message,
	SecretMessage,
	MapCoordinates,
	MapTime,
	MapTitle,
	MapTotals,
	Minimap,
	ComponentCount,
};

exhud_component_t components_template[std::to_underlying(ExHudComponentId::ComponentCount)] = {
	[std::to_underlying(ExHudComponentId::AmmoText)] = {
		dsda_InitAmmoTextHC,
		dsda_UpdateAmmoTextHC,
		dsda_DrawAmmoTextHC,
		"ammo_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::ArmorText)] = {
		dsda_InitArmorTextHC,
		dsda_UpdateArmorTextHC,
		dsda_DrawArmorTextHC,
		"armor_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::BigAmmo)] = {
		dsda_InitBigAmmoHC,
		dsda_UpdateBigAmmoHC,
		dsda_DrawBigAmmoHC,
		"big_ammo",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::BigArmor)] = {
		dsda_InitBigArmorHC,
		dsda_UpdateBigArmorHC,
		dsda_DrawBigArmorHC,
		"big_armor",
		.default_vpt = PatchTranslation::ExText | PatchTranslation::NoOffset,
	},
	[std::to_underlying(ExHudComponentId::BigArmorText)] = {
		dsda_InitBigArmorTextHC,
		dsda_UpdateBigArmorTextHC,
		dsda_DrawBigArmorTextHC,
		"big_armor_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::BigArtifact)] = {
		dsda_InitBigArtifactHC,
		dsda_UpdateBigArtifactHC,
		dsda_DrawBigArtifactHC,
		"big_artifact",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::BigHealth)] = {
		dsda_InitBigHealthHC,
		dsda_UpdateBigHealthHC,
		dsda_DrawBigHealthHC,
		"big_health",
		.default_vpt = PatchTranslation::ExText | PatchTranslation::NoOffset,
	},
	[std::to_underlying(ExHudComponentId::BigHealthText)] = {
		dsda_InitBigHealthTextHC,
		dsda_UpdateBigHealthTextHC,
		dsda_DrawBigHealthTextHC,
		"big_health_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::CompositeTime)] = {
		dsda_InitCompositeTimeHC,
		dsda_UpdateCompositeTimeHC,
		dsda_DrawCompositeTimeHC,
		"composite_time",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::HealthText)] = {
		dsda_InitHealthTextHC,
		dsda_UpdateHealthTextHC,
		dsda_DrawHealthTextHC,
		"health_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::Keys)] = {
		dsda_InitKeysHC,
		dsda_UpdateKeysHC,
		dsda_DrawKeysHC,
		"keys",
		.default_vpt = PatchTranslation::ExText | PatchTranslation::NoOffset,
	},
	[std::to_underlying(ExHudComponentId::ReadyAmmoText)] = {
		dsda_InitReadyAmmoTextHC,
		dsda_UpdateReadyAmmoTextHC,
		dsda_DrawReadyAmmoTextHC,
		"ready_ammo_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::SpeedText)] = {
		dsda_InitSpeedTextHC,
		dsda_UpdateSpeedTextHC,
		dsda_DrawSpeedTextHC,
		"speed_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::StatTotals)] = {
		dsda_InitStatTotalsHC,
		dsda_UpdateStatTotalsHC,
		dsda_DrawStatTotalsHC,
		"stat_totals",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::Tracker)] = {
		dsda_InitTrackerHC,
		dsda_UpdateTrackerHC,
		dsda_DrawTrackerHC,
		"tracker",
		.default_vpt = PatchTranslation::ExText,
		.strict = true,
	},
	[std::to_underlying(ExHudComponentId::WeaponText)] = {
		dsda_InitWeaponTextHC,
		dsda_UpdateWeaponTextHC,
		dsda_DrawWeaponTextHC,
		"weapon_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::RenderStats)] = {
		dsda_InitRenderStatsHC,
		dsda_UpdateRenderStatsHC,
		dsda_DrawRenderStatsHC,
		"render_stats",
		.default_vpt = PatchTranslation::ExText,
		.strict = true,
		.off_by_default = true,
	},
	[std::to_underlying(ExHudComponentId::Fps)] = {
		dsda_InitFPSHC,
		dsda_UpdateFPSHC,
		dsda_DrawFPSHC,
		"fps",
		.default_vpt = PatchTranslation::ExText,
		.off_by_default = true,
	},
	[std::to_underlying(ExHudComponentId::Attempts)] = {
		dsda_InitAttemptsHC,
		dsda_UpdateAttemptsHC,
		dsda_DrawAttemptsHC,
		"attempts",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::LocalTime)] = {
		dsda_InitLocalTimeHC,
		dsda_UpdateLocalTimeHC,
		dsda_DrawLocalTimeHC,
		"local_time",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::CoordinateDisplay)] = {
		dsda_InitCoordinateDisplayHC,
		dsda_UpdateCoordinateDisplayHC,
		dsda_DrawCoordinateDisplayHC,
		"coordinate_display",
		.default_vpt = PatchTranslation::ExText,
		.strict = true,
		.off_by_default = true,
	},
	[std::to_underlying(ExHudComponentId::LineDisplay)] = {
		dsda_InitLineDisplayHC,
		dsda_UpdateLineDisplayHC,
		dsda_DrawLineDisplayHC,
		"line_display",
		.default_vpt = PatchTranslation::ExText,
		.strict = true,
		.off_by_default = true,
	},
	[std::to_underlying(ExHudComponentId::CommandDisplay)] = {
		dsda_InitCommandDisplayHC,
		dsda_UpdateCommandDisplayHC,
		dsda_DrawCommandDisplayHC,
		"command_display",
		.default_vpt = PatchTranslation::ExText,
		.strict = true,
		.off_by_default = true,
		.intermission = true,
	},
	[std::to_underlying(ExHudComponentId::EventSplit)] = {
		dsda_InitEventSplitHC,
		dsda_UpdateEventSplitHC,
		dsda_DrawEventSplitHC,
		"event_split",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::LevelSplits)] = {
		dsda_InitLevelSplitsHC,
		dsda_UpdateLevelSplitsHC,
		dsda_DrawLevelSplitsHC,
		"level_splits",
		.default_vpt = PatchTranslation::ExText,
		.intermission = true,
		.not_level = true,
	},
	[std::to_underlying(ExHudComponentId::ColorTest)] = {
		dsda_InitColorTestHC,
		dsda_UpdateColorTestHC,
		dsda_DrawColorTestHC,
		"color_test",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::FreeText)] = {
		dsda_InitFreeTextHC,
		dsda_UpdateFreeTextHC,
		dsda_DrawFreeTextHC,
		"free_text",
		.default_vpt = PatchTranslation::ExText,
	},
	[std::to_underlying(ExHudComponentId::Message)] = {
		dsda_InitMessageHC,
		dsda_UpdateMessageHC,
		dsda_DrawMessageHC,
		"message",
	},
	[std::to_underlying(ExHudComponentId::SecretMessage)] = {
		dsda_InitSecretMessageHC,
		dsda_UpdateSecretMessageHC,
		dsda_DrawSecretMessageHC,
		"secret_message",
	},
	[std::to_underlying(ExHudComponentId::MapCoordinates)] = {
		dsda_InitMapCoordinatesHC,
		dsda_UpdateMapCoordinatesHC,
		dsda_DrawMapCoordinatesHC,
		"map_coordinates",
		.strict = true,
	},
	[std::to_underlying(ExHudComponentId::MapTime)] = {
		dsda_InitMapTimeHC,
		dsda_UpdateMapTimeHC,
		dsda_DrawMapTimeHC,
		"map_time",
	},
	[std::to_underlying(ExHudComponentId::MapTitle)] = {
		dsda_InitMapTitleHC,
		dsda_UpdateMapTitleHC,
		dsda_DrawMapTitleHC,
		"map_title",
	},
	[std::to_underlying(ExHudComponentId::MapTotals)] = {
		dsda_InitMapTotalsHC,
		dsda_UpdateMapTotalsHC,
		dsda_DrawMapTotalsHC,
		"map_totals",
	},
	[std::to_underlying(ExHudComponentId::Minimap)] = {
		dsda_InitMinimapHC,
		dsda_UpdateMinimapHC,
		dsda_DrawMinimapHC,
		"minimap",
		.default_vpt = PatchTranslation::ExText,
		.off_by_default = true,
	},
};

typedef struct
{
	const char* name;
	dboolean status_bar;
	dboolean allow_offset;
	dboolean loaded;
	exhud_component_t components[std::to_underlying(ExHudComponentId::ComponentCount)];
	int y_offset[std::to_underlying(PatchTranslation::AlignMax)];
} dsda_hud_container_t;

enum struct HudVariant : int32_t
{
	Ex,
	Off,
	Full,
	Map,
	Null,
};

static constinit EnumArray<dsda_hud_container_t, HudVariant, HudVariant::Null> containers = {
	{At(HudVariant::Ex), {"ex", true, true}},
	{At(HudVariant::Off), {"off", true, true}},
	{At(HudVariant::Full), {"full", false, true}},
	{At(HudVariant::Map), {"map", true, false}},
};

static dsda_hud_container_t* container;
static exhud_component_t* components;

int dsda_show_render_stats;

extern "C" int dsda_ExHudVerticalOffset()
{
	if(container && container->status_bar)
		return g_st_height;

	return 0;
}

static void dsda_TurnComponentOn(ExHudComponentId id)
{
	if(!components[std::to_underlying(id)].initialized)
		return;

	components[std::to_underlying(id)].on = true;
}

static void dsda_TurnComponentOff(ExHudComponentId id)
{
	components[std::to_underlying(id)].on = false;
}

static void dsda_InitializeComponent(ExHudComponentId id, int x, int y, PatchTranslation vpt, int* args, int arg_count)
{
	components[std::to_underlying(id)].initialized = true;
	components[std::to_underlying(id)].init(x, y, vpt | components[std::to_underlying(id)].default_vpt,
		args, arg_count, &components[std::to_underlying(id)].data);

	if(components[std::to_underlying(id)].off_by_default)
		dsda_TurnComponentOff(id);
	else
		dsda_TurnComponentOn(id);
}

static int dsda_AlignmentToVPT(const char* alignment)
{
	if(!strcmp(alignment, "bottom_left"))
		return std::to_underlying(PatchTranslation::AlignLeftBottom);
	else if(!strcmp(alignment, "bottom_right"))
		return std::to_underlying(PatchTranslation::AlignRightBottom);
	else if(!strcmp(alignment, "top_left"))
		return std::to_underlying(PatchTranslation::AlignLeftTop);
	else if(!strcmp(alignment, "top_right"))
		return std::to_underlying(PatchTranslation::AlignRightTop);
	else if(!strcmp(alignment, "top"))
		return std::to_underlying(PatchTranslation::AlignTop);
	else if(!strcmp(alignment, "bottom"))
		return std::to_underlying(PatchTranslation::AlignBottom);
	else if(!strcmp(alignment, "left"))
		return std::to_underlying(PatchTranslation::AlignLeft);
	else if(!strcmp(alignment, "right"))
		return std::to_underlying(PatchTranslation::AlignRight);
	else if(!strcmp(alignment, "none"))
		return std::to_underlying(PatchTranslation::Stretch);
	else
		return -1;
}

static int dsda_ParseHUDConfig(char** hud_config, int line_i)
{
	int i;
	int count;
	dboolean found;
	const char* line;
	char command[64];
	char args[64];

	for(++line_i; hud_config[line_i]; ++line_i)
	{
		line = hud_config[line_i];

		if(line[0] == '#' || line[0] == '/' || line[0] == '!' || !line[0])
			continue;

		count = sscanf(line, "%63s %63[^\n\r]", command, args);
		if(count != 2)
			I_Error("Invalid hud definition \"%s\"", line);

		// The start of another definition
		if(!strncmp(command, "doom", sizeof(command)) ||
			!strncmp(command, "heretic", sizeof(command)) ||
			!strncmp(command, "hexen", sizeof(command)))
			break;

		found = false;

		for(i = 0; i < std::to_underlying(ExHudComponentId::ComponentCount); ++i)
			if(!strncmp(command, components[i].name, sizeof(command)))
			{
				int x, y;
				int vpt;
				int component_args[6] = {0};
				char alignment[16];

				found = true;

				count = sscanf(args, "%d %d %15s %d %d %d %d %d %d", &x, &y, alignment,
					&component_args[0], &component_args[1],
					&component_args[2], &component_args[3],
					&component_args[4], &component_args[5]);
				if(count < 3)
					I_Error("Invalid hud component args \"%s\"", line);

				vpt = dsda_AlignmentToVPT(alignment);
				if(vpt < 0)
					I_Error("Invalid hud component alignment \"%s\"", line);

				dsda_InitializeComponent(static_cast<ExHudComponentId>(i), x, y, static_cast<PatchTranslation>(vpt), component_args, count - 3);
			}

		if(!strncmp(command, "add_offset", sizeof(command)))
		{
			int offset;
			int vpt;
			char alignment[16];

			found = true;

			if(!container->allow_offset)
				I_Error("The %s config does not support add_offset", container->name);

			count = sscanf(args, "%d %15s", &offset, alignment);
			if(count != 2)
				I_Error("Invalid hud offset \"%s\"", line);

			vpt = dsda_AlignmentToVPT(alignment);
			if(vpt < 0)
			{
				I_Error("Invalid hud offset alignment \"%s\"", line);
				vpt = 0; // TODO: remove after I_Error marked noreturn
			}

			container->y_offset[vpt] = offset;

			if(BOTTOM_ALIGNMENT(static_cast<PatchTranslation>(vpt)))
				container->y_offset[vpt] = -container->y_offset[vpt];
		}

		if(!found)
			I_Error("Invalid hud component \"%s\"", line);
	}

	// roll back the line that wasn't part of this config
	return line_i - 1;
}

static void dsda_ParseHUDConfigs(char** hud_config)
{
	const char* line;
	int line_i;
	const char* target_format;
	char hud_variant[5];

	target_format = hexen ? "hexen %s" : heretic ? "heretic %s" : "doom %s";

	for(line_i = 0; hud_config[line_i]; ++line_i)
	{
		line = hud_config[line_i];

		if(sscanf(line, target_format, hud_variant))
		{
			const auto match = std::ranges::find_if(containers, [&](const dsda_hud_container_t& entry)
			{
				return !strncmp(entry.name, hud_variant, sizeof(hud_variant));
			});

			// Upstream leaves `container` on its null terminator when nothing matches; every reader checks for nullptr instead
			container = match == containers.end() ? nullptr : &*match;

			if(container && !container->loaded)
			{
				container->loaded = true;
				components = container->components;
				memcpy(components, components_template, sizeof(components_template));

				line_i = dsda_ParseHUDConfig(hud_config, line_i);
			}
		}
	}
}

static void dsda_LoadHUDConfig()
{
	DO_ONCE
		char* hud_config = nullptr;
		char** lines;
		dsda_arg_t* arg;
		int lump;
		int length = 0;

		arg = dsda_Arg(ArgId::Hud);
		if(arg->found)
			length = M_ReadFileToString(arg->value.v_string, &hud_config);

		lump = -1;
		while((lump = W_FindNumFromName("DSDAHUD", lump)) >= 0)
		{
			if(!hud_config)
			{
				hud_config = W_ReadLumpToString(lump);
				length = W_LumpLength(lump);
			}
			else
			{
				hud_config = static_cast<char*>(Z_Realloc(hud_config, length + W_LumpLength(lump) + 2));
				hud_config[length++] = '\n'; // in case the file didn't end in a new line
				W_ReadLump(lump, hud_config + length);
				length += W_LumpLength(lump);
				hud_config[length] = '\0';
			}
		}

		if(hud_config)
		{
			lines = dsda_SplitString(hud_config, "\n\r");

			if(lines)
			{
				dsda_ParseHUDConfigs(lines);

				Z_Free(lines);
			}

			Z_Free(hud_config);
		}
	END_ONCE
}

static dboolean dsda_HideHUD()
{
	return dsda_Flag(ArgId::Nodraw) ||
		(R_FullView() && !dsda_IntConfig(ConfigId::HudDisplayed));
}

static dboolean dsda_HUDActive()
{
	return container && container->loaded;
}

static void dsda_ResetActiveHUD()
{
	container = nullptr;
	components = nullptr;
}

static void dsda_UpdateActiveHUD()
{
	container = R_FullView() ? &containers[HudVariant::Full] : dsda_IntConfig(ConfigId::Exhud) ? &containers[HudVariant::Ex] : &containers[HudVariant::Off];

	if(container->loaded)
		components = container->components;
	else
		dsda_ResetActiveHUD();
}

extern "C" void dsda_ResetExTextOffsets();
extern "C" void dsda_UpdateExTextOffset(PatchTranslation flags, int offset);
static void dsda_ResetOffsets()
{

	int i;

	dsda_ResetExTextOffsets();

	for(i = 0; i < std::to_underlying(PatchTranslation::AlignMax); ++i)
		if(container->y_offset[i])
			dsda_UpdateExTextOffset((PatchTranslation)i, container->y_offset[i]);
}

static void dsda_RefreshHUD()
{
	if(!dsda_HUDActive())
		return;

	dsda_ResetOffsets();

	if(dsda_show_render_stats)
		dsda_TurnComponentOn(ExHudComponentId::RenderStats);

	dsda_RefreshExHudFPS();
	dsda_RefreshExHudMinimap();
	dsda_RefreshExHudLevelSplits();
	dsda_RefreshExHudCoordinateDisplay();
	dsda_RefreshExHudCommandDisplay();
	dsda_RefreshMapCoordinates();
	dsda_RefreshMapTotals();
	dsda_RefreshMapTime();
	dsda_RefreshMapTitle();

	if(in_game && gamestate == GameState::Level)
		dsda_UpdateExHud();
}

void dsda_InitExHud()
{
	dsda_ResetActiveHUD();

	if(dsda_HideHUD())
		return;

	dsda_LoadHUDConfig();
	dsda_UpdateActiveHUD();
	dsda_RefreshHUD();
}

static void dsda_UpdateComponents(exhud_component_t* update_components)
{
	int i;

	for(i = 0; i < std::to_underlying(ExHudComponentId::ComponentCount); ++i)
		if(
			update_components[i].on &&
			!update_components[i].not_level &&
			(!update_components[i].strict || !dsda_StrictMode())
		)
			update_components[i].update(update_components[i].data);
}

void dsda_UpdateExHud()
{
	if(automap_stbar)
	{
		if(containers[HudVariant::Map].loaded)
			dsda_UpdateComponents(containers[HudVariant::Map].components);

		return;
	}

	if(!dsda_HUDActive())
		return;

	dsda_UpdateComponents(components);
}

static void dsda_DrawComponents(exhud_component_t* draw_components)
{
	int i;

	for(i = 0; i < std::to_underlying(ExHudComponentId::ComponentCount); ++i)
		if(
			draw_components[i].on &&
			!draw_components[i].not_level &&
			(!draw_components[i].strict || !dsda_StrictMode())
		)
			draw_components[i].draw(draw_components[i].data);
}

int global_patch_top_offset;

void dsda_DrawExHud()
{
	global_patch_top_offset = M_ConsoleOpen() ? dsda_ConsoleHeight() : 0;

	if(automap_stbar)
	{
		if(containers[HudVariant::Map].loaded)
			dsda_DrawComponents(containers[HudVariant::Map].components);
	}
	else if(dsda_HUDActive())
		dsda_DrawComponents(components);

	global_patch_top_offset = 0;
}

void dsda_DrawExIntermission()
{
	int i;

	if(!dsda_HUDActive())
		return;

	for(i = 0; i < std::to_underlying(ExHudComponentId::ComponentCount); ++i)
		if(
			components[i].on &&
			components[i].intermission &&
			(!components[i].strict || !dsda_StrictMode())
		)
			components[i].draw(components[i].data);
}

void dsda_ToggleRenderStats()
{
	dsda_show_render_stats = !dsda_show_render_stats;

	if(!dsda_HUDActive())
		return;

	if(components[std::to_underlying(ExHudComponentId::RenderStats)].on && !dsda_show_render_stats)
		dsda_TurnComponentOff(ExHudComponentId::RenderStats);
	else if(!components[std::to_underlying(ExHudComponentId::RenderStats)].on && dsda_show_render_stats)
	{
		dsda_BeginRenderStats();
		dsda_TurnComponentOn(ExHudComponentId::RenderStats);
	}
}

static void dsda_BasicRefresh(dboolean (*show_component)(), ExHudComponentId id)
{
	if(!dsda_HUDActive())
		return;

	if(show_component())
		dsda_TurnComponentOn(id);
	else
		dsda_TurnComponentOff(id);
}

static void dsda_BasicMapRefresh(dboolean (*show_component)(), ExHudComponentId id)
{
	exhud_component_t* old_components;

	if(!dsda_HUDActive())
		return;

	old_components = components;
	components = containers[HudVariant::Map].components;

	if(show_component())
		dsda_TurnComponentOn(id);
	else
		dsda_TurnComponentOff(id);

	components = old_components;
}

void dsda_RefreshExHudFPS()
{
	dsda_BasicRefresh(dsda_ShowFPS, ExHudComponentId::Fps);
}

void dsda_RefreshExHudMinimap()
{
	if(!dsda_HUDActive())
		return;

	if(dsda_ShowMinimap())
	{
		dsda_TurnComponentOn(ExHudComponentId::Minimap);

		// Need to update the component before calling AM_Start
		if(components[std::to_underlying(ExHudComponentId::Minimap)].initialized)
			components[std::to_underlying(ExHudComponentId::Minimap)].update(components[std::to_underlying(ExHudComponentId::Minimap)].data);

		if(in_game && gamestate == GameState::Level && !automap_full)
			AM_Start(AutomapStart::Minimap);
	}
	else
		dsda_TurnComponentOff(ExHudComponentId::Minimap);
}

void dsda_RefreshExHudLevelSplits()
{
	dsda_BasicRefresh(dsda_ShowLevelSplits, ExHudComponentId::LevelSplits);
}

void dsda_RefreshExHudCoordinateDisplay()
{
	if(!dsda_HUDActive())
		return;

	if(dsda_CoordinateDisplay())
	{
		dsda_TurnComponentOn(ExHudComponentId::CoordinateDisplay);
		dsda_TurnComponentOn(ExHudComponentId::LineDisplay);
	}
	else
	{
		dsda_TurnComponentOff(ExHudComponentId::CoordinateDisplay);
		dsda_TurnComponentOff(ExHudComponentId::LineDisplay);
	}
}

void dsda_RefreshExHudCommandDisplay()
{
	dsda_BasicRefresh(dsda_CommandDisplay, ExHudComponentId::CommandDisplay);
}

void dsda_RefreshMapCoordinates()
{
	dsda_BasicMapRefresh(dsda_MapCoordinates, ExHudComponentId::MapCoordinates);
}

void dsda_RefreshMapTotals()
{
	dsda_BasicMapRefresh(dsda_MapTotals, ExHudComponentId::MapTotals);
}

void dsda_RefreshMapTime()
{
	dsda_BasicMapRefresh(dsda_MapTime, ExHudComponentId::MapTime);
}

void dsda_RefreshMapTitle()
{
	dsda_BasicMapRefresh(dsda_MapTitle, ExHudComponentId::MapTitle);
}
