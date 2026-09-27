// SPDX-License-Identifier: GPL-2.0-or-later

#include <utility>

#include "doomdef.hpp"
#include "cpp/Util.hpp"
#include "dsda/demo.hpp"
#include "r_patch.hpp"
#include "st_stuff.hpp"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <winreg.h>
#endif
#include <SDL_opengl.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "SDL.h"
#ifdef _WIN32
#include <SDL_syswm.h>
#endif

#include "hu_lib.hpp"

#include "doomtype.hpp"
#include "doomstat.hpp"
#include "d_main.hpp"
#include "s_sound.hpp"
#include "i_system.hpp"
#include "i_main.hpp"
#include "i_sound.hpp"
#include "m_menu.hpp"
#include "lprintf.hpp"
#include "m_file.hpp"
#include "i_system.hpp"
#include "p_maputl.hpp"
#include "p_map.hpp"
#include "p_setup.hpp"
#include "i_video.hpp"
#include "info.hpp"
#include "r_main.hpp"
#include "r_things.hpp"
#include "r_sky.hpp"
#include "am_map.hpp"
#include "dsda.hpp"
#include "dsda/settings.hpp"
#include "gl_struct.hpp"
#include "gl_intern.hpp"
#include "g_game.hpp"
#include "d_deh.hpp"
#include "e6y.hpp"
#include "m_file.hpp"
#include "v_video.hpp"

#include "dsda/args.hpp"
#include "dsda/configuration.hpp"
#include "dsda/excmd.hpp"
#include "dsda/key_frame.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/mouse.hpp"
#include "dsda/playback.hpp"
#include "dsda/skip.hpp"
#include "dsda/stretch.hpp"

dboolean wasWiped = false;

int secretfound;
int demo_playerscount;
int demo_tics_count;
char demo_len_st[80];

int mouse_handler;
int gl_render_fov = 90;

camera_t walkcamera;

angle_t viewpitch;
float skyscale;
float screen_skybox_zplane;
float tan_pitch;
float skyUpAngle;
float skyUpShift;
float skyXShift;
float skyYShift;

#ifdef _WIN32
const char* WINError()
{
	static char* WinEBuff = nullptr;
	DWORD err = GetLastError();
	char* ch;

	if(WinEBuff)
	{
		LocalFree(WinEBuff);
	}

	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) & WinEBuff, 0, nullptr) == 0)
	{
		return "Unknown error";
	}

	if((ch = strchr(WinEBuff, '\n')) != 0)
		*ch = 0;
	if((ch = strchr(WinEBuff, '\r')) != 0)
		*ch = 0;

	return WinEBuff;
}
#endif

//--------------------------------------------------

/* ParamsMatchingCheck
 * Conflicting command-line parameters could cause the engine to be confused
 * in some cases. Added checks to prevent this.
 * Example: dsda-doom.exe -record mydemo -playdemo demoname
 */
void ParamsMatchingCheck()
{
	dboolean recording_attempt =
		dsda_Flag(ArgId::Record) ||
		dsda_Flag(ArgId::Recordfromto);

	dboolean playbacking_attempt =
		dsda_Flag(ArgId::Playdemo) ||
		dsda_Flag(ArgId::Timedemo) ||
		dsda_Flag(ArgId::Fastdemo);

	if(recording_attempt && playbacking_attempt)
		I_Error("Params are not matching: Can not being played back and recorded at the same time.");
}

