// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *    Declarations etc. for logical console output
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stddef.h>
#include "doomtype.hpp"

typedef enum
{
	LO_INFO  = 1,
	LO_WARN  = 2,
	LO_ERROR = 4,
	LO_DEBUG = 8,
} OutputLevels;

#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

extern int lprintf(OutputLevels pri, const char* fmt, ...) __attribute__((format(printf,2,3)));

void I_EnableVerboseLogging();
void I_DisableAllLogging();
void I_DisableMessageBoxes();

/* killough 3/20/98: add const
 * killough 4/25/98: add gcc attributes
 * cphipps 01/11- moved from i_system.h */
NORETURNC11 void I_Error(const char* error, ...) __attribute__((format(printf,1,2))) NORETURN;
void I_Warn(const char* error, ...) __attribute__((format(printf,1,2)));

#ifdef __cplusplus
}
#endif
