// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Whether an extended demo carries a valid signature.
//	The values are what `analysis.txt` writes for the `signature` key.

#pragma once

#include <cstdint>

enum struct Signature : int8_t
{
	// The `FEATURES` lump is malformed, or its signature does not match.
	Invalid = -1,
	// There is no `FEATURES` lump.
	Unsigned = 0,
	Signed = 1,
};
