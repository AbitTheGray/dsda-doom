// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void SC_OpenLump(const char* name);
void SC_OpenLumpByNum(int lump);
void SC_Close();
dboolean SC_GetString();
void SC_MustGetString();
void SC_MustGetStringName(const char* name);
dboolean SC_GetNumber();
void SC_MustGetNumber();
void SC_UnGet();
dboolean SC_Check();
dboolean SC_Compare(const char* text);
int SC_MatchString(const char* const* strings);
int SC_MustMatchString(const char* const* strings);
void SC_ScriptError(const char* message);

extern char* sc_String;
extern int sc_Number;
extern int sc_Line;
extern dboolean sc_End;
extern dboolean sc_Crossed;

#ifdef __cplusplus
}
#endif
