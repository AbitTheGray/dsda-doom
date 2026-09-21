// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Ghost

#ifndef __DSDA_GHOST__
#define __DSDA_GHOST__

void dsda_InitGhostExport(const char* name);
void dsda_InitGhostImport(const char** ghost_names, int count);
void dsda_ExportGhostFrame(void);
void dsda_SpawnGhost(void);
void dsda_UpdateGhosts(void* _void);

#endif