prboom_comp_t prboom_comp[std::to_underlying(PrboomComp::Max)] = {
	{0xffffffff, 0x02020615, 0, std::to_underlying(ArgId::ForceMonsterAvoidHazards)},
	{0x00000000, 0x02040601, 0, std::to_underlying(ArgId::ForceRemoveSlimeTrails)},
	{0x02020200, 0x02040801, 0, std::to_underlying(ArgId::ForceNoDropoff)},
	{0x00000000, 0x02040801, 0, std::to_underlying(ArgId::ForceTruncatedSectorSpecials)},
	{0x00000000, 0x02040802, 0, std::to_underlying(ArgId::ForceBoomBrainawake)},
	{0x00000000, 0x02040802, 0, std::to_underlying(ArgId::ForcePrboomFriction)},
	{0x02020500, 0x02040000, 0, std::to_underlying(ArgId::RejectPadWithFf)},
	{0xffffffff, 0x02040802, 0, std::to_underlying(ArgId::ForceLxdoomDemoCompatibility)},
	{0x00000000, 0x0202061b, 0, std::to_underlying(ArgId::AllowSsgDirect)},
	{0x00000000, 0x02040601, 0, std::to_underlying(ArgId::TreatNoClippingThingsAsNotBlocking)},
	{0x00000000, 0x02040803, 0, std::to_underlying(ArgId::ForceIncorrectProcessingOfRespawnFrameEntry)},
	{0x00000000, 0x02040601, 0, std::to_underlying(ArgId::ForceCorrectCodeFor3KeysDoorsInMbf)},
	{0x00000000, 0x02040601, 0, std::to_underlying(ArgId::UninitializeCrushFieldForStairs)},
	{0x00000000, 0x02040802, 0, std::to_underlying(ArgId::ForceBoomFindnexthighestfloor)},
	{0x00000000, 0x02040802, 0, std::to_underlying(ArgId::AllowSkyTransferInBoom)},
	{0x00000000, 0x02040803, 0, std::to_underlying(ArgId::ApplyGreenArmorClassToArmorBonuses)},
	{0x00000000, 0x02040803, 0, std::to_underlying(ArgId::ApplyBlueArmorClassToMegasphere)},
	{0x02020200, 0x02050003, 0, std::to_underlying(ArgId::ForceIncorrectBobbingInBoom)},
	{0xffffffff, 0x00000000, 0, std::to_underlying(ArgId::BoomDehParser)},
	{0x00000000, 0x02050007, 0, std::to_underlying(ArgId::MbfRemoveThinkerInKillmobj)},
	{0x00000000, 0x02050007, 0, std::to_underlying(ArgId::DoNotInheritFriendlynessFlagOnSpawn)},
	{0x00000000, 0x02050007, 0, std::to_underlying(ArgId::DoNotUseMisc12FrameParametersInAMushroom)},
	{0x00000000, 0x02050102, 0, std::to_underlying(ArgId::ApplyMbfCodepointersToAnyComplevel)},
	{0x00000000, 0x02050104, 0, std::to_underlying(ArgId::ResetMonsterspawnerParamsAfterLoading)},
};

extern "C" void M_ChangeShorttics()
{
	shorttics = dsda_IntConfig(ConfigId::MovementShorttics) || dsda_Flag(ArgId::Shorttics);
}

void e6y_InitCommandLine()
{
	stats_level = dsda_Flag(ArgId::Levelstat);

	if((stroller = dsda_Flag(ArgId::Stroller)))
		dsda_UpdateIntArg(ArgId::Turbo, "50");

	dsda_ReadCommandLine();

	M_ChangeShorttics();
}

int G_ReloadLevel()
{
	int result = false;

	if((gamestate == GameState::Level || gamestate == GameState::Intermission) &&
		allow_incompatibility &&
		menuactive == MenuActive::Inactive)
	{
		G_DeferedInitNew(gameskill, gameepisode, gamemap);
		result = true;
	}

	if(demoplayback)
	{
		dsda_RestartPlayback();
		result = true;
	}

	dsda_WatchLevelReload(&result);

	return result;
}

int G_GotoNextLevel()
{
	int epsd, map;
	int changed = false;

	dsda_NextMap(&epsd, &map);

	if((gamestate == GameState::Level) &&
		allow_incompatibility &&
		menuactive == MenuActive::Inactive)
	{
		G_DeferedInitNew(gameskill, epsd, map);
		changed = true;
	}

	return changed;
}

int G_GotoPrevLevel()
{
	int epsd, map;
	int changed = false;

	dsda_PrevMap(&epsd, &map);

	if((gamestate == GameState::Level) &&
		allow_incompatibility &&
		menuactive == MenuActive::Inactive)
	{
		G_DeferedInitNew(gameskill, epsd, map);
		changed = true;
	}

	return changed;
}

void M_ChangeSpeed()
{
	G_SetSpeed(true);
}

