// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

/*
Anything that implements all of these functions can play music in prboomplus.

render: If audio output isn't delivered by render (say, for midi out), then the
player won't be recordable with video capture.  In that case, still write 0s to
the buffer when render is called.

thread safety: Locks are handled in i_sound.c.  Don't worry about it.

Timing: If you're outputting to render, your timing should come solely from the
calls to render, and not some external timing source.  That's why things stay
synced.
*/


typedef struct
{
  // descriptive name of the player, such as "OPL2 Synth"
  const char *(*name)(void);

  // samplerate is in hz.  return is 1 for success
  int (*init)(int samplerate);

  // deallocate structures, cleanup, ...
  void (*shutdown)(void);

  // set volume, 0 = off, 15 = max
  void (*setvolume)(int v);

  // pause currently running song.
  void (*pause)(void);

  // undo pause
  void (*resume)(void);

  // return a player-specific handle, or NULL on failure.
  // data does not belong to player, but it will persist as long as unregister is not called
  const void *(*registersong)(const void *data, unsigned len);

  // deallocate structures, etc.  data is no longer valid
  void (*unregistersong)(const void *handle);

  void (*play)(const void *handle, int looping);

  // stop
  void (*stop)(void);

  // s16 stereo, with samplerate as specified in init.  player needs to be able to handle
  // just about anything for nsamp.  render can be called even during pause+stop.
  void (*render)(void *dest, unsigned nsamp);
} music_player_t;

#endif // MUSICPLAYER_H
