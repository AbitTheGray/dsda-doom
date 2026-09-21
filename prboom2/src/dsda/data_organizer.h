// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Data Organizer

#ifndef __DSDA_DATA_ORGANIZER__
#define __DSDA_DATA_ORGANIZER__

char* dsda_DetectDirectory(const char* env_key, int arg_id);
void dsda_InitDataDir(void);
char* dsda_DataDir(void);
const char* dsda_DataRoot(void);

#endif
