// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//    Reading of MIDI files.

#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef TEST
#include "doomdef.hpp"
#include "doomtype.hpp"
#else
using dboolean = bool;
typedef unsigned char byte;
#define PACKEDATTR __attribute__((packed))
#endif
#include "lprintf.hpp"
#include "midifile.hpp"

#define HEADER_CHUNK_ID "MThd"
#define TRACK_CHUNK_ID  "MTrk"
#define MAX_BUFFER_SIZE 0x10000



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef ntohl
#ifdef WORDS_BIGENDIAN
#define ntohl
#define ntohs
#else // WORDS_BIGENDIAN

#include <stdint.h>

#define ntohl(x) \
        ((((uint32_t)(x) & 0x000000ffU) << 24) | \
          (((uint32_t)(x) & 0x0000ff00U) <<  8) | \
          (((uint32_t)(x) & 0x00ff0000U) >>  8) | \
          (((uint32_t)(x) & 0xff000000U) >> 24))

#define ntohs(x) \
        ((((uint16_t)(x) & 0x00ff) << 8) | \
         (((uint16_t)(x) & 0xff00) >> 8))
#endif // WORDS_BIGENDIAN
#endif // ntohl


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct
{
	byte chunk_id[4];
	unsigned int chunk_size;
}
	PACKEDATTR chunk_header_t;

typedef struct
{
	chunk_header_t chunk_header;
	unsigned short format_type;
	unsigned short num_tracks;
	unsigned short time_division;
}
	PACKEDATTR midi_header_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct
{
	// Length in bytes:

	unsigned int data_len;

	// Events in this track:

	midi_event_t* events;
	unsigned int num_events;
	unsigned int num_event_mem; // NSM track size of structure
} midi_track_t;

struct midi_track_iter_s
{
	midi_track_t* track;
	unsigned int position;
};

struct midi_file_s
{
	midi_header_t header;

	// All tracks in this file:
	midi_track_t* tracks;
	unsigned int num_tracks;

	// Data buffer used to store data read for SysEx or meta events:
	byte* buffer;
	unsigned int buffer_size;
};



// Check the header of a chunk:

static dboolean CheckChunkHeader(chunk_header_t* chunk,
	const char* expected_id)
{
	dboolean result;

	result = (memcmp((char*)chunk->chunk_id, expected_id, 4) == 0);

	if(!result)
	{
		Log::Warn("CheckChunkHeader: Expected '{}' chunk header, "
			"got '{}{}{}{}'\n",
			expected_id,
			static_cast<char>(chunk->chunk_id[0]), static_cast<char>(chunk->chunk_id[1]),
			static_cast<char>(chunk->chunk_id[2]), static_cast<char>(chunk->chunk_id[3]));
	}

	return result;
}

// Read a single byte.  Returns false on error.

static dboolean ReadByte(byte* result, midimem_t* mf)
{
	if(mf->pos >= mf->len)
	{
		Log::Warn("ReadByte: Unexpected end of file\n");
		return false;
	}

	*result = mf->data[mf->pos++];
	return true;
}

static dboolean ReadMultipleBytes(void* dest, size_t len, midimem_t* mf)
{
	byte* cdest = (byte*)dest;
	unsigned i;
	for(i = 0; i < len; i++)
	{
		if(!ReadByte(cdest + i, mf))
		{
			Log::Warn("ReadMultipleBytes: Unexpected end of file\n");
			return false;
		}
	}
	return true;
}

// Read a variable-length value.

static dboolean ReadVariableLength(unsigned int* result, midimem_t* mf)
{
	int i;
	byte b;

	*result = 0;

	for(i = 0; i < 4; ++i)
	{
		if(!ReadByte(&b, mf))
		{
			Log::Warn("ReadVariableLength: Error while reading "
				"variable-length value\n");
			return false;
		}

		// Insert the bottom seven bits from this byte.

		*result <<= 7;
		*result |= b & 0x7f;

		// If the top bit is not set, this is the end.

		if((b & 0x80) == 0)
		{
			return true;
		}
	}

	Log::Warn("ReadVariableLength: Variable-length value too "
		"long: maximum of four bytes\n");
	return false;
}

