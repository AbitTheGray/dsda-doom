// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Uncapped framerate stuff
 */

#include "doomstat.hpp"
#include "m_random.hpp"
#include "r_defs.hpp"
#include "r_state.hpp"
#include "p_spec.hpp"
#include "smooth.hpp"
#include "r_fps.hpp"
#include "i_system.hpp"
#include "i_capture.hpp"
#include "e6y.hpp"

#include "dsda/aim.hpp"
#include "dsda/build.hpp"
#include "dsda/configuration.hpp"
#include "dsda/pause.hpp"
#include "dsda/scroll.hpp"
#include "dsda/settings.hpp"

#include "hexen/a_action.hpp"

int movement_smooth;
dboolean isExtraDDisplay = false;

enum struct InterpolationType : int32_t
{
	SectorFloor,
	SectorCeiling,
	WallPanning,
	FloorPanning,
	CeilingPanning
};

typedef struct
{
	InterpolationType type;
	void* address;
} interpolation_t;

static int numinterpolations = 0;

tic_vars_t tic_vars;

static void R_DoAnInterpolation(int i, fixed_t smoothratio);

extern "C" void D_Display(fixed_t frac);

void M_ChangeUncappedFrameRate()
{
	if(capturing_video)
		movement_smooth = true;
	else
		movement_smooth = (singletics ? false : dsda_IntConfig(ConfigId::UncappedFramerate));
}

typedef fixed_t fixed2_t[2];
static fixed2_t* oldipos;
static fixed2_t* bakipos;
static interpolation_t* curipos;

static dboolean NoInterpolateView;
static dboolean didInterp;
dboolean WasRenderedInTryRunTics;

dboolean R_ViewInterpolation()
{
	return !dsda_Paused() && movement_smooth;
}

void R_InterpolateView(player_t* player, fixed_t frac)
{
	static mobj_t* oviewer;
	int quake_intensity;

	dboolean NoInterpolate = dsda_CameraPaused() || dsda_PausedViaMenu();

	quake_intensity = dsda_IntConfig(ConfigId::QuakeIntensity);

	viewplayer = player;

	if(player->mo != oviewer || NoInterpolate)
	{
		R_ResetViewInterpolation();
		oviewer = player->mo;
	}

	if(NoInterpolate)
		frac = FRACUNIT;
	tic_vars.frac = frac;

	if(movement_smooth)
	{
		if(NoInterpolateView)
		{
			NoInterpolateView = false;

			player->prev_viewz = player->viewz;
			player->prev_viewangle = player->mo->angle;
			player->prev_viewpitch = dsda_PlayerPitch(player);

			P_ResetWalkcam();
		}

		if(walkcamera.type != 2)
		{
			viewx = player->mo->PrevX + FixedMul(frac, player->mo->x - player->mo->PrevX);
			viewy = player->mo->PrevY + FixedMul(frac, player->mo->y - player->mo->PrevY);
			viewz = player->prev_viewz + FixedMul(frac, player->viewz - player->prev_viewz);
		}
		else
		{
			viewx = walkcamera.PrevX + FixedMul(frac, walkcamera.x - walkcamera.PrevX);
			viewy = walkcamera.PrevY + FixedMul(frac, walkcamera.y - walkcamera.PrevY);
			viewz = walkcamera.PrevZ + FixedMul(frac, walkcamera.z - walkcamera.PrevZ);
		}

		if(walkcamera.type)
		{
			viewangle = walkcamera.PrevAngle + FixedMul(frac, walkcamera.angle - walkcamera.PrevAngle);
			viewpitch = walkcamera.PrevPitch + FixedMul(frac, walkcamera.pitch - walkcamera.PrevPitch);
		}
		else
		{
			viewangle = player->prev_viewangle + FixedMul(frac, R_SmoothPlaying_Get(player) - player->prev_viewangle);
			viewpitch = player->prev_viewpitch + FixedMul(frac, dsda_PlayerPitch(player) - player->prev_viewpitch);
		}
	}
	else
	{
		if(walkcamera.type != 2)
		{
			viewx = player->mo->x;
			viewy = player->mo->y;
			viewz = player->viewz;
		}
		else
		{
			viewx = walkcamera.x;
			viewy = walkcamera.y;
			viewz = walkcamera.z;
		}
		if(walkcamera.type)
		{
			viewangle = walkcamera.angle;
			viewpitch = walkcamera.pitch;
		}
		else
		{
			viewangle = R_SmoothPlaying_Get(player);
			viewpitch = dsda_PlayerPitch(player);
		}
	}

	if(localQuakeHappening[displayplayer] && !dsda_Paused())
	{
		static int x_displacement;
		static int y_displacement;
		static int last_leveltime = -1;

		if(leveltime != last_leveltime)
		{
			int intensity = localQuakeHappening[displayplayer];

			x_displacement = ((M_Random() % (intensity << 2)) - (intensity << 1)) << FRACBITS;
			y_displacement = ((M_Random() % (intensity << 2)) - (intensity << 1)) << FRACBITS;

			x_displacement = x_displacement * quake_intensity / 100;
			y_displacement = y_displacement * quake_intensity / 100;

			last_leveltime = leveltime;
		}

		viewx += x_displacement;
		viewy += y_displacement;
	}

	if(R_ViewInterpolation())
	{
		int i;

		didInterp = tic_vars.frac != FRACUNIT;
		if(didInterp)
		{
			for(i = numinterpolations - 1; i >= 0; i--)
			{
				R_DoAnInterpolation(i, tic_vars.frac);
			}
		}
	}
}

