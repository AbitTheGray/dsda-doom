// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 * This is designed to be a fast allocator for small, regularly used block sizes
 */

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include <string.h>

#include "doomtype.hpp"
#include "z_zone.hpp"
#include "z_bmalloc.hpp"
#include "lprintf.hpp"

typedef struct bmalpool_s
{
	struct bmalpool_s* nextpool;
	size_t blocks;
	byte used[0];
} bmalpool_t;

inline static void* getelem(bmalpool_t* p, size_t size, size_t n)
{
	return (((byte*)p) + sizeof(bmalpool_t) + sizeof(byte) * (p->blocks) + size * n);
}

inline static PUREFUNC int iselem(const bmalpool_t* pool, size_t size, const void* p)
{
	// CPhipps - need portable # of bytes between pointers
	int dif = (const char*)p - (const char*)pool;

	dif -= sizeof(bmalpool_t);
	dif -= pool->blocks;
	if(dif < 0) return -1;
	dif /= size;
	return (((size_t)dif >= pool->blocks) ? -1 : dif);
}

// the pool marks each block with one of these bytes
enum struct BlockState : uint8_t
{
	Unused = 0,
	Used   = 1
};

void* Z_BMalloc(struct block_memory_alloc_s* pzone)
{
	bmalpool_t** pool = (bmalpool_t**)&(pzone->firstpool);
	while(*pool != nullptr)
	{
		byte* p = static_cast<byte *>(memchr((*pool)->used, std::to_underlying(BlockState::Unused), (*pool)->blocks)); // Scan for unused marker
		if(p)
		{
			int n = p - (*pool)->used;
#ifdef SIMPLECHECKS
			if((n < 0) || ((size_t)n >= (*pool)->blocks))
				I_Error("Z_BMalloc: memchr returned pointer outside of array");
#endif
			(*pool)->used[n] = std::to_underlying(BlockState::Used);
			return getelem(*pool, pzone->size, n);
		}
		else
			pool = &((*pool)->nextpool);
	}
	{
		// Nothing available, must allocate a new pool
		bmalpool_t* newpool;

		// CPhipps: Allocate new memory, initialised to 0

		*pool = newpool =
			static_cast<bmalpool_t *>(Z_CallocLevel(sizeof(*newpool) + (sizeof(byte) + pzone->size) * (pzone->perpool), 1));
		newpool->nextpool = nullptr; // NULL = (void*)0 so this is redundant

		// Return element 0 from this pool to satisfy the request
		newpool->used[0] = std::to_underlying(BlockState::Used);
		newpool->blocks = pzone->perpool;
		return getelem(newpool, pzone->size, 0);
	}
}

void Z_BFree(struct block_memory_alloc_s* pzone, void* p)
{
	bmalpool_t** pool = (bmalpool_t**)&(pzone->firstpool);

	while(*pool != nullptr)
	{
		int n = iselem(*pool, pzone->size, p);
		if(n >= 0)
		{
#ifdef SIMPLECHECKS
			if((*pool)->used[n] == std::to_underlying(BlockState::Unused))
				I_Error("Z_BFree: Refree in zone %s", pzone->desc);
#endif
			(*pool)->used[n] = std::to_underlying(BlockState::Unused);
			if(memchr(((*pool)->used), std::to_underlying(BlockState::Used), (*pool)->blocks) == nullptr)
			{
				// Block is all unused, can be freed
				bmalpool_t* oldpool = *pool;
				*pool = (*pool)->nextpool;
				Z_Free(oldpool);
			}
			return;
		}
		else pool = &((*pool)->nextpool);
	}
	I_Error("Z_BFree: Free not in zone %s", pzone->desc);
}
