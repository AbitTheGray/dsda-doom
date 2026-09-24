// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *    Declarations etc. for logical console output
 */

#pragma once

#include <stdarg.h>
#include <stddef.h>
#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

enum struct OutputLevels : int32_t
{
	Info  = 1,
	Warn  = 2,
	Error = 4,
	Debug = 8,
};

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
