// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Savegame I/O, archiving, persistence.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

#define SAVEVERSION 7

/* Persistent storage/archiving.
 * These are the load / save game routines. */
void P_ArchivePlayers();
void P_UnArchivePlayers();
void P_ArchiveWorld();
void P_UnArchiveWorld();
void P_ThinkerToIndex(); /* phares 9/13/98: save soundtarget in savegame */
void P_IndexToThinker(); /* phares 9/13/98: save soundtarget in savegame */

/* 1/18/98 killough: add RNG info to savegame */
void P_ArchiveRNG();
void P_UnArchiveRNG();

/* 2/21/98 killough: add automap info to savegame */
void P_ArchiveMap();
void P_UnArchiveMap();

// dsda - fix save / load synchronization
void P_ArchiveThinkers();
void P_UnArchiveThinkers();

extern byte* save_p;
extern byte* savebuffer;

void CheckSaveGame(size_t size);
void P_InitSaveBuffer();
void P_ForgetSaveBuffer();
void P_FreeSaveBuffer();

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

void P_ArchiveAmbientSound();
void P_UnArchiveAmbientSound();

// hexen

void P_ArchiveACS();
void P_UnArchiveACS();
void P_ArchivePolyobjs();
void P_UnArchivePolyobjs();
void P_ArchiveScripts();
void P_UnArchiveScripts();
void P_ArchiveSounds();
void P_UnArchiveSounds();
void P_ArchiveMisc();
void P_UnArchiveMisc();

#ifdef __cplusplus
}
#endif
