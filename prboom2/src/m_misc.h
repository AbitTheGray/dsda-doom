// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  External non-system-specific stuff, like storing config settings,
 *  simple file handling, and saving screnshots.
 */

#pragma once

#include "doomtype.h"

#include "dsda/configuration.h"
#include "dsda/input.h"

void M_ScreenShot (void);
void M_DoScreenShot (const char*); // cph

void M_LoadDefaults (void);
void M_SaveDefaults (void);

dboolean M_StringCopy(char *dest, const char *src, size_t dest_size);
dboolean M_StringConcat(char *dest, const char *src, size_t dest_size);

int M_StrToInt(const char *s, int *l);
int M_StrToFloat(const char *s, float *f);

int M_DoubleToInt(double x);

char* M_Strlwr(char* str);
char* M_Strupr(char* str);
char* M_StrRTrim(char* str);

typedef struct array_s
{
  void *data;
  int capacity;
  int count;
} array_t;
void M_ArrayClear(array_t *data);
void* M_ArrayGetNewItem(array_t *data, int itemsize);
