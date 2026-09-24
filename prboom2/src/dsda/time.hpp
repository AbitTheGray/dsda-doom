// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Time

#pragma once

#include <stdint.h>
#include <utility>

#ifdef __cplusplus
extern "C"
{
#endif

enum struct DsdaTimer : int32_t
{
	Realtime,
	Fps,
	KeyFrame,
	BruteForce,
	RenderStats,
	Temp,
	Count
};

extern int (*dsda_GetTick)();
extern unsigned long long (*dsda_TickElapsedTime)();

void dsda_StartTimer(DsdaTimer timer);
unsigned long long dsda_ElapsedTime(DsdaTimer timer);
unsigned long long dsda_ElapsedTimeMS(DsdaTimer timer);
void dsda_PrintElapsedTime(DsdaTimer timer, const char* message);
void dsda_LimitFPS();
int dsda_GetTickRealTime();
void dsda_ResetTimeFunctions(int fastdemo);

#ifdef __cplusplus
}
#endif
