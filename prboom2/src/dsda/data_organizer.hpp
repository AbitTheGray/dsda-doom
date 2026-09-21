// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Data Organizer

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

char* dsda_DetectDirectory(const char* env_key, int arg_id);
void dsda_InitDataDir();
char* dsda_DataDir();
const char* dsda_DataRoot();

#ifdef __cplusplus
}
#endif