// Read a byte sequence into the data buffer.

static void* ReadByteSequence(unsigned int num_bytes, midimem_t* mf)
{
	unsigned int i;
	byte* result;

	// events can be length 0.  malloc(0) is not portable (can return nullptr)
	if(!num_bytes)
		return Z_Malloc(4);

	// Allocate a buffer:

	result = (byte*)Z_Malloc(num_bytes);

	if(result == nullptr)
	{
		Log::Warn("ReadByteSequence: Failed to allocate buffer {} bytes\n", num_bytes);
		return nullptr;
	}

	// Read the data:

	for(i = 0; i < num_bytes; ++i)
	{
		if(!ReadByte(&result[i], mf))
		{
			Log::Warn("ReadByteSequence: Error while reading byte {}\n", i);
			Z_Free(result);
			return nullptr;
		}
	}

	return result;
}

// Read a MIDI channel event.
// two_param indicates that the event type takes two parameters
// (three byte) otherwise it is single parameter (two byte)

static dboolean ReadChannelEvent(midi_event_t* event,
	byte event_type, dboolean two_param,
	midimem_t* mf)
{
	byte b;

	// Set basics:

	event->event_type = (MidiEventType)(event_type & 0xf0);
	event->data.channel.channel = event_type & 0x0f;

	// Read parameters:

	if(!ReadByte(&b, mf))
	{
		Log::Warn("ReadChannelEvent: Error while reading channel "
			"event parameters\n");
		return false;
	}

	event->data.channel.param1 = b;

	// Second parameter:

	if(two_param)
	{
		if(!ReadByte(&b, mf))
		{
			Log::Warn("ReadChannelEvent: Error while reading channel "
				"event parameters\n");
			return false;
		}

		event->data.channel.param2 = b;
	}

	return true;
}

// Read sysex event:

static dboolean ReadSysExEvent(midi_event_t* event, int event_type,
	midimem_t* mf)
{
	event->event_type = (MidiEventType)event_type;

	if(!ReadVariableLength(&event->data.sysex.length, mf))
	{
		Log::Warn("ReadSysExEvent: Failed to read length of "
			"SysEx block\n");
		return false;
	}

	// Read the byte sequence:

	event->data.sysex.data = (byte*)ReadByteSequence(event->data.sysex.length, mf);

	if(event->data.sysex.data == nullptr)
	{
		Log::Warn("ReadSysExEvent: Failed while reading SysEx event\n");
		return false;
	}

	return true;
}

// Read meta event:

static dboolean ReadMetaEvent(midi_event_t* event, midimem_t* mf)
{
	byte b;

	event->event_type = MidiEventType::Meta;

	// Read meta event type:

	if(!ReadByte(&b, mf))
	{
		Log::Warn("ReadMetaEvent: Failed to read meta event type\n");
		return false;
	}

	event->data.meta.type = b;

	// Read length of meta event data:

	if(!ReadVariableLength(&event->data.meta.length, mf))
	{
		Log::Warn("ReadMetaEvent: Failed to read length of "
			"MetaEvent block\n");
		return false;
	}

	// Read the byte sequence:

	event->data.meta.data = (byte*)ReadByteSequence(event->data.meta.length, mf);

	if(event->data.meta.data == nullptr)
	{
		Log::Warn("ReadMetaEvent: Failed while reading MetaEvent\n");
		return false;
	}

	return true;
}