void M_ChangeSkyMode()
{
	int gl_skymode;

	viewpitch = 0;

	R_InitSkyMap();

	gl_skymode = dsda_IntConfig(ConfigId::GlSkymode);

	if(gl_skymode == std::to_underlying(SkyType::Auto))
		gl_drawskys = dsda_FreeAim() ? SkyType::Skydome : SkyType::Standard;
	else
		gl_drawskys = static_cast<SkyType>(gl_skymode);
}

static const int upViewPitchLimit = -ANG90 + (1 << ANGLETOFINESHIFT);
static const int downViewPitchLimit = ANG90 - (1 << ANGLETOFINESHIFT);

void M_ChangeScreenMultipleFactor()
{
	V_ChangeScreenResolution();
}

dboolean HaveMouseLook()
{
	return (viewpitch != 0);
}

void CheckPitch(signed int* pitch)
{
	if(*pitch < upViewPitchLimit)
		*pitch = upViewPitchLimit;

	if(*pitch > downViewPitchLimit)
		*pitch = downViewPitchLimit;

	(*pitch) >>= 16;
	(*pitch) <<= 16;
}

float gl_render_ratio;
float gl_render_fovratio;
float gl_render_fovy = FOV90;
float gl_render_multiplier;

void M_ChangeAspectRatio()
{
	M_ChangeFOV();

	R_SetViewSize();
}

void M_ChangeStretch()
{
	render_stretch_hud = dsda_IntConfig(ConfigId::RenderStretchHud);

	R_SetViewSize();
}

void M_ChangeFOV()
{
	float f1, f2;
	dsda_arg_t* arg;
	int gl_render_aspect_width, gl_render_aspect_height;

	arg = dsda_Arg(ArgId::Aspect);
	if(
		arg->found &&
		sscanf(arg->value.v_string, "%dx%d", &gl_render_aspect_width, &gl_render_aspect_height) == 2
	)
	{
		SetRatio(SCREENWIDTH, SCREENHEIGHT);
		gl_render_fovratio = (float)gl_render_aspect_width / (float)gl_render_aspect_height;
		gl_render_ratio = RMUL * gl_render_fovratio;
		gl_render_multiplier = 64.0f / gl_render_fovratio / RMUL;
	}
	else
	{
		SetRatio(SCREENWIDTH, SCREENHEIGHT);
		gl_render_ratio = gl_ratio;
		gl_render_multiplier = (float)ratio_multiplier;
		if(!tallscreen)
		{
			gl_render_fovratio = 1.6f;
		}
		else
		{
			gl_render_fovratio = gl_render_ratio;
		}
	}

	gl_render_fovy = (float)(2 * RAD2DEG(atan(tan(DEG2RAD(gl_render_fov) / 2) / gl_render_fovratio)));

	screen_skybox_zplane = 320.0f / 2.0f / (float)tan(DEG2RAD(gl_render_fov/2));

	f1 = (float)(320.0f / 200.0f * (float)gl_render_fov / (float)FOV90 - 0.2f);
	f2 = (float)tan(DEG2RAD(gl_render_fovy) / 2.0f);
	if(f1 - f2 < 1)
		skyUpAngle = (float)-RAD2DEG(asin(f1-f2));
	else
		skyUpAngle = -90.0f;

	skyUpShift = (float)tan(DEG2RAD(gl_render_fovy) / 2.0f);

	skyscale = 1.0f / (float)tan(DEG2RAD(gl_render_fov / 2));
}

float viewPitch;

int StepwiseSum(int value, int direction, int minval, int maxval, int defval)
{
	int newvalue;
	int val = (direction > 0 ? value : value - 1);

	if(direction == 0)
		return defval;

	direction = (direction > 0 ? 1 : -1);

	{
		int exp = 1;
		while(exp * 10 <= val)
			exp *= 10;
		newvalue = direction * (val < exp * 5 && exp > 1 ? exp / 2 : exp);
		newvalue = (value + newvalue) / newvalue * newvalue;
	}

	if(newvalue > maxval) newvalue = maxval;
	if(newvalue < minval) newvalue = minval;

	if((value < defval && newvalue > defval) || (value > defval && newvalue < defval))
		newvalue = defval;

	return newvalue;
}

