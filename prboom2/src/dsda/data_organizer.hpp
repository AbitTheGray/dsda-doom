// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Data Organizer

#pragma once

#include <stdint.h>

// declared in dsda/args.hpp; the fixed underlying type makes this enough
enum struct ArgId : int32_t;

#ifdef __cplusplus
extern "C"
{
#endif

char* dsda_DetectDirectory(const char* env_key, ArgId arg_id);
void dsda_InitDataDir();
char* dsda_DataDir();
const char* dsda_DataRoot();

#ifdef __cplusplus
}
#endif
