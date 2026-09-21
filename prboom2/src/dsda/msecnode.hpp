// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA MSecNode Management

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "r_defs.hpp"

void dsda_ArchiveMSecNodes();
void dsda_UnArchiveMSecNodes(mobj_t** mobj_p, int mobj_count);

#ifdef __cplusplus
}
#endif
