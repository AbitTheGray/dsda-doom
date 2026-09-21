// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Dehacked Hash

#pragma once

#define DEH_INDEX_HASH_SIZE 128
#define DEH_INDEX_NOT_FOUND -1

typedef struct deh_index_entry_s {
  int index_in;
  int index_out;
  struct deh_index_entry_s* next;
} deh_index_entry_t;

typedef struct {
  deh_index_entry_t table[DEH_INDEX_HASH_SIZE];
  int start_index;
  int end_index;
} deh_index_hash_t;

int dsda_FindDehIndex(int index, deh_index_hash_t* hash);
int dsda_GetDehIndex(int index, deh_index_hash_t* hash);
