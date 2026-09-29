// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	System specific file globbing interface.

#pragma once

#include <cstdint>

#include "cpp/Util.hpp"

enum struct GlobFlag : uint8_t
{
	NoCase = Bit<uint8_t>(0u),
	Sorted = Bit<uint8_t>(1u),
};
ENUM_FLAGS_FUNC(GlobFlag)

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct glob_s glob_t;

// Start reading a list of file paths from the given directory which match
// the given glob pattern. I_EndGlob() must be called on completion.
glob_t* I_StartGlob(const char* directory, const char* glob, GlobFlag flags);

// Same as I_StartGlob but multiple glob patterns can be provided. The list
// of patterns must be terminated with NULL.
glob_t* I_StartMultiGlob(const char* directory, GlobFlag flags,
	const char* glob, ...);

// Finish reading file list.
void I_EndGlob(glob_t* glob);

// Read the name of the next globbed filename. NULL is returned if there
// are no more found.
const char* I_NextGlob(glob_t* glob);

#ifdef __cplusplus
}
#endif