void I_vWarning(const char* message, va_list argList)
{
	char msg[1024];
	vsnprintf(msg, sizeof(msg), message, argList);
	lprintf(OutputLevels::Error, "%s\n", msg);
#ifdef _WIN32
	I_MessageBox(msg, PRB_MB_OK);
#endif
}

int I_MessageBox(const char* text, unsigned int type)
{
#ifdef _WIN32
	int result = PRB_IDCANCEL;

	if(!dsda_Flag(ArgId::NoMessageBox))
	{
		HWND current_hwnd = GetForegroundWindow();
		wchar_t* wtext = ConvertUtf8ToWide(text);
		wchar_t* wpackage = ConvertUtf8ToWide(PROJECT_NAME);
		result = MessageBoxW(GetDesktopWindow(), wtext, wpackage, type | MB_TASKMODAL | MB_TOPMOST);
		Z_Free(wtext);
		Z_Free(wpackage);
		I_SwitchToWindow(current_hwnd);
		return result;
	}
#endif

	return PRB_IDCANCEL;
}

int stats_level;
int stroller;
int numlevels = 0;
int levels_max = 0;
timetable_t* stats = nullptr;

void e6y_G_DoCompleted()
{
	int i;

	dsda_EvaluateSkipModeDoCompleted();

	if(!stats_level)
		return;

	if(numlevels >= levels_max)
	{
		levels_max = levels_max ? levels_max * 2 : 32;
		stats = static_cast<decltype(stats)>(Z_Realloc(stats, sizeof(*stats) * levels_max));
	}

	memset(&stats[numlevels], 0, sizeof(timetable_t));

	FormatTo(stats[numlevels].map, "{}", dsda_MapLumpName(gameepisode, gamemap));

	if(secretexit)
	{
		size_t end_of_string = strlen(stats[numlevels].map);
		if(end_of_string < 15)
			stats[numlevels].map[end_of_string] = 's';
	}

	stats[numlevels].stat[std::to_underlying(TotalsDisplay::Time)] = leveltime;
	stats[numlevels].stat[std::to_underlying(TotalsDisplay::TotalTime)] = totalleveltimes;
	stats[numlevels].stat[std::to_underlying(TotalsDisplay::TotalKill)] = totalkills;
	stats[numlevels].stat[std::to_underlying(TotalsDisplay::TotalItem)] = totalitems;
	stats[numlevels].stat[std::to_underlying(TotalsDisplay::TotalSecret)] = totalsecret;

	for(i = 0; i < g_maxplayers; i++)
	{
		if(playeringame[i])
		{
			stats[numlevels].kill[i] = players[i].killcount - players[i].maxkilldiscount;
			stats[numlevels].item[i] = players[i].itemcount;
			stats[numlevels].secret[i] = players[i].secretcount;

			stats[numlevels].stat[std::to_underlying(TotalsDisplay::AllKill)] += stats[numlevels].kill[i];
			stats[numlevels].stat[std::to_underlying(TotalsDisplay::AllItem)] += stats[numlevels].item[i];
			stats[numlevels].stat[std::to_underlying(TotalsDisplay::AllSecret)] += stats[numlevels].secret[i];
		}
	}

	numlevels++;

	e6y_WriteStats();
}

typedef struct tmpdata_s
{
	char kill[200];
	char item[200];
	char secret[200];
} tmpdata_t;

