// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void SV_Init();
void SV_MapTeleport(int map, int position);
void SV_StoreMapArchive();
void SV_RestoreMapArchive();

#ifdef __cplusplus
}
#endif
