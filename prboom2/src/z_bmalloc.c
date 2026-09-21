// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 * This is designed to be a fast allocator for small, regularly used block sizes
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "doomtype.h"
#include "z_zone.h"
#include "z_bmalloc.h"
#include "lprintf.h"

typedef struct bmalpool_s {
  struct bmalpool_s *nextpool;
  size_t             blocks;
  byte               used[0];
} bmalpool_t;

inline static void* getelem(bmalpool_t *p, size_t size, size_t n)
{
  return (((byte*)p) + sizeof(bmalpool_t) + sizeof(byte)*(p->blocks) + size*n);
}

inline static PUREFUNC int iselem(const bmalpool_t *pool, size_t size, const void* p)
{
  // CPhipps - need portable # of bytes between pointers
  int dif = (const char*)p - (const char*)pool;

  dif -= sizeof(bmalpool_t);
  dif -= pool->blocks;
  if (dif<0) return -1;
  dif /= size;
  return (((size_t)dif >= pool->blocks) ? -1 : dif);
}

enum { unused_block = 0, used_block = 1};

void* Z_BMalloc(struct block_memory_alloc_s *pzone)
{
  register bmalpool_t **pool = (bmalpool_t **)&(pzone->firstpool);
  while (*pool != NULL) {
    byte *p = memchr((*pool)->used, unused_block, (*pool)->blocks); // Scan for unused marker
    if (p) {
      int n = p - (*pool)->used;
#ifdef SIMPLECHECKS
      if ((n<0) || ((size_t)n>=(*pool)->blocks))
  I_Error("Z_BMalloc: memchr returned pointer outside of array");
#endif
      (*pool)->used[n] = used_block;
      return getelem(*pool, pzone->size, n);
    } else
      pool = &((*pool)->nextpool);
  }
  {
    // Nothing available, must allocate a new pool
    bmalpool_t *newpool;

    // CPhipps: Allocate new memory, initialised to 0

    *pool = newpool =
      Z_CallocLevel(sizeof(*newpool) + (sizeof(byte) + pzone->size)*(pzone->perpool), 1);
    newpool->nextpool = NULL; // NULL = (void*)0 so this is redundant

    // Return element 0 from this pool to satisfy the request
    newpool->used[0] = used_block;
    newpool->blocks = pzone->perpool;
    return getelem(newpool, pzone->size, 0);
  }
}

void Z_BFree(struct block_memory_alloc_s *pzone, void* p)
{
  register bmalpool_t **pool = (bmalpool_t**)&(pzone->firstpool);

  while (*pool != NULL) {
    int n = iselem(*pool, pzone->size, p);
    if (n >= 0) {
#ifdef SIMPLECHECKS
      if ((*pool)->used[n] == unused_block)
  I_Error("Z_BFree: Refree in zone %s", pzone->desc);
#endif
      (*pool)->used[n] = unused_block;
      if (memchr(((*pool)->used), used_block, (*pool)->blocks) == NULL) {
  // Block is all unused, can be freed
  bmalpool_t *oldpool = *pool;
  *pool = (*pool)->nextpool;
  Z_Free(oldpool);
      }
      return;
    } else pool = &((*pool)->nextpool);
  }
  I_Error("Z_BFree: Free not in zone %s", pzone->desc);
}
