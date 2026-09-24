// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "r_defs.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct PolyDoorType : int32_t
{
	None,
	Slide,
	Swing,
};

typedef struct
{
	thinker_t thinker;
	int polyobj;
	int speed;
	unsigned int dist;
	int angle;
	fixed_t xSpeed; // for sliding walls
	fixed_t ySpeed;
} polyevent_t;

typedef struct
{
	thinker_t thinker;
	int polyobj;
	int speed;
	int dist;
	int totalDist;
	int direction;
	fixed_t xSpeed, ySpeed;
	int tics;
	int waitTics;
	PolyDoorType type;
	dboolean close;
} polydoor_t;

void T_PolyDoor(polydoor_t* pd);
void T_RotatePoly(polyevent_t* pe);
dboolean EV_RotatePoly(line_t* line, byte* args, int direction, dboolean overRide);
void T_MovePoly(polyevent_t* pe);
dboolean EV_MovePoly(line_t* line, byte* args, dboolean timesEight, dboolean overRide);
dboolean EV_OpenPolyDoor(line_t* line, byte* args, PolyDoorType type);

dboolean PO_MovePolyobj(int num, int x, int y);
dboolean PO_RotatePolyobj(int num, angle_t angle);
dboolean PO_Detect(int doomednum);
void PO_Init(int lump);
dboolean PO_Busy(int polyobj);

void PO_ResetBlockMap(dboolean allocate);

// zdoom

dboolean EV_RotateZDoomPoly(line_t* line, int polyobj, int speed,
	int angle, int direction, dboolean overRide);
dboolean EV_MoveZDoomPoly(line_t* line, int polyobj, int speed,
	int angle, int distance, dboolean timesEight, dboolean overRide);
dboolean EV_OpenZDoomPolyDoor(line_t* line, int polyobj, int speed,
	int angle, int distance, int delay, PolyDoorType type);
dboolean EV_StopPoly(int polyNum);
dboolean EV_MovePolyTo(line_t* line, int polyNum, fixed_t speed,
	fixed_t x, fixed_t y, dboolean overRide);

#ifdef __cplusplus
}
#endif
