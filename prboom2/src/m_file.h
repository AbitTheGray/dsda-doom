// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  External simple file handling.
 */

#pragma once

#include <stdio.h>

#include "doomtype.h"

dboolean M_ReadWriteAccess(const char* name);
dboolean M_ReadAccess(const char* name);
dboolean M_WriteAccess(const char* name);
int M_MakeDir(const char* path, int require);
dboolean M_IsDir(const char* name);
FILE* M_OpenFile(const char* name, const char* mode);
int M_OpenRB(const char* name);
dboolean M_FileExists(const char* name);
dboolean M_WriteFile(char const* name, const void* source, size_t length);
int M_ReadFile(char const* name, byte** buffer);
int M_ReadFileToString(char const* name, char** buffer);
dboolean M_RemoveFilesAtPath(const char* path);

int M_remove(const char* path);
char* M_getcwd(char* buffer, int len);
char* M_getenv(const char* name);

#ifdef _WIN32
wchar_t* ConvertUtf8ToWide(const char* str);
char* ConvertWideToUtf8(const wchar_t* wstr);
#endif
char* ConvertSysNativeMBToUtf8(const char* str);
char* ConvertUtf8ToSysNativeMB(const char* str);
