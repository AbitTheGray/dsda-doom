// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Save

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void dsda_ArchiveAll();
void dsda_UnArchiveAll();
void dsda_InitSaveDir();
char* dsda_SaveDir();
char* dsda_SaveGameName(int slot, dboolean via_excmd);
void dsda_ResetDemoSaveSlots();
void dsda_SetLastLoadSlot(int slot);
void dsda_SetLastSaveSlot(int slot);
int dsda_LastSaveSlot();
void dsda_ResetLastSaveSlot();
int dsda_AllowAnyMenuSave();
int dsda_AllowMenuLoad(int slot);
int dsda_AllowAnyMenuLoad();
void dsda_UpdateAutoSaves();

#ifdef __cplusplus
}
#endif