void R_ResetViewInterpolation()
{
	NoInterpolateView = true;
}

static void R_CopyInterpToOld(int i)
{
	switch(curipos[i].type)
	{
		case InterpolationType::SectorFloor:
			oldipos[i][0] = ((sector_t*)curipos[i].address)->floorheight;
			break;
		case InterpolationType::SectorCeiling:
			oldipos[i][0] = ((sector_t*)curipos[i].address)->ceilingheight;
			break;
		case InterpolationType::WallPanning:
			oldipos[i][0] = ((side_t*)curipos[i].address)->rowoffset;
			oldipos[i][1] = ((side_t*)curipos[i].address)->textureoffset;
			break;
		case InterpolationType::FloorPanning:
			oldipos[i][0] = ((sector_t*)curipos[i].address)->floor_xoffs;
			oldipos[i][1] = ((sector_t*)curipos[i].address)->floor_yoffs;
			break;
		case InterpolationType::CeilingPanning:
			oldipos[i][0] = ((sector_t*)curipos[i].address)->ceiling_xoffs;
			oldipos[i][1] = ((sector_t*)curipos[i].address)->ceiling_yoffs;
			break;
	}
}

static void R_CopyBakToInterp(int i)
{
	switch(curipos[i].type)
	{
		case InterpolationType::SectorFloor:
			((sector_t*)curipos[i].address)->floorheight = bakipos[i][0];
			break;
		case InterpolationType::SectorCeiling:
			((sector_t*)curipos[i].address)->ceilingheight = bakipos[i][0];
			break;
		case InterpolationType::WallPanning:
			((side_t*)curipos[i].address)->rowoffset = bakipos[i][0];
			((side_t*)curipos[i].address)->textureoffset = bakipos[i][1];
			break;
		case InterpolationType::FloorPanning:
			((sector_t*)curipos[i].address)->floor_xoffs = bakipos[i][0];
			((sector_t*)curipos[i].address)->floor_yoffs = bakipos[i][1];
			break;
		case InterpolationType::CeilingPanning:
			((sector_t*)curipos[i].address)->ceiling_xoffs = bakipos[i][0];
			((sector_t*)curipos[i].address)->ceiling_yoffs = bakipos[i][1];
			break;
	}
}

static void R_DoAnInterpolation(int i, fixed_t smoothratio)
{
	fixed_t pos;
	fixed_t* adr1 = nullptr;
	fixed_t* adr2 = nullptr;

	switch(curipos[i].type)
	{
		case InterpolationType::SectorFloor:
			adr1 = &((sector_t*)curipos[i].address)->floorheight;
			break;
		case InterpolationType::SectorCeiling:
			adr1 = &((sector_t*)curipos[i].address)->ceilingheight;
			break;
		case InterpolationType::WallPanning:
			adr1 = &((side_t*)curipos[i].address)->rowoffset;
			adr2 = &((side_t*)curipos[i].address)->textureoffset;
			break;
		case InterpolationType::FloorPanning:
			adr1 = &((sector_t*)curipos[i].address)->floor_xoffs;
			adr2 = &((sector_t*)curipos[i].address)->floor_yoffs;
			break;
		case InterpolationType::CeilingPanning:
			adr1 = &((sector_t*)curipos[i].address)->ceiling_xoffs;
			adr2 = &((sector_t*)curipos[i].address)->ceiling_yoffs;
			break;

		default:
			return;
	}

	if(adr1)
	{
		pos = bakipos[i][0] = *adr1;
		*adr1 = oldipos[i][0] + FixedMul(pos - oldipos[i][0], smoothratio);
	}

	if(adr2)
	{
		pos = bakipos[i][1] = *adr2;
		*adr2 = oldipos[i][1] + FixedMul(pos - oldipos[i][1], smoothratio);
	}

	switch(curipos[i].type)
	{
		case InterpolationType::SectorFloor:
		case InterpolationType::SectorCeiling:
			gld_UpdateSplitData(((sector_t*)curipos[i].address));
			break;
		default:
			break;
	}
}

