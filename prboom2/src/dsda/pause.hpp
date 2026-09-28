// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Pause Mode

#pragma once

#include "doomtype.hpp"

#include "cpp/Util.hpp"

// Why the game is paused; several reasons can apply at once.
enum struct PauseMode : uint8_t
{
	Command = Bit<uint8_t>(0u),
	Playback = Bit<uint8_t>(1u),
	BuildMode = Bit<uint8_t>(2u),
};
ENUM_FLAGS_FUNC(PauseMode)

#ifdef __cplusplus
extern "C"
{
#endif

dboolean dsda_Paused();
dboolean dsda_PausedViaMenu();
dboolean dsda_PausedOutsideDemo();
dboolean dsda_CameraPaused();
dboolean dsda_PauseMode(PauseMode mode);
void dsda_RemovePauseMode(PauseMode mode);
void dsda_ApplyPauseMode(PauseMode mode);
void dsda_TogglePauseMode(PauseMode mode);
void dsda_ResetPauseMode();
PauseMode dsda_MaskPause();
void dsda_UnmaskPause(PauseMode mask);

#ifdef __cplusplus
}
#endif