static dboolean ReadEvent(midi_event_t* event, unsigned int* last_event_type,
	midimem_t* mf)
{
	byte event_type;

	if(!ReadVariableLength(&event->delta_time, mf))
	{
		Log::Warn("ReadEvent: Failed to read event timestamp\n");
		return false;
	}

	if(!ReadByte(&event_type, mf))
	{
		Log::Warn("ReadEvent: Failed to read event type\n");
		return false;
	}

	// All event types have their top bit set.  Therefore, if
	// the top bit is not set, it is because we are using the "same
	// as previous event type" shortcut to save a byte.  Skip back
	// a byte so that we read this byte again.

	if((event_type & 0x80) == 0)
	{
		event_type = *last_event_type;
		mf->pos--;
	}
	else
	{
		*last_event_type = event_type;
	}

	// Check event type:

	switch(static_cast<MidiEventType>(event_type & 0xf0))
	{
		// Two parameter channel events:

		case MidiEventType::NoteOff:
		case MidiEventType::NoteOn:
		case MidiEventType::Aftertouch:
		case MidiEventType::Controller:
		case MidiEventType::End:
			return ReadChannelEvent(event, event_type, true, mf);

		// Single parameter channel events:

		case MidiEventType::ProgramChange:
		case MidiEventType::ChanAftertouch:
			return ReadChannelEvent(event, event_type, false, mf);

		default:
			break;
	}

	// Specific value?

	switch(static_cast<MidiEventType>(event_type))
	{
		case MidiEventType::Sysex:
		case MidiEventType::SysexSplit:
			return ReadSysExEvent(event, event_type, mf);

		case MidiEventType::Meta:
			return ReadMetaEvent(event, mf);

		default:
			break;
	}

	Log::Warn("ReadEvent: Unknown MIDI event type: 0x{:x}\n", event_type);
	return false;
}

// Free an event:

static void FreeEvent(midi_event_t* event)
{
	// Some event types have dynamically allocated buffers assigned
	// to them that must be freed.

	switch(event->event_type)
	{
		case MidiEventType::Sysex:
		case MidiEventType::SysexSplit:
			Z_Free(event->data.sysex.data);
			break;

		case MidiEventType::Meta:
			Z_Free(event->data.meta.data);
			break;

		default:
			// Nothing to do.
			break;
	}
}

// Read and check the track chunk header

static dboolean ReadTrackHeader(midi_track_t* track, midimem_t* mf)
{
	size_t records_read;
	chunk_header_t chunk_header;

	records_read = ReadMultipleBytes(&chunk_header, sizeof(chunk_header_t), mf);

	if(records_read < 1)
	{
		return false;
	}

	if(!CheckChunkHeader(&chunk_header, TRACK_CHUNK_ID))
	{
		return false;
	}

	track->data_len = ntohl(chunk_header.chunk_size);

	return true;
}

static dboolean ReadTrack(midi_track_t* track, midimem_t* mf)
{
	midi_event_t* new_events = nullptr;
	midi_event_t* event;
	unsigned int last_event_type;

	track->num_events = 0;
	track->events = nullptr;
	track->num_event_mem = 0; // NSM

	// Read the header:

	if(!ReadTrackHeader(track, mf))
	{
		return false;
	}

	// Then the events:

	last_event_type = 0;

	for(;;)
	{
		// Resize the track slightly larger to hold another event:
		/*
		new_events = static_cast<midi_event_t*>(Z_Realloc(track->events,
							sizeof(midi_event_t) * (track->num_events + 1)));
		*/
		if(track->num_events == track->num_event_mem)
		{
			// depending on the state of the heap and the malloc implementation, realloc()
			// one more event at a time can be VERY slow.  10sec+ in MSVC
			track->num_event_mem += 100;
			new_events = (midi_event_t*)Z_Realloc(track->events, sizeof(midi_event_t) * track->num_event_mem);
		}

		if(new_events == nullptr)
		{
			return false;
		}

		track->events = new_events;

		// Read the next event:

		event = &track->events[track->num_events];
		if(!ReadEvent(event, &last_event_type, mf))
		{
			return false;
		}

		++track->num_events;

		// End of track?

		if(event->event_type == MidiEventType::Meta
			&& event->data.meta.type == std::to_underlying(MidiMetaEventType::EndOfTrack))
		{
			break;
		}
	}

	return true;
}

// Free a track:

static void FreeTrack(midi_track_t* track)
{
	unsigned i;

	for(i = 0; i < track->num_events; ++i)
	{
		FreeEvent(&track->events[i]);
	}

	Z_Free(track->events);
}

