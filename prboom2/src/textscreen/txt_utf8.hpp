// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>

unsigned int TXT_DecodeUTF8(const char** ptr);

#ifdef __cplusplus
}
#endif
