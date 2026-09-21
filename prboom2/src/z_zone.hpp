// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
 *      Remark: this was the only stuff that, according
 *       to John Carmack, might have been useful for
 *       Quake.
 *
 * Rewritten by Lee Killough, though, since it was not efficient enough.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>

void Z_Free(void* ptr);
void Z_FreeLevel();

void* Z_Malloc(size_t size);
void* Z_Calloc(size_t n, size_t n2);
void* Z_Realloc(void* p, size_t n);
char* Z_Strdup(const char* s);

void* Z_MallocLevel(size_t size);
void* Z_CallocLevel(size_t n, size_t n2);
void* Z_ReallocLevel(void* p, size_t n);
char* Z_StrdupLevel(const char* s);

#ifdef __cplusplus
}
#endif
