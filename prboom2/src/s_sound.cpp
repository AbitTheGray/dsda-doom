// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  Platform-independent sound code
 */

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include "doomstat.hpp"
#include "doomtype.hpp"
#include "s_sound.hpp"
#include "s_advsound.hpp"
#include "i_sound.hpp"
#include "i_system.hpp"
#include "d_main.hpp"
#include "r_main.hpp"
#include "m_random.hpp"
#include "w_wad.hpp"
#include "lprintf.hpp"
#include "p_maputl.hpp"
#include "p_setup.hpp"
#include "e6y.hpp"

#include "hexen/sn_sonix.hpp"

#include "dsda/configuration.hpp"
#include "dsda/map_format.hpp"
#include "dsda/mapinfo.hpp"
#include "dsda/memory.hpp"
#include "dsda/music.hpp"
#include "dsda/settings.hpp"
#include "dsda/sfx.hpp"
#include "dsda/skip.hpp"

// Adjustable by menu.
#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128
#define S_STEREO_SWING (96<<FRACBITS)

const int channel_not_found = -1;

typedef struct
{
	sfxinfo_t* sfxinfo; // sound information (if null, channel avail.)
	void* origin;       // origin of sound
	int handle;         // handle of the sound being played
	int pitch;

	// heretic
	int priority;

	// hexen
	int volume;

	dboolean active;
	dboolean ambient;
	float attenuation;
	float volume_factor;
	dboolean loop;
	int loop_timeout;
	SfxClass sfx_class;
} channel_t;

// the set of channels available
static channel_t channels[MAX_CHANNELS];
static degenmobj_t sobjs[MAX_CHANNELS];

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int snd_SfxVolume;

// Derived value (not saved, accounts for muted sfx)
static int sfx_volume;

// Maximum volume of music.
int snd_MusicVolume = 15;

// whether songs are mus_paused
static dboolean mus_paused;

// music currently being played
musicinfo_t* mus_playing;

// music currently should play
static MusicId musicnum_current;

// number of channels available
int numChannels;

//jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Internals.
//

extern "C" void S_StopChannel(int cnum);

extern "C" int S_AdjustSoundParams(mobj_t* listener, mobj_t* source, channel_t* channel, sfx_params_t* params);

static int S_getChannel(void* origin, sfxinfo_t* sfxinfo, sfx_params_t* params);


// heretic
int max_snd_dist = 1600;
int dist_adjust = 160;

static byte* soundCurve;
static int AmbChan = -1;

static mobj_t* GetSoundListener();
static void Heretic_S_StopSound(void* _origin);
static void Raven_S_StartSoundAtVolume(void* _origin, SfxId sound_id, int volume, int loop_timeout);

extern "C" void S_ResetSfxVolume()
{
	snd_SfxVolume = dsda_IntConfig(ConfigId::SfxVolume);

	if(nosfxparm)
		return;

	if(dsda_MuteSfx())
		sfx_volume = 0;
	else
		sfx_volume = snd_SfxVolume;
}

extern "C" void I_ResetMusicVolume();
void S_ResetVolume()
{

	S_ResetSfxVolume();
	I_ResetMusicVolume();
}

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

extern "C" void I_ResetMusicVolume();
void S_Init()
{
	idmusnum = -1; //jff 3/17/98 insure idmus number is blank

	S_Stop();

	numChannels = dsda_IntConfig(ConfigId::SndChannels);

	//jff 1/22/98 skip sound init if sound not enabled
	if(!nosfxparm)
	{
		static dboolean first_s_init = true;

		// Whatever these did with DMX, these are rather dummies now.
		I_SetChannels();

		S_ResetSfxVolume();

		// Reset channel memory
		memset(channels, 0, sizeof(channels));
		memset(sobjs, 0, sizeof(sobjs));

		if(first_s_init)
		{
			int i;
			int snd_curve_lump;

			first_s_init = false;

			for(i = 1; i < num_sfx; i++)
				S_sfx[i].lumpnum = -1;

			dsda_CacheSoundLumps();

			Log::Debug(" Precaching all sound effects... ");
			I_CacheSounds();
			Log::Debug("done\n");

			// {
			//   int i;
			//   const int snd_curve_length = 1200;
			//   const int flat_curve_length = 160;
			//   byte* buffer = Z_Malloc(snd_curve_length);
			//   for (i = 0; i < snd_curve_length; ++i)
			//   {
			//     if (i < flat_curve_length)
			//       buffer[i] = 127;
			//     else
			//       buffer[i] = 127 * (snd_curve_length - i) / (snd_curve_length - flat_curve_length);

			//     if (!buffer[i])
			//       buffer[i] = 1;
			//   }
			//   M_WriteFile("sndcurve.lmp", buffer, snd_curve_length);
			// }

			snd_curve_lump = W_GetNumForName("SNDCURVE");
			max_snd_dist = W_LumpLength(snd_curve_lump);

			dist_adjust = max_snd_dist / 10;

			soundCurve = static_cast<byte*>(Z_Malloc(max_snd_dist));
			memcpy(soundCurve, (const byte*)W_LumpByNum(snd_curve_lump), max_snd_dist);
		}
	}

	// CPhipps - music init reformatted
	if(!nomusicparm)
	{

		I_ResetMusicVolume();

		// no sounds are playing, and they are not mus_paused
		mus_paused = 0;
	}
}

