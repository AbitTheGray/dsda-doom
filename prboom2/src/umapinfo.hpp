// SPDX-License-Identifier: LGPL-2.0-or-later

//-----------------------------------------------------------------------------
//
// Copyright 2017 Christoph Oelckers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//-----------------------------------------------------------------------------

#pragma once

#include <stdint.h>

#include "cpp/Util.hpp"

enum struct MobjType : int32_t;

enum struct UMapinfoFlags : uint32_t
{
	LabelClear = (1u << 0),

	EndGameClear    = (1u << 1),
	EndGameArt      = (1u << 2),
	EndGameStandard = (1u << 3),
	EndGameCast     = (1u << 4),
	EndGameScroll   = (1u << 5),

	NoIntermission       = (1u << 6),
	InterTextClear       = (1u << 7),
	InterTextSecretClear = (1u << 8),

	BossActionClear = (1u << 9),

	EndGameAny = (EndGameArt | EndGameStandard |
		EndGameCast | EndGameScroll),
};
ENUM_FLAGS_FUNC(UMapinfoFlags)

#ifdef __cplusplus
extern "C"
{
#endif

	struct BossAction
	{
		MobjType type;
		int special;
		int tag;
	};

	struct MapEntry
	{
		char* lumpname;
		char* levelname;
		char* label;
		char* author;
		char* intertext;
		char* intertextsecret;
		char levelpic[9];
		char nextmap[9];
		char nextsecret[9];
		char music[9];
		char skytexture[9];
		char endpic[9];
		char endpalette[9];
		char exitpic[9];
		char enterpic[9];
		char interbackdrop[9];
		char intermusic[9];
		int partime;
		UMapinfoFlags flags;

		int numbossactions;
		struct BossAction* bossactions;
	};

	struct MapList
	{
		unsigned int mapcount;
		struct MapEntry* maps;
	};

	typedef void (*umapinfo_errorfunc)(const char* fmt, ...); // this must not return!

	extern struct MapList Maps;

	int ParseUMapInfo(const unsigned char* buffer, size_t length, umapinfo_errorfunc err);
	void FreeMapList();
	struct MapProperty* FindProperty(struct MapEntry* map, const char* name);

#ifdef __cplusplus
}
#endif
