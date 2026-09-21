// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	How a single demo is handed to the game.

#pragma once

#include <string_view>

struct DemoOptions
{
	// File name inside `spec/support/lmps`, e.g. "30uv1755.lmp".
	std::string_view lmp;
	// File name inside `spec/support/wads`.
	std::string_view iwad = "DOOM2.WAD";
	// Optional file name inside `spec/support/wads`.
	std::string_view pwad;
	// Extra arguments appended verbatim, e.g. "-heretic".
	std::string_view extra;
};