void e6y_WriteStats()
{
	FILE* f;
	char str[200];
	int i, level, playerscount;
	timetable_t max;
	tmpdata_t tmp;
	tmpdata_t* all;
	size_t allkills_len = 0, allitems_len = 0, allsecrets_len = 0;

	f = M_OpenFile("levelstat.txt", "wb");

	if(f == nullptr)
	{
		lprintf(OutputLevels::Error, "Unable to open levelstat.txt for writing\n");
		return;
	}

	all = static_cast<tmpdata_t*>(Z_Malloc(sizeof(*all) * numlevels));
	memset(&max, 0, sizeof(timetable_t));

	playerscount = 0;
	for(i = 0; i < g_maxplayers; i++)
		if(playeringame[i])
			playerscount++;

	for(level = 0; level < numlevels; level++)
	{
		memset(&tmp, 0, sizeof(tmpdata_t));
		for(i = 0; i < g_maxplayers; i++)
		{
			if(playeringame[i])
			{
				char strtmp[200];
				strcpy(str, tmp.kill[0] == '\0' ? "%s%d" : "%s+%d");

				snprintf(strtmp, sizeof(strtmp), str, tmp.kill, stats[level].kill[i]);
				strcpy(tmp.kill, strtmp);

				snprintf(strtmp, sizeof(strtmp), str, tmp.item, stats[level].item[i]);
				strcpy(tmp.item, strtmp);

				snprintf(strtmp, sizeof(strtmp), str, tmp.secret, stats[level].secret[i]);
				strcpy(tmp.secret, strtmp);
			}
		}
		if(playerscount < 2)
			memset(&all[level], 0, sizeof(tmpdata_t));
		else
		{
			snprintf(all[level].kill, sizeof(all[level].kill), " (%s)", tmp.kill);
			snprintf(all[level].item, sizeof(all[level].item), " (%s)", tmp.item);
			snprintf(all[level].secret, sizeof(all[level].secret), " (%s)", tmp.secret);
		}

		if(strlen(all[level].kill) > allkills_len)
			allkills_len = strlen(all[level].kill);
		if(strlen(all[level].item) > allitems_len)
			allitems_len = strlen(all[level].item);
		if(strlen(all[level].secret) > allsecrets_len)
			allsecrets_len = strlen(all[level].secret);

		for(i = 0; i < std::to_underlying(TotalsDisplay::Max); i++)
			if(stats[level].stat[i] > max.stat[i])
				max.stat[i] = stats[level].stat[i];
	}
	max.stat[std::to_underlying(TotalsDisplay::Time)] = max.stat[std::to_underlying(TotalsDisplay::Time)] / TICRATE / 60;
	max.stat[std::to_underlying(TotalsDisplay::TotalTime)] = max.stat[std::to_underlying(TotalsDisplay::TotalTime)] / TICRATE / 60;

	for(i = 0; i < std::to_underlying(TotalsDisplay::Max); i++)
	{
		snprintf(str, sizeof(str), "%d", max.stat[i]);
		max.stat[i] = strlen(str);
	}

	for(level = 0; level < numlevels; level++)
	{
		snprintf(str, sizeof(str),
			"%%s - %%%dd:%%05.2f (%%%dd:%%02d)  K: %%%dd/%%-%dd%%%lds  I: %%%dd/%%-%dd%%%lds  S: %%%dd/%%-%dd %%%lds\r\n",
			max.stat[std::to_underlying(TotalsDisplay::Time)], max.stat[std::to_underlying(TotalsDisplay::TotalTime)],
			max.stat[std::to_underlying(TotalsDisplay::AllKill)], max.stat[std::to_underlying(TotalsDisplay::TotalKill)], (long)allkills_len,
			max.stat[std::to_underlying(TotalsDisplay::AllItem)], max.stat[std::to_underlying(TotalsDisplay::TotalItem)], (long)allitems_len,
			max.stat[std::to_underlying(TotalsDisplay::AllSecret)], max.stat[std::to_underlying(TotalsDisplay::TotalSecret)], (long)allsecrets_len);

		fprintf(f, str, stats[level].map,
			stats[level].stat[std::to_underlying(TotalsDisplay::Time)] / TICRATE / 60,
			(float)(stats[level].stat[std::to_underlying(TotalsDisplay::Time)] % (60 * TICRATE)) / TICRATE,
			(stats[level].stat[std::to_underlying(TotalsDisplay::TotalTime)]) / TICRATE / 60,
			(stats[level].stat[std::to_underlying(TotalsDisplay::TotalTime)] % (60 * TICRATE)) / TICRATE,
			stats[level].stat[std::to_underlying(TotalsDisplay::AllKill)], stats[level].stat[std::to_underlying(TotalsDisplay::TotalKill)], all[level].kill,
			stats[level].stat[std::to_underlying(TotalsDisplay::AllItem)], stats[level].stat[std::to_underlying(TotalsDisplay::TotalItem)], all[level].item,
			stats[level].stat[std::to_underlying(TotalsDisplay::AllSecret)], stats[level].stat[std::to_underlying(TotalsDisplay::TotalSecret)], all[level].secret
		);
	}

	Z_Free(all);
	fclose(f);
}