void S_Stop()
{
	int cnum;

	// heretic
	AmbChan = -1;

	//jff 1/22/98 skip sound init if sound not enabled
	if(!nosfxparm)
		for(cnum = 0; cnum < numChannels; cnum++)
			if(channels[cnum].active)
				S_StopChannel(cnum);
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start()
{
	int mnum;
	int muslump;
	dboolean no_musinfo_default;

	// kill all playing sounds at start of level
	//  (trust me - a good idea)

	S_Stop();

	// start new music for the level
	mus_paused = 0;

	dsda_MapMusic(&mnum, &muslump, gameepisode, gamemap);

	if(muslump >= 0)
	{
		musinfo.items[0] = muslump;
	}

	no_musinfo_default = (musinfo.items[0] == -1);

	// Keep map's default music available to MUSINFO slot 0
	// Needed when restoring queued music from a key frame
	if(no_musinfo_default)
		musinfo.items[0] = dsda_MusicIndexToLumpNum(mnum);

	if(!dsda_StartQueuedMusic())
	{
		if(no_musinfo_default)
			S_ChangeMusic(static_cast<MusicId>(mnum), true);
		else
			S_ChangeMusInfoMusic(musinfo.items[0], true);
	}
}

static float adjust_attenuation;
static float adjust_volume;

void S_AdjustAttenuation(float attenuation)
{
	adjust_attenuation = attenuation;
}

void S_AdjustVolume(float volume)
{
	adjust_volume = volume;
}

void S_ResetAdjustments()
{
	adjust_attenuation = 0;
	adjust_volume = 0;
}

void S_StartSoundAtVolume(void* origin_p, SfxId sfx_id, int volume, dboolean important, int loop_timeout)
{
	int cnum;
	sfx_params_t params;
	sfxinfo_t* sfx;
	mobj_t* origin;
	mobj_t* listener;

	if(raven) return Raven_S_StartSoundAtVolume(origin_p, sfx_id, volume, loop_timeout);

	origin = (mobj_t*)origin_p;
	listener = GetSoundListener();

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

	// killough 4/25/98
	if(sfx_id == g_sfx_secret)
		params.sfx_class = SfxClass::Secret;
	else if(important || SfxIsPickup(sfx_id) || sfx_id == SfxId::Oof ||
		(compatibility_level >= CompLevel::Prboom2 && sfx_id == SfxId::Noway))
		params.sfx_class = SfxClass::Important;
	else
		params.sfx_class = SfxClass::None;

	params.ambient = false;
	params.attenuation = adjust_attenuation;
	params.volume_factor = adjust_volume;
	params.loop = loop_timeout > 0;
	params.loop_timeout = loop_timeout;

	sfx_id = SfxWithoutPickup(sfx_id);

	if(sfx_id == SfxId::None)
		return;

	// check for bogus sound #
	if(std::to_underlying(sfx_id) < 1 || std::to_underlying(sfx_id) > num_sfx)
		Log::Fatal("S_StartSoundAtVolume: Bad sfx #: {}", std::to_underlying(sfx_id));

	sfx = &S_sfx[std::to_underlying(sfx_id)];

	// Initialize sound parameters
	params.priority = 128 - sfx->priority;
	if(params.priority <= 0)
		params.priority = 1;
	if(sfx->pitch < 0)
		params.pitch = NORM_PITCH;
	else
		params.pitch = sfx->pitch;
	params.volume = volume;

	// Check to see if it is audible, modify the params
	// killough 3/7/98, 4/25/98: code rearranged slightly

	if(!origin || origin == listener)
	{
		params.separation = NORM_SEP;
		params.volume *= 8;
		params.priority *= 10;
	}
	else if(!S_AdjustSoundParams(listener, origin, nullptr, &params))
		return;
	else if(origin->x == listener->x && origin->y == listener->y)
		params.separation = NORM_SEP;

	if(dsda_BlockSFX(sfx)) return;

	// hacks to vary the sfx pitches
	if(sfx_id >= SfxId::Sawup && sfx_id <= SfxId::Sawhit)
		params.pitch += 8 - (M_Random() & 15);
	else if(sfx_id != SfxId::Itemup && sfx_id != SfxId::Tink)
		params.pitch += 16 - (M_Random() & 31);

	if(params.pitch < 0)
		params.pitch = 0;

	if(params.pitch > 255)
		params.pitch = 255;

	// try to find a channel
	cnum = S_getChannel(origin, sfx, &params);

	if(cnum == channel_not_found)
		return;

	// get lumpnum if necessary
	// killough 2/28/98: make missing sounds non-fatal
	if(sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
		return;

	// Assigns the handle to one of the channels in the mix/output buffer.
	{
		// e6y: [Fix] Crash with zero-length sounds.
		int h = I_StartSound(sfx_id, cnum, &params);
		if(h != -1)
		{
			channels[cnum].handle = h;
			channels[cnum].pitch = params.pitch;
			channels[cnum].priority = params.priority;
			channels[cnum].ambient = params.ambient;
			channels[cnum].attenuation = params.attenuation;
			channels[cnum].volume_factor = params.volume_factor;
			channels[cnum].loop = params.loop;
			channels[cnum].loop_timeout = params.loop_timeout;
			channels[cnum].active = true;
		}
	}
}

void S_StartSectorSound(sector_t* sector, SfxId sfx_id)
{
	if(sector->flags & SECF_SILENT)
		return;

	S_StartSound((mobj_t*)&sector->soundorg, sfx_id);
}

void S_LoopSectorSound(sector_t* sector, SfxId sfx_id, int timeout)
{
	if(sector->flags & SECF_SILENT)
		return;

	S_LoopSound((mobj_t*)&sector->soundorg, sfx_id, timeout);
}

void S_StartMobjSound(mobj_t* mobj, SfxId sfx_id)
{
	if(mobj && mobj->subsector && mobj->subsector->sector->flags & SECF_SILENT)
		return;

	S_StartSound(mobj, sfx_id);
}

void S_LoopMobjSound(mobj_t* mobj, SfxId sfx_id, int timeout)
{
	if(mobj && mobj->subsector && mobj->subsector->sector->flags & SECF_SILENT)
		return;

	S_LoopSound(mobj, sfx_id, timeout);
}

void S_StartVoidSound(SfxId sfx_id)
{
	S_StartSound(nullptr, sfx_id);
}

void S_LoopVoidSound(SfxId sfx_id, int timeout)
{
	S_LoopSound(nullptr, sfx_id, timeout);
}

void S_StartOptionalSound(SfxId sfx_id, SfxId fallback_sfx_id, dboolean important)
{
	if(I_GetSfxLumpNum(&S_sfx[std::to_underlying(sfx_id)]) != -1)
	{
		S_StartSoundAtVolume(nullptr, sfx_id, raven ? 127 : sfx_volume, important, 0);
	}
	else if(fallback_sfx_id != SfxId::NoFallback) // Play a fallback?
	{
		S_StartSoundAtVolume(nullptr, fallback_sfx_id, raven ? 127 : sfx_volume, important, 0);
	}
}

void S_StartLineSound(line_t* line, degenmobj_t* soundorg, SfxId sfx_id)
{
	if(line && line->frontsector && line->frontsector->flags & SECF_SILENT)
		return;

	S_StartSound((mobj_t*)soundorg, sfx_id);
}

void S_StartSound(void* origin, SfxId sfx_id)
{
	S_StartSoundAtVolume(origin, sfx_id, raven ? 127 : sfx_volume, false, 0);
}

void S_LoopSound(void* origin, SfxId sfx_id, int timeout)
{
	S_StartSoundAtVolume(origin, sfx_id, raven ? 127 : sfx_volume, false, timeout);
}

void S_StopSound(void* origin)
{
	int cnum;

	if(raven) return Heretic_S_StopSound(origin);

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

	for(cnum = 0; cnum < numChannels; cnum++)
		if(channels[cnum].active && channels[cnum].origin == origin)
		{
			S_StopChannel(cnum);
			break;
		}
}

void S_StopSoundLoops()
{
	int cnum;

	if(nosfxparm)
		return;

	for(cnum = 0; cnum < numChannels; ++cnum)
		if(channels[cnum].active && channels[cnum].loop)
			S_StopChannel(cnum);
}

// [FG] disable sound cutoffs
int full_sounds;

void S_UnlinkSound(void* origin)
{
	int cnum;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

	if(origin)
	{
		for(cnum = 0; cnum < numChannels; cnum++)
		{
			if(channels[cnum].active && channels[cnum].origin == origin)
			{
				degenmobj_t* const sobj = &sobjs[cnum];
				const mobj_t* const mobj = (mobj_t*)origin;
				sobj->x = mobj->x;
				sobj->y = mobj->y;
				sobj->z = mobj->z;
				channels[cnum].origin = (mobj_t*)sobj;
				break;
			}
		}
	}
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound()
{
	//jff 1/22/98 return if music is not enabled
	if(nomusicparm)
		return;

	if(mus_playing && !mus_paused)
	{
		I_PauseSong(mus_playing->handle);
		mus_paused = true;
	}
}

void S_ResumeSound()
{
	//jff 1/22/98 return if music is not enabled
	if(nomusicparm)
		return;

	if(mus_playing && mus_paused)
	{
		I_ResumeSong(mus_playing->handle);
		mus_paused = false;
	}
}


//
// Updates music & sounds
//
void S_UpdateSounds()
{
	mobj_t* listener;
	int cnum;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

#ifdef UPDATE_MUSIC
	I_UpdateMusic();
#endif

	listener = GetSoundListener();
	if(sfx_volume == 0)
		return;

	if(map_format.sndseq)
	{
		// Update any Sequences
		SN_UpdateActiveSequences();
	}

	for(cnum = 0; cnum < numChannels; cnum++)
	{
		channel_t* channel = &channels[cnum];

		if(channel->active)
		{
			if(channel->loop && --channel->loop_timeout < 0)
			{
				S_StopChannel(cnum);
			}
			else if(I_SoundIsPlaying(channel->handle))
			{
				sfx_params_t params;

				// check non-local sounds for distance clipping
				// or modify their params
				if(channel->origin && listener != channel->origin) // killough 3/20/98
				{
					if(S_AdjustSoundParams(listener, static_cast<mobj_t *>(channel->origin), channel, &params))
					{
						I_UpdateSoundParams(channel->handle, &params);
						channel->priority = params.priority;
					}
					else
					{
						raven ? S_StopSound(channel->origin) : S_StopChannel(cnum);
					}
				}
			}
			else // if channel is allocated but sound has stopped, free it
				S_StopChannel(cnum);
		}
	}
}

// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(MusicId m_id)
{
	S_ChangeMusic(m_id, false);
}

dboolean S_ChangeMusicByName(const char* name, dboolean looping)
{
	int lump = W_CheckNumForName(name);

	if(lump == LUMP_NOT_FOUND)
	{
		S_StopMusic();
		return false;
	}

	S_ChangeMusInfoMusic(lump, looping);
	return true;
}

void S_ChangeMusic(MusicId musicnum, int looping)
{
	musicinfo_t* music;

	// current music which should play
	musicnum_current = musicnum;
	musinfo.current_item = -1;
	S_music[mus_musinfo].lumpnum = -1;

	//jff 1/22/98 return if music is not enabled
	if(nomusicparm)
		return;

	if(musicnum <= MusicId::None || std::to_underlying(musicnum) >= num_music)
		Log::Fatal("S_ChangeMusic: Bad music number {}", std::to_underlying(musicnum));

	music = &S_music[std::to_underlying(musicnum)];

	if(mus_playing == music)
		return;

	// shutdown old music
	S_StopMusic();

	// get lumpnum if necessary
	if(!music->lumpnum)
		music->lumpnum = dsda_MusicIndexToLumpNum(std::to_underlying(musicnum));

	// load & register it
	music->data = static_cast<decltype(music->data)>(W_LumpByNum(music->lumpnum));
	music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

	// play it
	I_PlaySong(music->handle, looping);

	mus_playing = music;

	musinfo.current_item = -1;

	// [crispy] MUSINFO value 0 is reserved for the map's default music
	if(musinfo.items[0] == -1)
	{
		musinfo.items[0] = music->lumpnum;
		S_music[mus_musinfo].lumpnum = -1;
	}
}

void S_RestartMusic()
{
	if(musinfo.current_item != -1)
	{
		S_ChangeMusInfoMusic(musinfo.current_item, true);
	}
	else
	{
		if(musicnum_current > MusicId::None && std::to_underlying(musicnum_current) < num_music)
		{
			S_ChangeMusic(musicnum_current, true);
		}
	}
}

void S_ChangeMusInfoMusic(int lumpnum, int looping)
{
	musicinfo_t* music;

	if(dsda_SkipMode())
	{
		musinfo.current_item = lumpnum;
		return;
	}

	//jff 1/22/98 return if music is not enabled
	if(nomusicparm)
		return;

	if(mus_playing && mus_playing->lumpnum == lumpnum)
		return;

	music = &S_music[mus_musinfo];

	// Allow MUSINFO music to restart after MIDI player changes
	if(music->lumpnum == lumpnum && mus_playing)
		return;

	// shutdown old music
	S_StopMusic();

	// save lumpnum
	music->lumpnum = lumpnum;

	// load & register it
	music->data = static_cast<decltype(music->data)>(W_LumpByNum(music->lumpnum));
	music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

	// play it
	I_PlaySong(music->handle, looping);

	mus_playing = music;

	musinfo.current_item = lumpnum;
}

void S_StopMusic()
{
	//jff 1/22/98 return if music is not enabled
	if(nomusicparm)
		return;

	if(mus_playing)
	{
		if(mus_paused)
			I_ResumeSong(mus_playing->handle);

		I_StopSong(mus_playing->handle);
		I_UnRegisterSong(mus_playing->handle);

		mus_playing->data = nullptr;
		mus_playing = nullptr;
	}
}



extern "C" void S_StopChannel(int cnum)
{
	channel_t* c = &channels[cnum];

	if(AmbChan == cnum)
		AmbChan = -1;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

	if(c->active)
	{
		// stop the sound playing
		if(I_SoundIsPlaying(c->handle))
			I_StopSound(c->handle);

		c->active = false;
		c->sfxinfo = nullptr;
		c->origin = nullptr;
		c->handle = 0;
	}
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

extern "C" int S_AdjustSoundParams(mobj_t* listener, mobj_t* source, channel_t* channel, sfx_params_t* params)
{
	fixed_t adx, ady;
	ufixed_t approx_dist;
	angle_t angle;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return 0;

	// e6y
	if(!listener)
		return 0;

	if(channel)
	{
		params->ambient = channel->ambient;
		params->attenuation = channel->attenuation;
		params->volume_factor = channel->volume_factor;
		params->loop = channel->loop;
		params->loop_timeout = channel->loop_timeout;
	}

	// calculate the distance to sound origin
	//  and clip it if necessary
	adx = D_abs(listener->x - source->x);
	ady = D_abs(listener->y - source->y);

	approx_dist = P_AproxDistance(adx, ady);
	approx_dist >>= FRACBITS;

	if(params->attenuation)
		approx_dist *= params->attenuation;

	if(approx_dist >= max_snd_dist)
		return 0;

	// angle of source to listener
	angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

	if(angle <= listener->angle)
		angle += 0xffffffff;
	angle -= listener->angle;
	angle >>= ANGLETOFINESHIFT;

	// stereo separation
	params->separation = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

	// volume calculation
	if(raven)
	{
		if(channel)
		{
			params->volume =
				(soundCurve[approx_dist] * sfx_volume * 8 * channel->volume) >> 14;
		}
		else
		{
			// currently raven only adjusts on update (channel exists)
		}
	}
	else
	{
		params->volume = (soundCurve[approx_dist] * sfx_volume * 8) >> 7;
		if(params->volume_factor)
		{
			params->volume *= params->volume_factor;
			if(params->volume > 119)
				params->volume = 119;
		}
	}

	if(channel)
	{
		params->pitch = channel->pitch;
		params->priority = channel->sfxinfo->priority;
		if(!raven)
			params->priority = 128 - params->priority;
	}

	// heretic_note: divides by 256 instead of the dist_adjust
	params->priority *= (10 - approx_dist / dist_adjust);

	return (params->volume > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int S_ChannelScore(channel_t* channel)
{
	return channel->priority;
}

static int S_LowestScoreChannel()
{
	int cnum;
	int lowest_score = INT_MAX;
	int lowest_cnum = channel_not_found;

	for(cnum = 0; cnum < numChannels; ++cnum)
	{
		int score = S_ChannelScore(&channels[cnum]);

		if(score < lowest_score)
		{
			lowest_score = score;
			lowest_cnum = cnum;
		}
	}

	return lowest_cnum;
}

static int S_getChannel(void* origin, sfxinfo_t* sfxinfo, sfx_params_t* params)
{
	// channel number to use
	int cnum;
	channel_t* c;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return channel_not_found;

	// Only allow one sound per origin
	// Preserve the secret revealed sound, unless a new one is called
	for(cnum = 0; cnum < numChannels; cnum++)
		if(channels[cnum].active && channels[cnum].origin == origin &&
			(comp[std::to_underlying(CompOption::Sound)] || channels[cnum].sfx_class == params->sfx_class) &&
			(channels[cnum].sfx_class != SfxClass::Secret || params->sfx_class == SfxClass::Secret))
		{
			// The sound is already playing
			if(channels[cnum].sfxinfo == sfxinfo && channels[cnum].loop && params->loop)
			{
				channels[cnum].loop_timeout = params->loop_timeout;

				return channel_not_found;
			}

			S_StopChannel(cnum);
			break;
		}

	// Find an open channel
	for(cnum = 0; cnum < numChannels; cnum++)
		if(!channels[cnum].active)
			break;

	// None available
	if(cnum == numChannels)
	{
		// Look for lower priority
		channel_t temp_channel;

		memset(&temp_channel, 0, sizeof(temp_channel));
		temp_channel.priority = params->priority;
		temp_channel.volume = params->volume;

		cnum = S_LowestScoreChannel();

		if(cnum == channel_not_found)
			return channel_not_found;

		if(S_ChannelScore(&temp_channel) > S_ChannelScore(&channels[cnum]))
			S_StopChannel(cnum);
		else
			return channel_not_found;
	}

	c = &channels[cnum]; // channel is decided to be cnum.
	c->sfxinfo = sfxinfo;
	c->origin = origin;
	c->sfx_class = params->sfx_class;
	return cnum;
}

// heretic

static dboolean S_StopSoundInfo(sfxinfo_t* sfx, sfx_params_t* params)
{
	int i;
	int priority;
	int least_priority;
	int found;

	if(sfx->numchannels == -1)
		return true;

	priority = params->priority;
	least_priority = -1;
	found = 0;

	for(i = 0; i < numChannels; i++)
	{
		if(channels[i].active && channels[i].sfxinfo == sfx && channels[i].origin)
		{
			found++; //found one.  Now, should we replace it??
			if(priority >= channels[i].priority)
			{
				// if we're gonna kill one, then this'll be it
				if(!channels[i].loop || priority > channels[i].priority)
				{
					least_priority = i;
					priority = channels[i].priority;
				}
			}
		}
	}

	if(found < sfx->numchannels)
		return true;

	if(least_priority >= 0)
	{
		S_StopChannel(least_priority);

		return true;
	}

	return false; // don't replace any sounds
}

static int Raven_S_getChannel(mobj_t* listener, mobj_t* origin, sfxinfo_t* sfx, sfx_params_t* params)
{
	int i;
	static int sndcount = 0;

	for(i = 0; i < numChannels; i++)
	{
		// The sound is already playing
		if(channels[i].active &&
			channels[i].sfxinfo == sfx &&
			channels[i].origin == origin &&
			channels[i].loop && params->loop)
		{
			channels[i].loop_timeout = params->loop_timeout;

			return channel_not_found;
		}
	}

	if(!S_StopSoundInfo(sfx, params))
		return channel_not_found; // other sounds have greater priority

	for(i = 0; i < numChannels; i++)
	{
		if(gamestate != GameState::Level || origin == listener)
		{
			i = numChannels;
			break; // let the player have more than one sound.
		}
		if(origin == channels[i].origin)
		{
			// only allow other mobjs one sound
			S_StopSound(channels[i].origin);
			break;
		}
	}

	if(i >= numChannels)
	{
		// TODO: can ambient sounds even reach this flow?
		if(params->ambient)
		{
			if(AmbChan != -1 && sfx->priority <= channels[AmbChan].sfxinfo->priority)
				return channel_not_found; //ambient channel already in use

			AmbChan = -1;
		}

		for(i = 0; i < numChannels; i++)
			if(!channels[i].active)
				break;

		if(i >= numChannels)
		{
			int chan;

			//look for a lower priority sound to replace.
			sndcount++;
			if(sndcount >= numChannels)
				sndcount = 0;

			for(chan = 0; chan < numChannels; chan++)
			{
				i = (sndcount + chan) % numChannels;
				if(params->priority >= channels[i].priority)
				{
					chan = -1; //denote that sound should be replaced.
					break;
				}
			}

			if(chan != -1)
				return channel_not_found; //no free channels.

			S_StopChannel(i);
		}
	}

	return i;
}

static mobj_t* GetSoundListener()
{
	static degenmobj_t dummy_listener;

	// If we are at the title screen, the display player doesn't have an
	// object yet, so return a pointer to a static dummy listener instead.

	if(players[displayplayer].mo != nullptr)
	{
		if(walkcamera.type > 1)
		{
			static mobj_t walkcamera_listener;

			walkcamera_listener.x = walkcamera.x;
			walkcamera_listener.y = walkcamera.y;
			walkcamera_listener.z = walkcamera.z;
			walkcamera_listener.angle = walkcamera.angle;

			return &walkcamera_listener;
		}

		return players[displayplayer].mo;
	}
	else
	{
		dummy_listener.x = 0;
		dummy_listener.y = 0;
		dummy_listener.z = 0;

		return (mobj_t*)&dummy_listener;
	}
}

static void Raven_S_StartSoundAtVolume(void* _origin, SfxId sound_id, int volume, int loop_timeout)
{
	sfxinfo_t* sfx;
	mobj_t* origin;
	mobj_t* listener;
	sfx_params_t params;
	int dist;
	int cnum;
	angle_t angle;
	fixed_t absx;
	fixed_t absy;

	origin = (mobj_t*)_origin;
	listener = GetSoundListener();

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

	if(sound_id == SfxId::None)
		return;

	if(origin == nullptr)
		origin = listener;

	sfx = &S_sfx[std::to_underlying(sound_id)];

	params.ambient = heretic && sound_id >= SfxId::HereticWind;
	params.attenuation = 0;
	params.volume_factor = 0;
	params.loop = loop_timeout > 0;
	params.loop_timeout = loop_timeout;

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	absx = abs(origin->x - listener->x);
	absy = abs(origin->y - listener->y);
	dist = P_AproxDistance(absx, absy);
	dist >>= FRACBITS;

	if(dist >= max_snd_dist)
		return; //sound is beyond the hearing range...
	if(dist < 0)
		dist = 0;

	params.priority = sfx->priority;
	params.priority *= (10 - (dist / dist_adjust));

	if(sound_id == g_sfx_secret)
		params.sfx_class = SfxClass::Secret;
	else
		params.sfx_class = SfxClass::None;

	cnum = Raven_S_getChannel(listener, origin, sfx, &params);
	if(cnum == channel_not_found)
		return;

	if(sfx->lumpnum <= 0)
		sfx->lumpnum = I_GetSfxLumpNum(sfx);

	params.volume = (soundCurve[dist] * volume * sfx_volume * 8) >> 14;

	if(origin == listener)
		params.separation = 128;
	else
	{
		angle = R_PointToAngle2(listener->x, listener->y, origin->x, origin->y);
		if(angle <= listener->angle)
			angle += 0xffffffff;
		angle -= listener->angle;
		angle >>= ANGLETOFINESHIFT;

		// stereo separation
		params.separation = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);
	}

	if(!hexen || sfx->pitch)
	{
		params.pitch = (byte)(NORM_PITCH + (M_Random() & 7) - (M_Random() & 7));
	}
	else
	{
		params.pitch = NORM_PITCH;
	}

	channels[cnum].pitch = params.pitch;
	channels[cnum].handle = I_StartSound(sound_id, cnum, &params);
	channels[cnum].origin = origin;
	channels[cnum].sfxinfo = sfx;
	channels[cnum].priority = params.priority;
	channels[cnum].volume = volume; // original volume, not attenuated volume
	channels[cnum].ambient = params.ambient;
	channels[cnum].attenuation = params.attenuation;
	channels[cnum].volume_factor = params.volume_factor;
	channels[cnum].loop = params.loop;
	channels[cnum].loop_timeout = params.loop_timeout;
	channels[cnum].active = true;
	if(channels[cnum].ambient) // TODO: can ambient sounds even reach this flow?
		AmbChan = cnum;
}

void S_StartAmbientSound(void* _origin, SfxId sound_id, int volume)
{
	sfxinfo_t* sfx;
	sfx_params_t params;
	mobj_t* origin;
	mobj_t* listener;
	int i;

	origin = (mobj_t*)_origin;
	listener = GetSoundListener();

	if(nosfxparm)
		return;

	if(sound_id == SfxId::None || volume == 0)
		return;

	if(origin == nullptr)
		origin = listener;

	sfx = &S_sfx[std::to_underlying(sound_id)];

	if(sfx_volume > 0)
		params.volume = (volume * (sfx_volume + 1) * 8) >> 7;
	else
		params.volume = 0;

	params.pitch = (byte)(NORM_PITCH - (M_Random() & 3) + (M_Random() & 3));
	params.priority = 1; // super low priority
	params.separation = 128;
	params.sfx_class = SfxClass::None;
	params.ambient = true;
	params.attenuation = 0;
	params.volume_factor = 0;
	params.loop = false;
	params.loop_timeout = 0;

	// no priority checking, as ambient sounds would be the LOWEST.
	for(i = 0; i < numChannels; i++)
		if(channels[i].origin == nullptr)
			break;

	if(i >= numChannels)
		return;

	if(sfx->lumpnum <= 0)
		sfx->lumpnum = I_GetSfxLumpNum(sfx);

	channels[i].pitch = params.pitch;
	channels[i].handle = I_StartSound(sound_id, i, &params);
	channels[i].origin = origin;
	channels[i].sfxinfo = sfx;
	channels[i].priority = params.priority;
	channels[i].ambient = params.ambient;
	channels[i].attenuation = params.attenuation;
	channels[i].volume_factor = params.volume_factor;
	channels[i].loop = params.loop;
	channels[i].loop_timeout = params.loop_timeout;
	channels[i].active = true;
}

static void Heretic_S_StopSound(void* _origin)
{
	mobj_t* origin = static_cast<mobj_t *>(_origin);
	int i;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return;

	for(i = 0; i < numChannels; i++)
	{
		if(channels[i].active && channels[i].origin == origin)
		{
			S_StopChannel(i);
		}
	}
}

// hexen

dboolean S_GetSoundPlayingInfo(void* origin, SfxId sound_id)
{
	int i;
	sfxinfo_t* sfx;

	//jff 1/22/98 return if sound is not enabled
	if(nosfxparm)
		return false;

	sfx = &S_sfx[std::to_underlying(sound_id)];

	for(i = 0; i < numChannels; i++)
	{
		if(channels[i].active && channels[i].sfxinfo == sfx && channels[i].origin == origin)
		{
			if(I_SoundIsPlaying(channels[i].handle))
			{
				return true;
			}
		}
	}
	return false;
}

SfxId S_GetSoundID(const char* name)
{
	int i;

	for(i = 0; i < num_sfx; i++)
	{
		if(!strcmp(S_sfx[i].tagname, name))
		{
			return static_cast<SfxId>(i);
		}
	}
	return SfxId::None;
}

void S_StartSongName(const char* songLump, dboolean loop)
{
	MusicId musicnum;

	// lazy shortcut hack - this is a unique character
	switch(songLump[1])
	{
		case 'e':
			musicnum = MusicId::HexenHexen;
			break;
		case 'u':
			musicnum = MusicId::HexenHub;
			break;
		case 'a':
			musicnum = MusicId::HexenHall;
			break;
		case 'r':
			musicnum = MusicId::HexenOrb;
			break;
		case 'h':
			musicnum = MusicId::HexenChess;
			break;
		default:
			musicnum = MusicId::HexenHub;
			break;
	}

	S_ChangeMusic(musicnum, loop);
}