static dboolean ReadAllTracks(midi_file_t* file, midimem_t* mf)
{
	unsigned int i;

	// Allocate list of tracks and read each track:

	file->tracks = (midi_track_t*)Z_Malloc(sizeof(midi_track_t) * file->num_tracks);

	if(file->tracks == nullptr)
	{
		return false;
	}

	memset(file->tracks, 0, sizeof(midi_track_t) * file->num_tracks);

	// Read each track:

	for(i = 0; i < file->num_tracks; ++i)
	{
		if(!ReadTrack(&file->tracks[i], mf))
		{
			return false;
		}
	}

	return true;
}

// Read and check the header chunk.

static dboolean ReadFileHeader(midi_file_t* file, midimem_t* mf)
{
	size_t records_read;
	unsigned int format_type;

	records_read = ReadMultipleBytes(&file->header, sizeof(midi_header_t), mf);

	if(records_read < 1)
	{
		return false;
	}

	if(!CheckChunkHeader(&file->header.chunk_header, HEADER_CHUNK_ID)
		|| ntohl(file->header.chunk_header.chunk_size) != 6)
	{
		Log::Warn("ReadFileHeader: Invalid MIDI chunk header! "
			"chunk_size={}\n",
			ntohl(file->header.chunk_header.chunk_size));
		return false;
	}

	format_type = ntohs(file->header.format_type);
	file->num_tracks = ntohs(file->header.num_tracks);

	if((format_type != 0 && format_type != 1)
		|| file->num_tracks < 1)
	{
		Log::Warn("ReadFileHeader: Only type 0/1 "
			"MIDI files supported!\n");
		return false;
	}
	// NSM
	file->header.time_division = ntohs(file->header.time_division);


	return true;
}

void MIDI_FreeFile(midi_file_t* file)
{
	unsigned i;

	if(file->tracks != nullptr)
	{
		for(i = 0; i < file->num_tracks; ++i)
		{
			FreeTrack(&file->tracks[i]);
		}

		Z_Free(file->tracks);
	}

	Z_Free(file);
}

midi_file_t* MIDI_LoadFile(midimem_t* mf)
{
	midi_file_t* file;

	file = (midi_file_t*)Z_Malloc(sizeof(midi_file_t));

	if(file == nullptr)
	{
		return nullptr;
	}

	file->tracks = nullptr;
	file->num_tracks = 0;
	file->buffer = nullptr;
	file->buffer_size = 0;

	// Read MIDI file header

	if(!ReadFileHeader(file, mf))
	{
		MIDI_FreeFile(file);
		return nullptr;
	}

	// Read all tracks:

	if(!ReadAllTracks(file, mf))
	{
		MIDI_FreeFile(file);
		return nullptr;
	}

	return file;
}

// Get the number of tracks in a MIDI file.

unsigned int MIDI_NumTracks(const midi_file_t* file)
{
	return file->num_tracks;
}

// Start iterating over the events in a track.

midi_track_iter_t* MIDI_IterateTrack(const midi_file_t* file, unsigned int track)
{
	midi_track_iter_t* iter;

	assert(track < file->num_tracks);

	iter = (midi_track_iter_t*)Z_Malloc(sizeof(*iter));
	iter->track = &file->tracks[track];
	iter->position = 0;

	return iter;
}

void MIDI_FreeIterator(midi_track_iter_t* iter)
{
	Z_Free(iter);
}

// Get the time until the next MIDI event in a track.

unsigned int MIDI_GetDeltaTime(midi_track_iter_t* iter)
{
	if(iter->position < iter->track->num_events)
	{
		midi_event_t* next_event;

		next_event = &iter->track->events[iter->position];

		return next_event->delta_time;
	}
	else
	{
		return 0;
	}
}

// Get a pointer to the next MIDI event.

