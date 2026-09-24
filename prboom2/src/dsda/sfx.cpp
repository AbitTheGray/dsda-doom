// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA SFX

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomtype.hpp"

#include "dsda/configuration.hpp"

#include "sfx.hpp"

sfxinfo_t* S_sfx;
int num_sfx;
static int deh_soundnames_size;
static char** deh_soundnames;
static byte* sfx_state;
static int highest_index;

static void dsda_ResetSFX(int from, int to)
{
	int i;

	for(i = from; i < to; ++i)
	{
		S_sfx[i].priority = 127;
		S_sfx[i].pitch = -1;
	}
}

static void dsda_PrepAllocation()
{
	static int first_allocation = true;

	if(first_allocation)
	{
		sfxinfo_t* source = S_sfx;

		first_allocation = false;
		S_sfx = static_cast<sfxinfo_t*>(malloc(num_sfx * sizeof(*S_sfx)));
		memcpy(S_sfx, source, num_sfx * sizeof(*S_sfx));
	}
}

static void dsda_EnsureCapacity(int limit)
{
	while(limit >= num_sfx)
	{
		int old_num_sfx = num_sfx;

		dsda_PrepAllocation();

		num_sfx *= 2;

		S_sfx = static_cast<sfxinfo_t*>(realloc(S_sfx, num_sfx * sizeof(*S_sfx)));
		memset(S_sfx + old_num_sfx, 0, (num_sfx - old_num_sfx) * sizeof(*S_sfx));

		if(sfx_state)
		{
			sfx_state = static_cast<byte*>(realloc(sfx_state, num_sfx * sizeof(*sfx_state)));
			memset(sfx_state + old_num_sfx, 0,
				(num_sfx - old_num_sfx) * sizeof(*sfx_state));
		}

		dsda_ResetSFX(old_num_sfx, num_sfx);
	}
}

int dsda_GetDehSFXIndex(const char* key, size_t length)
{
	int i;

	// offset "ds" for dehacked names
	for(i = 1; i < num_sfx; ++i)
		if(
			S_sfx[i].name &&
			strlen(S_sfx[i].name + 2) == length &&
			!strnicmp(S_sfx[i].name + 2, key, length) &&
			!sfx_state[i]
		)
		{
			sfx_state[i] = true; // sfx has been edited
			return i;
		}

	return -1;
}

int dsda_GetOriginalSFXIndex(const char* key)
{
	int i;
	const char* c;

	for(i = 1; deh_soundnames[i]; ++i)
		if(!strncasecmp(deh_soundnames[i], key, 6))
			return i;

	// is it a number?
	for(c = key; *c; c++)
		if(!isdigit(*c))
			return -1;

	i = atoi(key);
	dsda_EnsureCapacity(i);

	return i;
}

static sfxinfo_t* dsda_SFXAtIndex(int index)
{
	dsda_EnsureCapacity(index);

	if(index > highest_index)
		highest_index = index;

	return &S_sfx[index];
}

sfxinfo_t* dsda_GetDehSFX(int index)
{
	return dsda_SFXAtIndex(index);
}

sfxinfo_t* dsda_NewSFX(int* index)
{
	*index = highest_index + 1;

	return dsda_SFXAtIndex(*index);
}

void dsda_InitializeSFX(sfxinfo_t* source, int count)
{
	int i;
	extern int raven;

	num_sfx = count;
	highest_index = count - 1;
	deh_soundnames_size = num_sfx + 1;

	S_sfx = source;

	if(raven) return;

	deh_soundnames = static_cast<char**>(malloc(deh_soundnames_size * sizeof(*deh_soundnames)));
	for(i = 1; i < num_sfx; i++)
		if(S_sfx[i].name != nullptr)
			deh_soundnames[i] = strdup(S_sfx[i].name + 2); // offset "ds" for dehacked names
		else
			deh_soundnames[i] = nullptr;
	deh_soundnames[0] = nullptr;
	deh_soundnames[num_sfx] = nullptr;

	sfx_state = static_cast<byte*>(calloc(num_sfx, sizeof(*sfx_state)));
}

void dsda_FreeDehSFX()
{
	int i;

	if(deh_soundnames)
		for(i = 0; i < deh_soundnames_size; i++)
			if(deh_soundnames[i])
				free(deh_soundnames[i]);

	free(deh_soundnames);
	free(sfx_state);

	deh_soundnames = nullptr;
	sfx_state = nullptr;
}

static int dsda_parallel_sfx_limit;
static int dsda_parallel_sfx_window;

extern "C" void dsda_InitParallelSFXFilter()
{
	dsda_parallel_sfx_limit = dsda_IntConfig(ConfigId::ParallelSfxLimit);
	dsda_parallel_sfx_window = dsda_IntConfig(ConfigId::ParallelSfxWindow);
}

dboolean dsda_BlockSFX(sfxinfo_t* sfx)
{
	extern int gametic;

	if(!dsda_parallel_sfx_limit) return false;

	if(gametic - sfx->parallel_tic >= dsda_parallel_sfx_window)
	{
		sfx->parallel_tic = gametic;
		sfx->parallel_count = 0;
	}

	++sfx->parallel_count;

	return sfx->parallel_count > dsda_parallel_sfx_limit;
}
