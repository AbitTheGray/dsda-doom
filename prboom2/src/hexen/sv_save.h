// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __HEXEN_SV_SAVE__
#define __HEXEN_SV_SAVE__

void SV_Init(void);
void SV_MapTeleport(int map, int position);
void SV_StoreMapArchive(void);
void SV_RestoreMapArchive(void);

#endif