//--------------------------------------------------

static double mouse_accelfactor;
static double analog_accelfactor;

void AccelChanging()
{
	int mouse_acceleration;
	int analog_acceleration;

	mouse_acceleration = dsda_IntConfig(ConfigId::MouseAcceleration);
	mouse_accelfactor = (double)mouse_acceleration / 100.0 + 1.0;

	analog_acceleration = dsda_IntConfig(ConfigId::AnalogLookAcceleration);
	analog_accelfactor = (double)analog_acceleration / 100.0 + 1.0;
}

int AccelerateMouse(int val)
{
	if(!mouse_accelfactor)
		return val;

	if(val < 0)
		return -AccelerateMouse(-val);

	return M_DoubleToInt(pow((double)val, mouse_accelfactor));
}

int AccelerateAnalog(float val)
{
	if(!analog_accelfactor)
		return val;

	if(val < 0)
		return -AccelerateAnalog(-val);

	return M_DoubleToInt(pow((double)val, analog_accelfactor));
}

int mlooky = 0;

void e6y_G_Compatibility()
{
	deh_applyCompatibility();

	if(dsda_PlaybackName())
	{
		int i;
		dsda_arg_t* arg;

		//"2.4.8.2" -> 0x02040802
		arg = dsda_Arg(ArgId::Emulate);
		if(arg->found)
		{
			unsigned int emulated_version = 0;
			int b[4], k = 1;
			memset(b, 0, sizeof(b));
			sscanf(arg->value.v_string, "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]);
			for(i = 3; i >= 0; i--, k *= 256)
			{
#ifdef RANGECHECK
				if(b[i] >= 256)
					I_Error("Wrong version number of package: %s", PROJECT_VERSION);
#endif
				emulated_version += b[i] * k;
			}

			for(i = 0; i < std::to_underlying(PrboomComp::Max); i++)
			{
				prboom_comp[i].state =
				(emulated_version >= prboom_comp[i].minver &&
					emulated_version < prboom_comp[i].maxver);
			}
		}

		for(i = 0; i < std::to_underlying(PrboomComp::Max); i++)
		{
			if(dsda_Flag((ArgId)prboom_comp[i].arg_id))
				prboom_comp[i].state = true;
		}
	}

	P_CrossSubsector = P_CrossSubsector_PrBoom;
	if(!prboom_comp[std::to_underlying(PrboomComp::ForceLxdoomDemoCompatibility)].state)
	{
		if(demo_compatibility)
			P_CrossSubsector = P_CrossSubsector_Doom;

		switch(compatibility_level)
		{
			case CompLevel::BoomCompatibility:
			case CompLevel::Boom201:
			case CompLevel::Boom202:
			case CompLevel::Mbf:
			case CompLevel::Mbf21:
				P_CrossSubsector = P_CrossSubsector_Boom;
				break;
		}
	}
}

const char* PathFindFileName(const char* pPath)
{
	const char* pT = pPath;

	if(pPath)
	{
		for(; *pPath; pPath++)
		{
			if((pPath[0] == '\\' || pPath[0] == ':' || pPath[0] == '/')
				&& pPath[1] && pPath[1] != '\\' && pPath[1] != '/')
				pT = pPath + 1;
		}
	}

	return pT;
}

int levelstarttic;

int force_singletics_to = 0;

dboolean HU_MouseOnDemoProgressBar(int* position_x)
{
	int mouse_x;
	int mouse_y;

	if(!dsda_IntConfig(ConfigId::PlaybackMouseControls) ||
		!demoplayback || timingdemo || walkcamera.type || viewport_rect.h <= 0)
		return false;

	dsda_GetMousePosition(&mouse_x, &mouse_y);
	mouse_y = mouse_y * ACTUALHEIGHT / viewport_rect.h;

	if(!mouse_x || mouse_y <= ACTUALHEIGHT - ST_SCALED_HEIGHT / 6)
		return false;

	if(position_x)
		*position_x = mouse_x;

	return true;
}

int HU_DrawDemoProgress(int force)
{
	extern int mouse_hide_timer;
	static unsigned int last_update = 0;
	static int prev_len = -1;

	int len, tics_count, diff;
	unsigned int tick, max_period;

	if(gamestate == GameState::Demoscreen || !demoplayback)
		return false;

	tics_count = demo_tics_count * demo_playerscount;
	len = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * dsda_DemoTic() / tics_count));

	if(!force)
	{
		max_period = ((tics_count - dsda_DemoTic() > 35 * demo_playerscount) ? 500 : 15);

		// Unnecessary updates of progress bar
		// can slow down demo skipping and playback
		tick = SDL_GetTicks();
		if(tick - last_update < max_period)
			return false;
		last_update = tick;

		// Do not update progress bar if difference is small
		diff = len - prev_len;
		if(diff == 0 || diff == 1) // because of static prev_len
			return false;
	}

	prev_len = len;

	if(dsda_IntConfig(ConfigId::PlaybackMouseControls) && mouse_hide_timer > 0 && !timingdemo && !walkcamera.type)
	{
		extern auto_kf_t* auto_key_frames;
		extern int auto_kf_size;
		extern dsda_key_frame_t* playback_key_frames;
		extern int playback_kf_size;
		extern dsda_key_frame_t quick_kf;
		int x;

		int bar_h = ST_SCALED_HEIGHT / 6;
		int bar_y = SCREENHEIGHT - bar_h;
		int inner_h = bar_h * 2 / 3;
		int inner_y = SCREENHEIGHT - bar_h + (bar_h - inner_h) / 2;

		V_FillRect(0, 0, bar_y, len, bar_h, playpal_lightest);
		if(len > 4)
			V_FillRect(0, 2, inner_y, len - 4, inner_h, playpal_darkest);

		// playback key frames in light blue
		for(int i = 0; i < playback_kf_size; i++)
		{
			if(!playback_key_frames[i].buffer) continue;
			x = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * playback_key_frames[i].game_tic_count / tics_count));
			V_FillRect(0, x, inner_y, 1, inner_h, colrngs[std::to_underlying(ColorRange::Lightblue)][playpal_lightest]);
		}

		// rewind key frames in green
		for(int i = 0; i < auto_kf_size; i++)
		{
			if(!auto_key_frames[i].kf.buffer) continue;
			x = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * auto_key_frames[i].kf.game_tic_count / tics_count));
			V_FillRect(0, x, inner_y, 1, inner_h, colrngs[std::to_underlying(ColorRange::Green)][playpal_lightest]);
		}

		// quick key frame in red
		if(quick_kf.buffer)
		{
			x = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * quick_kf.game_tic_count / tics_count));
			V_FillRect(0, x, inner_y, 1, inner_h, colrngs[std::to_underlying(ColorRange::Red)][playpal_lightest]);
		}

		V_FillRect(0, len - 1, bar_y, 2, bar_h, playpal_lightest);

		return true;
	}
	else if(dsda_IntConfig(ConfigId::HudaddDemoprogressbar))
	{
		V_FillRect(0, 0, SCREENHEIGHT - 4, len - 0, 4, playpal_lightest);
		if(len > 4)
			V_FillRect(0, 2, SCREENHEIGHT - 3, len - 4, 2, playpal_darkest);

		return true;
	}

	return false;
}

#ifdef _WIN32
int GetFullPath(const char* FileName, const char* ext, char* Buffer, size_t BufferLength)
{
	int i, Result;
	char* p;
	char dir[PATH_MAX];

	for(i = 0; i < 3; i++)
	{
		switch(i)
		{
			case 0:
				M_getcwd(dir, sizeof(dir));
				break;
			case 1:
				if(!M_getenv("DOOMWADDIR"))
					continue;
				strcpy(dir, M_getenv("DOOMWADDIR"));
				break;
			case 2:
				strcpy(dir, I_ConfigDir());
				break;
		}

		Result = SearchPath(dir, FileName, ext, BufferLength, Buffer, &p);
		if(Result)
			return Result;
	}

	return false;
}
#endif
