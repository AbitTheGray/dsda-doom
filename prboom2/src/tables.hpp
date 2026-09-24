// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Lookup tables.
 *      Do not try to look them up :-).
 *      In the order of appearance:
 *
 *      int finetangent[4096]   - Tangens LUT.
 *       Should work with BAM fairly well (12 of 16bit,
 *      effectively, by shifting).
 *
 *      int finesine[10240]             - Sine lookup.
 *       Guess what, serves as cosine, too.
 *       Remarkable thing is, how to use BAMs with this?
 *
 *      int tantoangle[2049]    - ArcTan LUT,
 *        maps tan(angle) to angle fast. Gotta search.
 */

#pragma once

#include "m_fixed.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT        19

// Binary Angle Measument, BAM.
#define ANG45   0x20000000
#define ANG90   0x40000000
#define ANG135  0x60000000
#define ANG180  0x80000000
#define ANG225  0xa0000000
#define ANG270  0xc0000000
#define ANG315  0xe0000000
#define ANG1      (ANG45/45)
#define ANG60     (ANG180 / 3)
#define ANGLE_MAX 0xffffffff
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define FIXED_PI 205887

#define SLOPERANGE 2048
#define SLOPEBITS    11
#define DBITS      (FRACBITS-SLOPEBITS)

typedef unsigned angle_t;

// Angles are modular, so the difference between two of them has to be computed
// in unsigned arithmetic and only then reinterpreted as signed. Casting each
// angle to int32_t first overflows whenever they are more than half a circle
// apart - which is exactly the wraparound case this is used for - and signed
// overflow is undefined behaviour: the optimiser assumes it cannot happen and
// gets the sign, and so the turn direction, wrong.
constexpr int32_t AngleDifference(angle_t left, angle_t right)
{
	return static_cast<int32_t>(left - right);
}

// Magnitude of an AngleDifference. abs() is undefined for the exact half
// circle (INT32_MIN), so negate through uint32_t instead, which reproduces the
// two's complement result the recorded demos were made against.
constexpr int32_t AngleAbs(int32_t difference)
{
	return difference >= 0
		? difference
		: static_cast<int32_t>(0u - static_cast<uint32_t>(difference));
}

#define ANGLE_T_TO_PITCH_F(x) ((float) ((x) >> ANGLETOFINESHIFT) * 360.0f / FINEANGLES)
#define ANGLE_T_TO_LOOKDIR(x) ((int) -((int) (x) * M_PI / ANG1))

// lookdir range is -110 (down) to 90 (up)
// pitch is -lookdir * ang1 / pi
// precomputed to avoid compiler-dependent floating point operation!
static const angle_t raven_angle_down_limit = 0x18e70000; // (angle_t) (int) (110 * ANG1 / M_PI);
static const angle_t raven_angle_up_limit = 0xeba00000;   // (angle_t) (int) (-90 * ANG1 / M_PI);

// Load trig tables if needed
void R_LoadTrigTables();

// Effective size is 10240.
extern fixed_t finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 phase shift.
static fixed_t* const finecosine = finesine + (FINEANGLES / 4);

// Effective size is 4096.
extern fixed_t finetangent[FINEANGLES / 2];

// Effective size is 2049;
// The +1 size is to handle the case when x==y without additional checking.

extern angle_t tantoangle[SLOPERANGE + 1];

// Utility function, called by R_PointToAngle.
typedef int (*slope_div_fn)(unsigned int num, unsigned int den);
int SlopeDiv(unsigned int num, unsigned int den);
int SlopeDivEx(unsigned int num, unsigned int den);

// More utility functions, courtesy of Quasar (James Haley).
// These are straight from Eternity so demos stay in sync.
inline static angle_t FixedToAngle(fixed_t a)
{
	return (angle_t)(((uint64_t)a * ANG1) >> FRACBITS);
}

inline static fixed_t AngleToFixed(angle_t a)
{
	return (fixed_t)(((uint64_t)a << FRACBITS) / ANG1);
}

// [XA] Clamped angle->slope, for convenience
inline static fixed_t AngleToSlope(int a)
{
	if(a > ANG90)
		return finetangent[0];
	else if(-a > ANG90)
		return finetangent[FINEANGLES / 2 - 1];
	else
		return finetangent[(ANG90 - a) >> ANGLETOFINESHIFT];
}

// [XA] Ditto, using fixed-point-degrees input
inline static fixed_t DegToSlope(fixed_t a)
{
	if(a >= 0)
		return AngleToSlope(FixedToAngle(a));
	else
		return AngleToSlope(-(int)FixedToAngle(-a));
}

#ifdef __cplusplus
}
#endif
