// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Utility String View

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"

// Non-allocated, read-only view into a string, like C++ string_view
typedef struct
{
	const char* string;
	size_t size;
} dsda_string_view_t;

void dsda_InitStringView(dsda_string_view_t* sv, const char* string, size_t size);

dboolean dsda_IsStringViewEmpty(const dsda_string_view_t* sv);

void dsda_StringViewAtOffset(const dsda_string_view_t* sv, size_t offset, dsda_string_view_t* ofs);

// Splits `sv` at the first instance of `c`, putting everything up to and including it in `before`
// and everything after in `after`.  `before` or `after` may safely alias `sv`.  If `c` does not
// occur in `sv`, `before` is set to `sv` and `after` is set to empty.  Returns `true` if
// an occurence was found.
dboolean dsda_SplitStringViewAfterChar(const dsda_string_view_t* sv, char c,
	dsda_string_view_t* before,
	dsda_string_view_t* after);

// Like the above, except `c` is included in `after` if found
dboolean dsda_SplitStringViewBeforeChar(const dsda_string_view_t* sv, char c,
	dsda_string_view_t* before,
	dsda_string_view_t* after);

// Sets `line` to the current line in `sv`, including any trailing '\r' or '\n'
// and advances `sv` to the next line.
// If the string does not end with a '\n', `line` is set to `sv` and `sv` is emptied.
// Returns `false` if `sv` is empty.
dboolean dsda_GetStringViewLine(dsda_string_view_t* sv, dsda_string_view_t* line);

dboolean dsda_StringViewStartsWith(const dsda_string_view_t* sv, const char* prefix);

void dsda_StringViewAfterChars(const dsda_string_view_t* sv, const char* chars,
	dsda_string_view_t* after);

#ifdef __cplusplus
}
#endif
