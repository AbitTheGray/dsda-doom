// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Fixed point arithemtics, implementation.
 */

#pragma once

#include <stdlib.h>
#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * Fixed point, 32bit as 16.16.
 */

#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)

typedef int fixed_t;
typedef unsigned int ufixed_t;

/*
 * Absolute Value
 *
 * killough 5/10/98: In djgpp, use inlined assembly for performance
 * killough 9/05/98: better code seems to be gotten from using inlined C
 */

// e6y
// Microsoft and Intel compilers produce stupid and slow code for asm version of D_abs:
//
// mov    DWORD PTR $T49478[esp+12], eax
// mov    eax, DWORD PTR $T49478[esp+12]
//
// Plane abs() generates absolutely the same code as in asm version of D_abs(),
// so we do not need additional implementations for abs() at all.
//
// btw, GCC generates code without nonsenses

#define D_abs abs

/*
 * Fixed Point Multiplication
 */

/* CPhipps - made __inline__ to inline, as specified in the gcc docs
 * Also made const */

inline static CONSTFUNC fixed_t FixedMul(fixed_t a, fixed_t b)
{
	return (fixed_t)((int64_t)a * b >> FRACBITS);
}

inline static CONSTFUNC int64_t FixedMul64(int64_t a, int64_t b)
{
	return a * b >> FRACBITS;
}

/*
 * Fixed Point Division
 */

static CONSTFUNC fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	return (D_abs(a) >> 14) >= D_abs(b) ? ((a ^ b) >> 31) ^ INT_MAX : (fixed_t)(((int64_t)a << FRACBITS) / b);
}

/* CPhipps -
 * FixedMod - returns a % b, guaranteeing 0<=a<b
 * (notice that the C standard for % does not guarantee this)
 */

inline static CONSTFUNC fixed_t FixedMod(fixed_t a, fixed_t b)
{
	if(b & (b - 1))
	{
		fixed_t r = a % b;
		return ((r < 0) ? r + b : r);
	}
	else
		return (a & (b - 1));
}

static CONSTFUNC fixed_t Scale(fixed_t a, fixed_t b, fixed_t c)
{
	return (fixed_t)(((int64_t)a * b) / c);
}

#ifdef __cplusplus
}
#endif
