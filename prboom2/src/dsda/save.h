// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Save

#pragma once

void dsda_ArchiveAll(void);
void dsda_UnArchiveAll(void);
void dsda_InitSaveDir(void);
char* dsda_SaveDir(void);
char* dsda_SaveGameName(int slot, dboolean via_excmd);
void dsda_ResetDemoSaveSlots(void);
void dsda_SetLastLoadSlot(int slot);
void dsda_SetLastSaveSlot(int slot);
int dsda_LastSaveSlot(void);
void dsda_ResetLastSaveSlot(void);
int dsda_AllowAnyMenuSave(void);
int dsda_AllowMenuLoad(int slot);
int dsda_AllowAnyMenuLoad(void);
void dsda_UpdateAutoSaves(void);
