// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

unsigned int TXT_DecodeUTF8(const char** ptr);

#ifdef __cplusplus
}
#endif
