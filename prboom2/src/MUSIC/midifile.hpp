// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//     MIDI file parsing.

#pragma once

#include "doomtype.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct midi_file_s midi_file_t;
typedef struct midi_track_iter_s midi_track_iter_t;

#define MIDI_CHANNELS_PER_TRACK 16

typedef struct
{
	const byte* data;
	size_t len;
	size_t pos;
} midimem_t;

enum struct MidiEventType : int32_t
{
	NoteOff        = 0x80,
	NoteOn         = 0x90,
	Aftertouch      = 0xa0,
	Controller      = 0xb0,
	ProgramChange  = 0xc0,
	ChanAftertouch = 0xd0,
	End      = 0xe0,

	Sysex       = 0xf0,
	SysexSplit = 0xf7,
	Meta        = 0xff,
};

enum struct MidiController : int32_t
{
	BankSelect    = 0x0,
	Modulation     = 0x1,
	BreathControl = 0x2,
	FootControl   = 0x3,
	Portamento     = 0x4,
	DataEntry     = 0x5,

	MainVolume = 0x7,
	Pan         = 0xa,

	AllNotesOff = 0x7b
};

enum struct MidiMetaEventType : int32_t
{
	SequenceNumber = 0x0,

	Text       = 0x1,
	Copyright  = 0x2,
	TrackName = 0x3,
	InstrName = 0x4,
	Lyrics     = 0x5,
	Marker     = 0x6,
	CuePoint  = 0x7,

	ChannelPrefix = 0x20,
	EndOfTrack   = 0x2f,

	SetTempo          = 0x51,
	SmpteOffset       = 0x54,
	TimeSignature     = 0x58,
	KeySignature      = 0x59,
	SequencerSpecific = 0x7f,
};

typedef struct
{
	// Meta event type:

	unsigned int type;

	// Length:

	unsigned int length;

	// Meta event data:

	byte* data;
} midi_meta_event_data_t;

typedef struct
{
	// Length:

	unsigned int length;

	// Event data:

	byte* data;
} midi_sysex_event_data_t;

typedef struct
{
	// The channel number to which this applies:

	unsigned int channel;

	// Extra parameters:

	unsigned int param1;
	unsigned int param2;
} midi_channel_event_data_t;

typedef struct
{
	// Time between the previous event and this event.
	unsigned int delta_time;

	// Type of event:
	MidiEventType event_type;

	union
	{
		midi_channel_event_data_t channel;
		midi_meta_event_data_t meta;
		midi_sysex_event_data_t sysex;
	} data;
} midi_event_t;

// Load a MIDI file.

midi_file_t* MIDI_LoadFile(midimem_t* mf);

// Free a MIDI file.

void MIDI_FreeFile(midi_file_t* file);

// Get the time division value from the MIDI header.

unsigned int MIDI_GetFileTimeDivision(const midi_file_t* file);

// Get the number of tracks in a MIDI file.

unsigned int MIDI_NumTracks(const midi_file_t* file);

// Start iterating over the events in a track.

midi_track_iter_t* MIDI_IterateTrack(const midi_file_t* file, unsigned int track_num);

// Free an iterator.

void MIDI_FreeIterator(midi_track_iter_t* iter);

// Get the time until the next MIDI event in a track.

unsigned int MIDI_GetDeltaTime(midi_track_iter_t* iter);

// Get a pointer to the next MIDI event.

int MIDI_GetNextEvent(midi_track_iter_t* iter, midi_event_t** event);

// Reset an iterator to the beginning of a track.

void MIDI_RestartIterator(midi_track_iter_t* iter);

// NSM: an alternate iterator tool.
midi_event_t** MIDI_GenerateFlatList(midi_file_t* file);
void MIDI_DestroyFlatList(midi_event_t** evs);

// NSM: timing calculator
double MIDI_spmc(const midi_file_t* file, const midi_event_t* ev, unsigned sndrate);

#ifdef __cplusplus
}
#endif
