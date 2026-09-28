// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Zone Memory Allocation. Neat.
 *
 * Neat enough to be rewritten by Lee Killough...
 *
 * Must not have been real neat :)
 *
 * Made faster and more general, and added wrappers for all of Doom's
 * memory allocation functions, including malloc() and similar functions.
 * Added line and file numbers, in case of error. Added performance
 * statistics and tunables.
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "z_zone.hpp"
#include "doomstat.hpp"
#include "v_video.hpp"
#include "g_game.hpp"
#include "lprintf.hpp"

#ifdef DJGPP
#include <dpmi.h>
#endif

#define ZONE_SIGNATURE 0x931d4a11

enum struct ZoneTag : int32_t
{
	Static,
	Level,
	Max
};

typedef struct memblock
{
	unsigned signature;
	struct memblock *next, *prev;
	size_t size;
	ZoneTag tag;
} memblock_t;

static const size_t HEADER_SIZE = sizeof(memblock_t);

static memblock_t* blockbytag[std::to_underlying(ZoneTag::Max)];

/* Z_Malloc
 * cph - the algorithm here was a very simple first-fit round-robin
 *  one - just keep looping around, freeing everything we can until
 *  we get a large enough space
 *
 * This has been changed now; we still do the round-robin first-fit,
 * but we only free the blocks we actually end up using; we don't
 * free all the stuff we just pass on the way.
 */

static void* Z_MallocTag(size_t size, ZoneTag tag)
{
	memblock_t* block = nullptr;

	if(!size)
		return nullptr; // malloc(0) returns NULL

	if(!(block = static_cast<memblock_t*>(malloc(size + HEADER_SIZE))))
	{
		Log::Fatal("Z_Malloc: Failure trying to allocate {} bytes", size);
	}

	if(!blockbytag[std::to_underlying(tag)])
	{
		blockbytag[std::to_underlying(tag)] = block;
		block->next = block->prev = block;
	}
	else
	{
		blockbytag[std::to_underlying(tag)]->prev->next = block;
		block->prev = blockbytag[std::to_underlying(tag)]->prev;
		block->next = blockbytag[std::to_underlying(tag)];
		blockbytag[std::to_underlying(tag)]->prev = block;
	}

	block->size = size;
	block->signature = ZONE_SIGNATURE;
	block->tag = tag; // tag
	block = (memblock_t*)((char*)block + HEADER_SIZE);

	return block;
}

void Z_Free(void* p)
{
	memblock_t* block = (memblock_t*)((char*)p - HEADER_SIZE);

	if(!p)
		return;

	if(block->signature != ZONE_SIGNATURE)
		Log::Fatal("Z_Free: freed a non-zone pointer");
	block->signature = 0; // Nullify signature so another free fails

	if(block == block->next)
		blockbytag[std::to_underlying(block->tag)] = nullptr;
	else if(blockbytag[std::to_underlying(block->tag)] == block)
		blockbytag[std::to_underlying(block->tag)] = block->next;
	block->prev->next = block->next;
	block->next->prev = block->prev;

	free(block);
}

static void Z_FreeTag(ZoneTag tag)
{
	memblock_t *block, *end_block;

	if(tag < ZoneTag::Static || tag >= ZoneTag::Max)
		Log::Fatal("Z_FreeTag: Tag {} does not exist", std::to_underlying(tag));

	block = blockbytag[std::to_underlying(tag)];
	if(!block)
		return;
	end_block = block->prev;
	while(1)
	{
		memblock_t* next = block->next;
		Z_Free((char*)block + HEADER_SIZE);
		if(block == end_block)
			break;
		block = next; // Advance to next block
	}
}

static void* Z_ReallocTag(void* ptr, size_t n, ZoneTag tag)
{
	void* p = Z_MallocTag(n, tag);
	if(ptr)
	{
		memblock_t* block = (memblock_t*)((char*)ptr - HEADER_SIZE);
		memcpy(p, ptr, n <= block->size ? n : block->size);
		Z_Free(ptr);
	}
	return p;
}

static void* Z_CallocTag(size_t n1, size_t n2, ZoneTag tag)
{
	return
		(n1 *= n2) ? memset(Z_MallocTag(n1, tag), 0, n1) : nullptr;
}

static char* Z_StrdupTag(const char* s, ZoneTag tag)
{
	return strcpy(static_cast<char *>(Z_MallocTag(strlen(s) + 1, tag)), s);
}

void* Z_Malloc(size_t size)
{
	return Z_MallocTag(size, ZoneTag::Static);
}

void* Z_Calloc(size_t n, size_t n2)
{
	return Z_CallocTag(n, n2, ZoneTag::Static);
}

void* Z_Realloc(void* p, size_t n)
{
	return Z_ReallocTag(p, n, ZoneTag::Static);
}

char* Z_Strdup(const char* s)
{
	return Z_StrdupTag(s, ZoneTag::Static);
}

void Z_FreeLevel()
{
	return Z_FreeTag(ZoneTag::Level);
}

void* Z_MallocLevel(size_t size)
{
	return Z_MallocTag(size, ZoneTag::Level);
}

void* Z_CallocLevel(size_t n, size_t n2)
{
	return Z_CallocTag(n, n2, ZoneTag::Level);
}

void* Z_ReallocLevel(void* p, size_t n)
{
	return Z_ReallocTag(p, n, ZoneTag::Level);
}

char* Z_StrdupLevel(const char* s)
{
	return Z_StrdupTag(s, ZoneTag::Level);
}
