// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Console

#ifndef __DSDA_CONSOLE__
#define __DSDA_CONSOLE__

#include "doomtype.h"
#include "m_menu.h"

#define CONSOLE_SCRIPT_COUNT 10

extern menu_t dsda_ConsoleDef;

int dsda_ConsoleHeight(void);
dboolean dsda_OpenConsole(void);
void dsda_UpdateConsoleText(char* text);
void dsda_UpdateConsole(int action);
void dsda_ExecuteConsoleScript(int i);
void dsda_InterpretConsoleCommands(const char* str, dboolean noise, dboolean raise_errors);

#endif
