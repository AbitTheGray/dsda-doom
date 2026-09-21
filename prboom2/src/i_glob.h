// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	System specific file globbing interface.

#ifndef __I_GLOB__
#define __I_GLOB__

#define GLOB_FLAG_NOCASE  0x01
#define GLOB_FLAG_SORTED  0x02

typedef struct glob_s glob_t;

// Start reading a list of file paths from the given directory which match
// the given glob pattern. I_EndGlob() must be called on completion.
glob_t *I_StartGlob(const char *directory, const char *glob, int flags);

// Same as I_StartGlob but multiple glob patterns can be provided. The list
// of patterns must be terminated with NULL.
glob_t *I_StartMultiGlob(const char *directory, int flags,
                         const char *glob, ...);

// Finish reading file list.
void I_EndGlob(glob_t *glob);

// Read the name of the next globbed filename. NULL is returned if there
// are no more found.
const char *I_NextGlob(glob_t *glob);

#endif

