// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	How a single demo is handed to the game.

#pragma once

#include <string_view>

struct DemoOptions
{
	// File name inside `spec/support/lmps`, e.g. "30uv1755.lmp", or an absolute path.
	std::string_view lmp;
	// File name inside `spec/support/wads`.
	std::string_view iwad = "DOOM2.WAD";
	// Optional file name inside `spec/support/wads`.
	std::string_view pwad;
	// Extra arguments appended verbatim, e.g. "-heretic".
	std::string_view extra;
	// Also write the text file (`-export_text_file`) besides `levelstat.txt` and `analysis.txt`.
	bool textFile = true;
	// Print the reports the game wrote into the test output.
	bool printReports = true;
};
