// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Console

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "doomtype.hpp"
#include "m_menu.hpp"

#define CONSOLE_SCRIPT_COUNT 10

extern menu_t dsda_ConsoleDef;

int dsda_ConsoleHeight();
dboolean dsda_OpenConsole();
void dsda_UpdateConsoleText(char* text);
void dsda_UpdateConsole(int action);
void dsda_ExecuteConsoleScript(int i);
void dsda_InterpretConsoleCommands(const char* str, dboolean noise, dboolean raise_errors);

#ifdef __cplusplus
}
#endif
