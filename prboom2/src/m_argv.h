// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  Argument handling.
 */

#pragma once

/* Returns the position of the given parameter in the params list (-1 if not found). */
int M_CheckParmEx(const char* check, char** params, int paramscount);

/* Parses the command line and sets up the argv[] array */
void M_ParseCmdLine(char* cmdstart, char** argv, char* args, int* numargs, int* numchars);
