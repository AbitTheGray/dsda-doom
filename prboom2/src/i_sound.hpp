// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      System interface, sound.
 */

#pragma once

#include <stddef.h>
#include "sounds.hpp"
#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#define SNDSERV
#undef SNDINTR

#ifndef SNDSERV
#include "l_soundgen.h"
#endif

// Init at program start...
void I_InitSound();

// ... shut down and relase at program termination.
void I_ShutdownSound();

//
//  SFX I/O
//

void I_CacheSounds();

// Initialize channels?
void I_SetChannels();

// Get raw data lump index for sound descriptor.
int I_GetSfxLumpNum(sfxinfo_t* sfxinfo);

// Starts a sound in a particular sound channel.
int I_StartSound(SfxId id, int channel, sfx_params_t* params);

// Stops a sound channel.
void I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
dboolean I_SoundIsPlaying(int handle);

// Called by m_menu.c to let the quit sound play and quit right after it stops
dboolean I_AnySoundStillPlaying();

// Updates the volume, separation,
//  and pitch of a sound channel.
void I_UpdateSoundParams(int handle, sfx_params_t* params);

// NSM sound capture routines
// silences sound output, and instead allows sound capture to work
// call this before sound startup
void I_SetSoundCap();
// grabs len samples of audio (16 bit interleaved)
unsigned char* I_GrabSound(int len);

// NSM helper routine for some of the streaming audio
void I_ResampleStream(void* dest, unsigned nsamp, void (*proc)(void* dest, unsigned nsamp), unsigned sratein, unsigned srateout);

//
//  MUSIC I/O
//
extern char music_player_order[][200];

void I_InitMusic();
void I_ShutdownMusic();

// PAUSE game handling.
void I_PauseSong(int handle);
void I_ResumeSong(int handle);

// Registers a song handle to song data.
int I_RegisterSong(const void* data, size_t len);

// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void I_PlaySong(int handle, int looping);

// Stops a song over 3 seconds.
void I_StopSong(int handle);

// See above (register), then think backwards
void I_UnRegisterSong(int handle);

// CPhipps - put these in config file
extern int snd_samplerate;

// prefered MIDI player
enum struct MidiPlayerName : int32_t
{
	Fluidsynth,
	Opl,
	Portmidi,

	Last
};

extern const char* midiplayers[];

void M_ChangeMIDIPlayer();

#ifdef __cplusplus
}
#endif