void R_UpdateInterpolations()
{
	int i;

	P_UpdateMobjInterpolations();

	if(!movement_smooth)
		return;
	for(i = numinterpolations - 1; i >= 0; --i)
		R_CopyInterpToOld(i);
}

int interpolations_max = 0;

static void R_SetInterpolation(InterpolationType type, void* posptr)
{
	int* i;
	if(!movement_smooth)
		return;

	if(numinterpolations >= interpolations_max)
	{
		int prevmax = interpolations_max;

		interpolations_max = interpolations_max ? interpolations_max * 2 : 256;

		if(interpolations_max == prevmax)
		{
			return;
		}

		oldipos = (fixed2_t*)Z_Realloc(oldipos, sizeof(*oldipos) * interpolations_max);
		bakipos = (fixed2_t*)Z_Realloc(bakipos, sizeof(*bakipos) * interpolations_max);
		curipos = (interpolation_t*)Z_Realloc(curipos, sizeof(*curipos) * interpolations_max);
	}

	i = nullptr;
	switch(type)
	{
		case InterpolationType::SectorFloor:
			i = &(((sector_t*)posptr)->INTERP_SectorFloor);
			break;
		case InterpolationType::SectorCeiling:
			i = &(((sector_t*)posptr)->INTERP_SectorCeiling);
			break;
		case InterpolationType::WallPanning:
			i = &(((side_t*)posptr)->INTERP_WallPanning);
			break;
		case InterpolationType::FloorPanning:
			i = &(((sector_t*)posptr)->INTERP_FloorPanning);
			break;
		case InterpolationType::CeilingPanning:
			i = &(((sector_t*)posptr)->INTERP_CeilingPanning);
			break;
	}

	if(i != nullptr && (*i) == 0)
	{
		curipos[numinterpolations].address = posptr;
		curipos[numinterpolations].type = type;
		R_CopyInterpToOld(numinterpolations);
		numinterpolations++;
		(*i) = numinterpolations;
	}
}

static void R_StopInterpolation(InterpolationType type, void* posptr)
{
	int *i, *j;
	void* posptr_last;

	if(!movement_smooth)
		return;

	i = nullptr;
	switch(type)
	{
		case InterpolationType::SectorFloor:
			i = &(((sector_t*)posptr)->INTERP_SectorFloor);
			break;
		case InterpolationType::SectorCeiling:
			i = &(((sector_t*)posptr)->INTERP_SectorCeiling);
			break;
		case InterpolationType::WallPanning:
			i = &(((side_t*)posptr)->INTERP_WallPanning);
			break;
		case InterpolationType::FloorPanning:
			i = &(((sector_t*)posptr)->INTERP_FloorPanning);
			break;
		case InterpolationType::CeilingPanning:
			i = &(((sector_t*)posptr)->INTERP_CeilingPanning);
			break;
	}

	if(i != nullptr && (*i) != 0)
	{
		numinterpolations--;

		// we have +1 in index field of interpolation's parent
		oldipos[*i - 1][0] = oldipos[numinterpolations][0];
		oldipos[*i - 1][1] = oldipos[numinterpolations][1];
		bakipos[*i - 1][0] = bakipos[numinterpolations][0];
		bakipos[*i - 1][1] = bakipos[numinterpolations][1];
		curipos[*i - 1] = curipos[numinterpolations];

		// swap indexes
		posptr_last = curipos[numinterpolations].address;
		j = nullptr;
		switch(curipos[numinterpolations].type)
		{
			case InterpolationType::SectorFloor:
				j = &(((sector_t*)posptr_last)->INTERP_SectorFloor);
				break;
			case InterpolationType::SectorCeiling:
				j = &(((sector_t*)posptr_last)->INTERP_SectorCeiling);
				break;
			case InterpolationType::WallPanning:
				j = &(((side_t*)posptr_last)->INTERP_WallPanning);
				break;
			case InterpolationType::FloorPanning:
				j = &(((sector_t*)posptr_last)->INTERP_FloorPanning);
				break;
			case InterpolationType::CeilingPanning:
				j = &(((sector_t*)posptr_last)->INTERP_CeilingPanning);
				break;
		}

		// swap
		if(j != nullptr)
		{
			*j = *i;
		}

		// reset
		*i = 0;
	}
}

