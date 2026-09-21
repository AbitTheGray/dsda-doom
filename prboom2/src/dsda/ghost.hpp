// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ghost

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_InitGhostExport(const char* name);
void dsda_InitGhostImport(const char** ghost_names, int count);
void dsda_ExportGhostFrame();
void dsda_SpawnGhost();
void dsda_UpdateGhosts(void* _void);

#ifdef __cplusplus
}
#endif
