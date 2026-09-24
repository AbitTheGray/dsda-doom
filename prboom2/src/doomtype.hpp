// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Simple basic typedefs, isolated here to make it easier
 *       separating modules.
 */

#pragma once

#include <stdint.h>

#include "cpp/Util.hpp"

/* cph - from v_video.h, needed by gl_struct.h
 * The low nibble selects an alignment; the higher bits are independent flags.
 */
enum struct PatchTranslation : uint32_t
{
	// e6y: wide-res
	AlignLeft        = 1,
	AlignRight       = 2,
	AlignTop         = 3,
	AlignLeftTop     = 4,
	AlignRightTop    = 5,
	AlignBottom      = 6,
	AlignWide        = 7,
	AlignLeftBottom  = 8,
	AlignRightBottom = 9,
	AlignMax         = 10,
	Stretch          = 16, // Stretch to compensate for high-res
	ExText           = 32,

	None        = 128, // Normal
	Flip        = 256, // Flip image horizontally
	Trans       = 512, // Translate image via a translation table
	NoOffset    = 1024,
	StretchReal = 2048, // [XA] VPT_STRETCH in gld_fillRect means "tile", rather than "stretch"... these flags probably need a rename.

	AlignMask   = 0xf,
	StretchMask = 0x1f,
};
ENUM_FLAGS_FUNC(PatchTranslation)

// the low nibble of the flags selects the alignment
inline constexpr PatchTranslation PatchAlignment(const PatchTranslation flags) noexcept
{
	return static_cast<PatchTranslation>(
		std::to_underlying(flags) & std::to_underlying(PatchTranslation::AlignMask));
}

// the alignment nibble plus the stretch bit
inline constexpr PatchTranslation PatchStretchBits(const PatchTranslation flags) noexcept
{
	return static_cast<PatchTranslation>(
		std::to_underlying(flags) & std::to_underlying(PatchTranslation::StretchMask));
}

#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

typedef int dboolean;

typedef unsigned char byte;

//e6y
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef BETWEEN
#define BETWEEN(l,u,x) ((l)>(x)?(l):(x)>(u)?(u):(x))
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX MAX_PATH
#else
#define PATH_MAX 1024
#endif
#endif

#ifdef __GNUC__
#define CONSTFUNC __attribute__((const))
#define PUREFUNC __attribute__((pure))
#define NORETURN __attribute__ ((noreturn))
#else
#define CONSTFUNC
#define PUREFUNC
#define NORETURN
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define NORETURNC11 _Noreturn
#else
#define NORETURNC11
#endif

// Definition of PACKEDATTR from Chocolate Doom
#ifdef __GNUC__
#if defined(_WIN32) && !defined(__clang__)
#define PACKEDATTR __attribute__((packed,gcc_struct))
#else
#define PACKEDATTR __attribute__((packed))
#endif
#else
#define PACKEDATTR
#endif

#ifdef WIN32
#define C_DECL __cdecl
#else
#define C_DECL
#endif

#ifdef _MSC_VER
#define INLINE __forceinline /* use __forceinline (VC++ specific) */
#else
#define INLINE inline        /* use standard inline */
#endif

enum struct CompLevel : int32_t
{
	Doom12,            /* Doom v1.2 */
	Doom1666,          /* Doom v1.666 */
	Doom219,           /* Doom & Doom 2 v1.9 */
	Ultdoom,            /* Ultimate Doom and Doom95 */
	Finaldoom,          /* Final Doom */
	Dosdoom,            /* DosDoom 0.47 */
	Tasdoom,            /* TASDoom */
	BoomCompatibility, /* Boom's compatibility mode */
	Boom201,           /* Boom v2.01 */
	Boom202,           /* Boom v2.02 */
	Lxdoom1,           /* LxDoom v1.3.2+ */
	Mbf,                /* MBF */
	Prboom1,           /* PrBoom 2.03beta? */
	Prboom2,           /* PrBoom 2.1.0-2.1.1 */
	Prboom3,           /* PrBoom 2.2.x */
	Prboom4,           /* PrBoom 2.3.x */
	Prboom5,           /* PrBoom 2.4.0 */
	Prboom6,           /* Latest PrBoom */
	Placeholder18,
	Placeholder19,
	Placeholder20,
	Mbf21,                         /* MBF21 */
	Max,                     /* Must be last entry */
	/* Aliases follow */
	Boom = Boom201, /* Alias used by G_Compatibility */
	Best = Mbf21
};

typedef CompLevel complevel_t;

/* cph - from v_video.h, needed by gl_struct.h */
extern int global_patch_top_offset;

#define BOTTOM_ALIGNMENT(x) ((x) == PatchTranslation::AlignBottom || \
                             (x) == PatchTranslation::AlignLeftBottom || \
                             (x) == PatchTranslation::AlignRightBottom)

#define TOP_ALIGNMENT(x) ((x) == PatchTranslation::AlignTop || \
                          (x) == PatchTranslation::AlignLeftTop || \
                          (x) == PatchTranslation::AlignRightTop)

#define arrlen(array) (sizeof(array) / sizeof(*array))

#ifdef __cplusplus
}
#endif