void R_StopAllInterpolations()
{
	int i;

	if(!movement_smooth)
		return;

	for(i = numinterpolations - 1; i >= 0; --i)
	{
		numinterpolations--;
		oldipos[i][0] = oldipos[numinterpolations][0];
		oldipos[i][1] = oldipos[numinterpolations][1];
		bakipos[i][0] = bakipos[numinterpolations][0];
		bakipos[i][1] = bakipos[numinterpolations][1];
		curipos[i] = curipos[numinterpolations];
	}

	for(i = 0; i < numsectors; i++)
	{
		sectors[i].INTERP_CeilingPanning = 0;
		sectors[i].INTERP_FloorPanning = 0;
		sectors[i].INTERP_SectorCeiling = 0;
		sectors[i].INTERP_SectorFloor = 0;
	}

	for(i = 0; i < numsides; i++)
	{
		sides[i].INTERP_WallPanning = 0;
	}
}

void R_RestoreInterpolations()
{
	int i;

	if(!movement_smooth)
		return;

	if(didInterp)
	{
		didInterp = false;
		for(i = numinterpolations - 1; i >= 0; --i)
		{
			R_CopyBakToInterp(i);
		}
	}
}

void R_ActivateSectorInterpolations()
{
	int i;
	sector_t* sec;

	if(!movement_smooth)
		return;

	for(i = 0, sec = sectors; i < numsectors; i++, sec++)
	{
		if(sec->floordata)
			R_SetInterpolation(InterpolationType::SectorFloor, sec);
		if(sec->ceilingdata)
			R_SetInterpolation(InterpolationType::SectorCeiling, sec);
	}
}

static void R_InterpolationGetData(thinker_t* th,
	InterpolationType* type1, InterpolationType* type2,
	void** posptr1, void** posptr2)
{
	*posptr1 = nullptr;
	*posptr2 = nullptr;

	if(th->function == reinterpret_cast<think_t>(T_MoveFloor))
	{
		*type1 = InterpolationType::SectorFloor;
		*posptr1 = ((floormove_t*)th)->sector;
	}
	else if(th->function == reinterpret_cast<think_t>(T_PlatRaise))
	{
		*type1 = InterpolationType::SectorFloor;
		*posptr1 = ((plat_t*)th)->sector;
	}
	else if(th->function == reinterpret_cast<think_t>(T_MoveCeiling))
	{
		*type1 = InterpolationType::SectorCeiling;
		*posptr1 = ((ceiling_t*)th)->sector;
	}
	else if(th->function == reinterpret_cast<think_t>(T_VerticalDoor))
	{
		*type1 = InterpolationType::SectorCeiling;
		*posptr1 = ((vldoor_t*)th)->sector;
	}
	else if(th->function == reinterpret_cast<think_t>(T_MoveElevator))
	{
		*type1 = InterpolationType::SectorFloor;
		*posptr1 = ((elevator_t*)th)->sector;
		*type2 = InterpolationType::SectorCeiling;
		*posptr2 = ((elevator_t*)th)->sector;
	}
	else if(th->function == reinterpret_cast<think_t>(dsda_UpdateSideScroller) || th->function == reinterpret_cast<think_t>(dsda_UpdateControlSideScroller))
	{
		*type1 = InterpolationType::WallPanning;
		*posptr1 = sides + ((scroll_t*)th)->affectee;
	}
	else if(th->function == reinterpret_cast<think_t>(dsda_UpdateFloorScroller) || th->function == reinterpret_cast<think_t>(dsda_UpdateControlFloorScroller))
	{
		*type1 = InterpolationType::FloorPanning;
		*posptr1 = sectors + ((scroll_t*)th)->affectee;
	}
	else if(th->function == reinterpret_cast<think_t>(dsda_UpdateCeilingScroller) || th->function == reinterpret_cast<think_t>(dsda_UpdateControlCeilingScroller))
	{
		*type1 = InterpolationType::CeilingPanning;
		*posptr1 = sectors + ((scroll_t*)th)->affectee;
	}
}

void R_ActivateThinkerInterpolations(thinker_t* th)
{
	void* posptr1;
	void* posptr2;
	InterpolationType type1, type2;

	if(!movement_smooth)
		return;

	R_InterpolationGetData(th, &type1, &type2, &posptr1, &posptr2);

	if(posptr1)
	{
		R_SetInterpolation(type1, posptr1);

		if(posptr2)
			R_SetInterpolation(type2, posptr2);
	}
}

void R_StopInterpolationIfNeeded(thinker_t* th)
{
	void* posptr1;
	void* posptr2;
	InterpolationType type1, type2;

	if(!movement_smooth)
		return;

	R_InterpolationGetData(th, &type1, &type2, &posptr1, &posptr2);

	if(posptr1)
	{
		R_StopInterpolation(type1, posptr1);
		if(posptr2)
			R_StopInterpolation(type2, posptr2);
	}
}
