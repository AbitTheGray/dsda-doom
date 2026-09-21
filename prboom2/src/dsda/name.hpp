// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Name

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define NAME_NOT_FOUND (-1)

int dsda_ActorNameToType(const char* name);
int dsda_ActionNameToNumber(const char* name);

#ifdef __cplusplus
}
#endif
