// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Savegame I/O, archiving, persistence.
 */

#pragma once

#include "doomtype.h"

#define SAVEVERSION 7

/* Persistent storage/archiving.
 * These are the load / save game routines. */
void P_ArchivePlayers(void);
void P_UnArchivePlayers(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ThinkerToIndex(void); /* phares 9/13/98: save soundtarget in savegame */
void P_IndexToThinker(void); /* phares 9/13/98: save soundtarget in savegame */

/* 1/18/98 killough: add RNG info to savegame */
void P_ArchiveRNG(void);
void P_UnArchiveRNG(void);

/* 2/21/98 killough: add automap info to savegame */
void P_ArchiveMap(void);
void P_UnArchiveMap(void);

// dsda - fix save / load synchronization
void P_ArchiveThinkers(void);
void P_UnArchiveThinkers(void);

extern byte* save_p;
extern byte* savebuffer;

void CheckSaveGame(size_t size);
void P_InitSaveBuffer(void);
void P_ForgetSaveBuffer(void);
void P_FreeSaveBuffer(void);

#define P_SAVE_X(x) { CheckSaveGame(sizeof(x)); \
                      memcpy(save_p, &x, sizeof(x)); \
                      save_p += sizeof(x); }

#define P_LOAD_X(x) { memcpy(&x, save_p, sizeof(x)); \
                      save_p += sizeof(x); }

#define P_SAVE_SIZE(x, size) { CheckSaveGame(size); \
                               memcpy(save_p, x, size); \
                               save_p += size; }

#define P_LOAD_SIZE(x, size) { memcpy(x, save_p, size); \
                               save_p += size; }

#define P_SAVE_TYPE(x, type) { CheckSaveGame(sizeof(type)); \
                               memcpy(save_p, x, sizeof(type)); \
                               save_p += sizeof(type); }

#define P_SAVE_TYPE_REF(x, ref, type) { CheckSaveGame(sizeof(type)); \
                                        ref = (type *) save_p; \
                                        memcpy(save_p, x, sizeof(type)); \
                                        save_p += sizeof(type); }

#define P_LOAD_P(p) { memcpy(p, save_p, sizeof(*p)); \
                      save_p += sizeof(*p); }

#define P_SAVE_BYTE(x) { CheckSaveGame(1); \
                         *save_p++ = x; }

#define P_LOAD_BYTE(x) { x = *save_p++; }

#define P_SAVE_ARRAY(x) { CheckSaveGame(sizeof(x)); \
                          memcpy(save_p, x, sizeof(x)); \
                          save_p += sizeof(x); }

#define P_LOAD_ARRAY(x) { memcpy(x, save_p, sizeof(x)); \
                          save_p += sizeof(x); }

// heretic

void P_ArchiveAmbientSound(void);
void P_UnArchiveAmbientSound(void);

// hexen

void P_ArchiveACS(void);
void P_UnArchiveACS(void);
void P_ArchivePolyobjs(void);
void P_UnArchivePolyobjs(void);
void P_ArchiveScripts(void);
void P_UnArchiveScripts(void);
void P_ArchiveSounds(void);
void P_UnArchiveSounds(void);
void P_ArchiveMisc(void);
void P_UnArchiveMisc(void);
