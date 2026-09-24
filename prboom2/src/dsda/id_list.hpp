// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA ID List

#pragma once

#include "r_main.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_AddLineID(int id, int value);
void dsda_AddSectorID(int id, int value);
const int* dsda_FindLinesFromID(int id);
const int* dsda_FindSectorsFromID(int id);
const int* dsda_FindSectorsFromIDOrLine(int id, const line_t* line);
void dsda_ResetLineIDList(int size);
void dsda_ResetSectorIDList(int size);

#define FIND_SECTORS(id_p, tag) for (id_p = dsda_FindSectorsFromID(tag); *id_p >= 0; id_p++)
#define FIND_SECTORS2(id_p, tag, line) for (id_p = dsda_FindSectorsFromIDOrLine(tag, line); *id_p >= 0; id_p++)

#define FIND_LINES(id_p, tag) for (id_p = dsda_FindLinesFromID(tag); *id_p >= 0; id_p++)

#ifdef __cplusplus
}
#endif
