// SPDX-License-Identifier: GPL-2.0-or-later

// e6y
// All tabs are replaced with spaces.
// Fixed eol style of files.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _MEMFILE MEMFILE;

enum struct MemSeek : int32_t
{
	Set,
	Cur,
	End,
};

MEMFILE* mem_fopen_read(const void* buf, size_t buflen);
size_t mem_fread(void* buf, size_t size, size_t nmemb, MEMFILE* stream);
MEMFILE* mem_fopen_write();
size_t mem_fwrite(const void* ptr, size_t size, size_t nmemb, MEMFILE* stream);
void mem_get_buf(MEMFILE* stream, void** buf, size_t* buflen);
void mem_fclose(MEMFILE* stream);
long mem_ftell(MEMFILE* stream);
int mem_fseek(MEMFILE* stream, signed long offset, MemSeek whence);

#ifdef __cplusplus
}
#endif
