// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Pause Mode

#include "doomstat.hpp"
#include "e6y.hpp"

#include "pause.hpp"

static PauseMode paused;

dboolean dsda_Paused()
{
	return paused != PauseMode{};
}

dboolean dsda_PausedViaMenu()
{
	return menuactive != MenuActive::Inactive && !netgame;
}

dboolean dsda_PausedOutsideDemo()
{
	return dsda_PauseMode(PauseMode::Playback | PauseMode::BuildMode) || dsda_PausedViaMenu();
}

dboolean dsda_CameraPaused()
{
	return paused != PauseMode{} && !walkcamera.type;
}

dboolean dsda_PauseMode(const PauseMode mode)
{
	return (paused & mode) != PauseMode{};
}

void dsda_RemovePauseMode(const PauseMode mode)
{
	paused -= mode;
}

void dsda_ApplyPauseMode(const PauseMode mode)
{
	paused |= mode;
}

void dsda_TogglePauseMode(const PauseMode mode)
{
	paused = paused ^ mode;
}

void dsda_ResetPauseMode()
{
	paused = {};
}

PauseMode dsda_MaskPause()
{
	const PauseMode mask = paused - PauseMode::Command;
	paused = paused & PauseMode::Command;

	return mask;
}

void dsda_UnmaskPause(const PauseMode mask)
{
	paused |= mask;
}
