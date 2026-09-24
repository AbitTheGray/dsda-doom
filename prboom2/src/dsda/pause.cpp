// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Pause Mode

#include "doomstat.hpp"
#include "e6y.hpp"

#include "pause.hpp"

static dboolean paused;

dboolean dsda_Paused()
{
	return paused != 0;
}

dboolean dsda_PausedViaMenu()
{
	return menuactive != MenuActive::Inactive && !netgame;
}

dboolean dsda_PausedOutsideDemo()
{
	return dsda_PauseMode(PAUSE_PLAYBACK | PAUSE_BUILDMODE) || dsda_PausedViaMenu();
}

dboolean dsda_CameraPaused()
{
	return paused && !walkcamera.type;
}

dboolean dsda_PauseMode(int mode)
{
	return (paused & mode) != 0;
}

void dsda_RemovePauseMode(int mode)
{
	paused &= ~mode;
}

void dsda_ApplyPauseMode(int mode)
{
	paused |= mode;
}

void dsda_TogglePauseMode(int mode)
{
	paused ^= mode;
}

void dsda_ResetPauseMode()
{
	paused = 0;
}

int dsda_MaskPause()
{
	int mask;

	mask = paused & ~PAUSE_COMMAND;
	paused &= PAUSE_COMMAND;

	return mask;
}

void dsda_UnmaskPause(int mask)
{
	paused |= mask;
}
