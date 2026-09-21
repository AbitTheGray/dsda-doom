// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      The not so system specific sound interface.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"
#include "p_mobj.hpp"
#include "r_defs.hpp"

#define MAX_CHANNELS 32

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init();

// Kills all sounds
void S_Stop();

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start();

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(void* origin, int sound_id);
void S_LoopSound(void* origin, int sfx_id, int timeout);

void S_StartSectorSound(sector_t* sector, int sfx_id);
void S_LoopSectorSound(sector_t* sector, int sfx_id, int timeout);

void S_StartMobjSound(mobj_t* mobj, int sfx_id);
void S_LoopMobjSound(mobj_t* mobj, int sfx_id, int timeout);

void S_StartVoidSound(int sfx_id);
void S_StartImportantVoidSound(int sfx_id);
void S_LoopVoidSound(int sfx_id, int timeout);

void S_StartOptionalSound(int sfx_id, int fallback_sfx_id, dboolean important);

void S_StartLineSound(line_t* line, degenmobj_t* soundorg, int sfx_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(void* origin, int sound_id, int volume, dboolean important, int loop_timeout);

// killough 4/25/98: mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND (0x8000)

// Stop sound for thing at <origin>
void S_StopSound(void* origin);

void S_StopSoundLoops();

extern int full_sounds;
void S_UnlinkSound(void* origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h, and set whether looping
void S_ChangeMusic(int music_id, int looping);
void S_ChangeMusInfoMusic(int lumpnum, int looping);
dboolean S_ChangeMusicByName(const char* name, dboolean looping);
void S_RestartMusic();

// Stops the music fer sure.
void S_StopMusic();

// Stop and resume music, during game PAUSE.
void S_PauseSound();
void S_ResumeSound();

void S_AdjustAttenuation(float attenuation);
void S_AdjustVolume(float volume);
void S_ResetAdjustments();

//
// Updates music & sounds
//
void S_UpdateSounds();

// machine-independent sound params
extern int default_numChannels;
extern int numChannels;

//jff 3/17/98 holds last IDMUS number, or -1
extern int idmusnum;

// heretic

#include "doomtype.hpp"

void S_SetSoundCurve(dboolean fullprocess);
void S_StartAmbientSound(void* origin, int sound_id, int volume);

// hexen

void S_StartSongName(const char* songLump, dboolean loop);
dboolean S_GetSoundPlayingInfo(void* mobj, int sound_id);
int S_GetSoundID(const char* name);

void S_ResetVolume();

#ifdef __cplusplus
}
#endif