int MIDI_GetNextEvent(midi_track_iter_t* iter, midi_event_t** event)
{
	if(iter->position < iter->track->num_events)
	{
		*event = &iter->track->events[iter->position];
		++iter->position;

		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int MIDI_GetFileTimeDivision(const midi_file_t* file)
{
	return file->header.time_division;
}

void MIDI_RestartIterator(midi_track_iter_t* iter)
{
	iter->position = 0;
}



static void MIDI_PrintFlatListDBG(const midi_event_t** evs)
{
	const midi_event_t* event;

	while(1)
	{
		event = *evs++;

		if(event->delta_time > 0)
			printf("Delay: %i ticks\n", event->delta_time);


		switch(event->event_type)
		{
			case MidiEventType::NoteOff:
				printf("MIDI_EVENT_NOTE_OFF\n");
				break;
			case MidiEventType::NoteOn:
				printf("MIDI_EVENT_NOTE_ON\n");
				break;
			case MidiEventType::Aftertouch:
				printf("MIDI_EVENT_AFTERTOUCH\n");
				break;
			case MidiEventType::Controller:
				printf("MIDI_EVENT_CONTROLLER\n");
				break;
			case MidiEventType::ProgramChange:
				printf("MIDI_EVENT_PROGRAM_CHANGE\n");
				break;
			case MidiEventType::ChanAftertouch:
				printf("MIDI_EVENT_CHAN_AFTERTOUCH\n");
				break;
			case MidiEventType::End:
				printf("MIDI_EVENT_PITCH_BEND\n");
				break;
			case MidiEventType::Sysex:
				printf("MIDI_EVENT_SYSEX\n");
				break;
			case MidiEventType::SysexSplit:
				printf("MIDI_EVENT_SYSEX_SPLIT\n");
				break;
			case MidiEventType::Meta:
				printf("MIDI_EVENT_META\n");
				break;

			default:
				printf("(unknown)\n");
				break;
		}
		switch(event->event_type)
		{
			case MidiEventType::NoteOff:
			case MidiEventType::NoteOn:
			case MidiEventType::Aftertouch:
			case MidiEventType::Controller:
			case MidiEventType::ProgramChange:
			case MidiEventType::ChanAftertouch:
			case MidiEventType::End:
				printf("\tChannel: %i\n", event->data.channel.channel);
				printf("\tParameter 1: %i\n", event->data.channel.param1);
				printf("\tParameter 2: %i\n", event->data.channel.param2);
				break;

			case MidiEventType::Sysex:
			case MidiEventType::SysexSplit:
				printf("\tLength: %i\n", event->data.sysex.length);
				break;

			case MidiEventType::Meta:
				printf("\tMeta type: %i\n", event->data.meta.type);
				printf("\tLength: %i\n", event->data.meta.length);
				break;
		}
		if(event->event_type == MidiEventType::Meta &&
			event->data.meta.type == std::to_underlying(MidiMetaEventType::EndOfTrack))
		{
			printf("gotta go!\n");
			return;
		}
	}
}



// NSM: an alternate iterator tool.

midi_event_t** MIDI_GenerateFlatList(midi_file_t* file)
{
	int totalevents = 0;
	unsigned i, delta;
	int nextrk;

	int totaldelta = 0;

	int* trackpos = (int*)Z_Calloc(file->num_tracks, sizeof(int));
	int* tracktime = (int*)Z_Calloc(file->num_tracks, sizeof(int));
	int trackactive = file->num_tracks;

	midi_event_t** ret;
	midi_event_t** epos;

	for(i = 0; i < file->num_tracks; i++)
		totalevents += file->tracks[i].num_events;

	ret = (midi_event_t**)Z_Malloc(totalevents * sizeof(midi_event_t**));

	epos = ret;

	while(trackactive)
	{
		delta = 0x10000000;
		nextrk = -1;
		for(i = 0; i < file->num_tracks; i++)
		{
			if(trackpos[i] != -1 && file->tracks[i].events[trackpos[i]].delta_time - tracktime[i] < delta)
			{
				delta = file->tracks[i].events[trackpos[i]].delta_time - tracktime[i];
				nextrk = i;
			}
		}
		if(nextrk == -1) // unexpected EOF (not every track got end track)
			break;

		*epos = file->tracks[nextrk].events + trackpos[nextrk];

		for(i = 0; i < file->num_tracks; i++)
		{
			if(i == (unsigned)nextrk)
			{
				tracktime[i] = 0;
				trackpos[i]++;
			}
			else
				tracktime[i] += delta;
		}
		// yes, this clobbers the original timecodes
		epos[0]->delta_time = delta;
		totaldelta += delta;

		if(epos[0]->event_type == MidiEventType::Meta
			&& epos[0]->data.meta.type == std::to_underlying(MidiMetaEventType::EndOfTrack))
		{
			// change end of track into no op
			trackactive--;
			trackpos[nextrk] = -1;
			epos[0]->data.meta.type = std::to_underlying(MidiMetaEventType::Text);
		}
		else if((unsigned)trackpos[nextrk] == file->tracks[nextrk].num_events)
		{
			Log::Warn("MIDI_GenerateFlatList: Unexpected end of track\n");
			Z_Free(trackpos);
			Z_Free(tracktime);
			Z_Free(ret);
			return nullptr;
		}
		epos++;
	}

	if(trackactive)
	{
		// unexpected EOF
		Log::Warn("MIDI_GenerateFlatList: Unexpected end of midi file\n");
		Z_Free(trackpos);
		Z_Free(tracktime);
		Z_Free(ret);
		return nullptr;
	}

	// last end of track event is preserved though
	epos[-1]->data.meta.type = std::to_underlying(MidiMetaEventType::EndOfTrack);

	Z_Free(trackpos);
	Z_Free(tracktime);

	if(totaldelta < 100)
	{
		Log::Warn("MIDI_GeneratFlatList: very short file {}\n", totaldelta);
		Z_Free(ret);
		return nullptr;
	}


	//MIDI_PrintFlatListDBG (ret);

	return ret;
}



void MIDI_DestroyFlatList(midi_event_t** evs)
{
	Z_Free(evs);
}



// NSM: timing assist functions
// they compute samples per midi clock, where midi clock is the units
// that the time deltas are in, and samples is an arbitrary unit of which
// "sndrate" of them occur per second


static double compute_spmc_normal(unsigned mpq, unsigned tempo, unsigned sndrate)
{
	// returns samples per midi clock

	// inputs: mpq (midi clocks per quarternote, from header)
	// tempo (from tempo event, in microseconds per quarternote)
	// sndrate (sound sample rate in hz)

	// samples   quarternote     microsec    samples    second
	// ------- = ----------- * ----------- * ------- * --------
	// midiclk     midiclk     quarternote   second    microsec

	// return  =  (1 / mpq)  *    tempo    * sndrate * (1 / 1000000)

	return (double)tempo / 1000000 * sndrate / mpq;
}

static double compute_spmc_smpte(unsigned smpte_fps, unsigned mpf, unsigned sndrate)
{
	// returns samples per midi clock

	// inputs: smpte_fps (24, 25, 29, 30)
	// mpf (midi clocks per frame, 0-255)
	// sndrate (sound sample rate in hz)

	// tempo is ignored here

	// samples     frame      seconds    samples
	// ------- = --------- * --------- * -------
	// midiclk    midiclk      frame     second

	// return  = (1 / mpf) * (1 / fps) * sndrate


	double fps; // actual frames per second
	switch(smpte_fps)
	{
		case 24:
		case 25:
		case 30:
			fps = smpte_fps;
			break;
		case 29:
			// i hate NTSC, i really do
			fps = smpte_fps * 1000.0 / 1001.0;
			break;
		default:
			Log::Warn("MIDI_spmc: Unexpected SMPTE timestamp {}\n", smpte_fps);
			// assume
			fps = 30.0;
			break;
	}

	return (double)sndrate / fps / mpf;
}

// if event is nullptr, compute with default starting tempo (120BPM)

double MIDI_spmc(const midi_file_t* file, const midi_event_t* ev, unsigned sndrate)
{
	int smpte_fps;
	unsigned tempo;
	unsigned headerval = MIDI_GetFileTimeDivision(file);

	if(headerval & 0x8000) // SMPTE
	{
		// i don't really have any files to test this on...
		smpte_fps = -(short)headerval >> 8;
		return compute_spmc_smpte(smpte_fps, headerval & 0xff, sndrate);
	}

	// normal timing
	tempo = 500000; // default 120BPM
	if(ev)
	{
		if(ev->event_type == MidiEventType::Meta)
		{
			if(ev->data.meta.length == 3)
			{
				tempo = (unsigned)ev->data.meta.data[0] << 16 |
					(unsigned)ev->data.meta.data[1] << 8 |
					(unsigned)ev->data.meta.data[2];
			}
			else
				Log::Warn("MIDI_spmc: wrong length tempo meta message in midi file\n");
		}
		else
			Log::Warn("MIDI_spmc: passed non-meta event\n");
	}

	return compute_spmc_normal(headerval, tempo, sndrate);
}

#ifdef TEST

static char* MIDI_EventTypeToString(MidiEventType event_type)
{
	switch(event_type)
	{
		case MidiEventType::NoteOff:
			return "MIDI_EVENT_NOTE_OFF";
		case MidiEventType::NoteOn:
			return "MIDI_EVENT_NOTE_ON";
		case MidiEventType::Aftertouch:
			return "MIDI_EVENT_AFTERTOUCH";
		case MidiEventType::Controller:
			return "MIDI_EVENT_CONTROLLER";
		case MidiEventType::ProgramChange:
			return "MIDI_EVENT_PROGRAM_CHANGE";
		case MidiEventType::ChanAftertouch:
			return "MIDI_EVENT_CHAN_AFTERTOUCH";
		case MidiEventType::End:
			return "MIDI_EVENT_PITCH_BEND";
		case MidiEventType::Sysex:
			return "MIDI_EVENT_SYSEX";
		case MidiEventType::SysexSplit:
			return "MIDI_EVENT_SYSEX_SPLIT";
		case MidiEventType::Meta:
			return "MIDI_EVENT_META";

		default:
			return "(unknown)";
	}
}

void PrintTrack(midi_track_t* track)
{
	midi_event_t* event;
	unsigned int i;

	for(i = 0; i < track->num_events; ++i)
	{
		event = &track->events[i];

		if(event->delta_time > 0)
		{
			printf("Delay: %i ticks\n", event->delta_time);
		}

		printf("Event type: %s (%i)\n",
			MIDI_EventTypeToString(event->event_type),
			event->event_type);

		switch(event->event_type)
		{
			case MidiEventType::NoteOff:
			case MidiEventType::NoteOn:
			case MidiEventType::Aftertouch:
			case MidiEventType::Controller:
			case MidiEventType::ProgramChange:
			case MidiEventType::ChanAftertouch:
			case MidiEventType::End:
				printf("\tChannel: %i\n", event->data.channel.channel);
				printf("\tParameter 1: %i\n", event->data.channel.param1);
				printf("\tParameter 2: %i\n", event->data.channel.param2);
				break;

			case MidiEventType::Sysex:
			case MidiEventType::SysexSplit:
				printf("\tLength: %i\n", event->data.sysex.length);
				break;

			case MidiEventType::Meta:
				printf("\tMeta type: %i\n", event->data.meta.type);
				printf("\tLength: %i\n", event->data.meta.length);
				break;
		}
	}
}

int main(int argc, char* argv[])
{
	FILE* f;
	midimem_t mf;
	midi_file_t* file;
	unsigned int i;

	if(argc < 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	f = fopen(argv[1], "rb");
	if(!f)
	{
		fprintf(stderr, "Failed to open %s\n", argv[1]);
		exit(1);
	}
	fseek(f, 0, SEEK_END);
	mf.len = ftell(f);
	mf.pos = 0;
	rewind(f);
	mf.data = static_cast<decltype(mf.data)>(Z_Malloc(mf.len));
	fread(mf.data, 1, mf.len, f);
	fclose(f);

	file = MIDI_LoadFile(&mf);

	if(file == nullptr)
	{
		fprintf(stderr, "Failed to open %s\n", argv[1]);
		exit(1);
	}

	for(i = 0; i < file->num_tracks; ++i)
	{
		printf("\n== Track %i ==\n\n", i);

		PrintTrack(&file->tracks[i]);
	}

	return 0;
}

#endif
