// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Block memory allocator
 *  This is designed to be a fast allocator for small, regularly used block sizes
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

struct block_memory_alloc_s
{
	void* firstpool;
	size_t size;
	size_t perpool;
	const char* desc;
};

#define DECLARE_BLOCK_MEMORY_ALLOC_ZONE(name) extern struct block_memory_alloc_s name
#define IMPLEMENT_BLOCK_MEMORY_ALLOC_ZONE(name, size, num, desc) \
struct block_memory_alloc_s name = { nullptr, size, num, desc}
#define NULL_BLOCK_MEMORY_ALLOC_ZONE(name) name.firstpool = NULL

void* Z_BMalloc(struct block_memory_alloc_s* pzone);

inline static void* Z_BCalloc(struct block_memory_alloc_s* pzone)
{
	void* p = Z_BMalloc(pzone);
	memset(p, 0, pzone->size);
	return p;
}

void Z_BFree(struct block_memory_alloc_s* pzone, void* p);

#ifdef __cplusplus
}
#endif
