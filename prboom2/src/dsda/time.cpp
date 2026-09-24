// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Time

#include <utility>

#include <time.h>
#include <string.h>

#include "i_system.hpp"
#include "lprintf.hpp"

#include "dsda/args.hpp"
#include "dsda/configuration.hpp"

#include "time.hpp"
#include "doomdef.hpp"

// clock_gettime implementation for msvc
// NOTE: Only supports CLOCK_MONOTONIC
#ifdef _MSC_VER

#include <windows.h>

#define CLOCK_MONOTONIC -1

static int clock_gettime(int clockid, struct timespec* tp)
{
	static unsigned long long timer_frequency = 0;
	unsigned long long time;

	// Get number of timer counts per second
	if(!timer_frequency)
		QueryPerformanceFrequency((LARGE_INTEGER*)&timer_frequency);

	// Get timer counts
	QueryPerformanceCounter((LARGE_INTEGER*)&time);

	// Convert timer counts to timespec (that is, nanoseconds and seconds)
	tp->tv_nsec = time % timer_frequency * 1000000000 / timer_frequency;
	tp->tv_sec = time / timer_frequency;

	return 0;
}

#endif //_MSC_VER

static struct timespec dsda_time[std::to_underlying(DsdaTimer::Count)];

void dsda_StartTimer(DsdaTimer timer)
{
	clock_gettime(CLOCK_MONOTONIC, &dsda_time[std::to_underlying(timer)]);
}

unsigned long long dsda_ElapsedTime(DsdaTimer timer)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	return (unsigned long long)(
		(signed long long)(now.tv_nsec - dsda_time[std::to_underlying(timer)].tv_nsec) / 1000 +
		(signed long long)(now.tv_sec - dsda_time[std::to_underlying(timer)].tv_sec) * 1000000
	);
}

unsigned long long dsda_ElapsedTimeMS(DsdaTimer timer)
{
	return dsda_ElapsedTime(timer) / 1000;
}

void dsda_PrintElapsedTime(DsdaTimer timer, const char* message)
{
	unsigned long long result;

	result = dsda_ElapsedTime(timer);
	lprintf(OutputLevels::Info, "%s: %lf\n", message, (double)result / 1000);
}

static void dsda_Throttle(DsdaTimer timer, unsigned long long target_time)
{
	unsigned long long elapsed_time;
	unsigned long long remaining_time;

	while(1)
	{
		elapsed_time = dsda_ElapsedTime(timer);

		if(elapsed_time >= target_time)
		{
			dsda_StartTimer(timer);
			return;
		}

		// Sleeping doesn't have high accuracy
		remaining_time = target_time - elapsed_time;
		if(remaining_time > 1000)
			I_uSleep(remaining_time - 1000);
	}
}

void dsda_LimitFPS()
{
	extern int movement_smooth;
	extern int window_focused;

	int allow_limit;
	int fps_limit;

	allow_limit = (movement_smooth || !window_focused) && !dsda_Flag(ArgId::Timedemo) && !dsda_Flag(ArgId::Fastdemo);
	fps_limit = window_focused
		? dsda_IntConfig(ConfigId::FpsLimit)
		: dsda_IntConfig(ConfigId::BackgroundFpsLimit);

	if(allow_limit && fps_limit)
	{
		unsigned long long target_time;

		target_time = 1000000 / fps_limit;

		dsda_Throttle(DsdaTimer::Fps, target_time);
	}
}

extern "C" int dsda_GameSpeed();

static unsigned long long dsda_RealTime()
{
	static dboolean started = false;

	if(!started)
	{
		started = true;
		dsda_StartTimer(DsdaTimer::Realtime);
	}

	return dsda_ElapsedTime(DsdaTimer::Realtime);
}

static unsigned long long dsda_ScaledTime()
{
	return dsda_RealTime() * dsda_GameSpeed() / 100;
}

extern int ms_to_next_tick;

// During a fast demo, each call yields a new tick
static int dsda_GetTickFastDemo()
{
	static int tick;
	return tick++;
}

int dsda_GetTickRealTime()
{
	int i;
	unsigned long long t;

	t = dsda_RealTime();

	i = t * TICRATE / 1000000;
	ms_to_next_tick = (i + 1) * 1000 / TICRATE - t / 1000;
	if(ms_to_next_tick > 1000 / TICRATE) ms_to_next_tick = 1;
	if(ms_to_next_tick < 1) ms_to_next_tick = 0;
	return i;
}

static int dsda_TickMS(int n)
{
	return n * 1000 * 100 / dsda_GameSpeed() / TICRATE;
}

static int dsda_GetTickScaledTime()
{
	int i;
	unsigned long long t;

	t = dsda_RealTime();

	i = t * TICRATE * dsda_GameSpeed() / 100 / 1000000;
	ms_to_next_tick = dsda_TickMS(i + 1) - t / 1000;
	if(ms_to_next_tick > dsda_TickMS(1)) ms_to_next_tick = 1;
	if(ms_to_next_tick < 1) ms_to_next_tick = 0;
	return i;
}

// During a fast demo, no time elapses in between ticks
static unsigned long long dsda_TickElapsedTimeFastDemo()
{
	return 0;
}

static unsigned long long dsda_TickElapsedRealTime()
{
	int tick = dsda_GetTick();

	return dsda_RealTime() - (unsigned long long)tick * 1000000 / TICRATE;
}

static unsigned long long dsda_TickElapsedScaledTime()
{
	int tick = dsda_GetTick();

	return dsda_ScaledTime() - (unsigned long long)tick * 1000000 / TICRATE;
}

int (*dsda_GetTick)() = dsda_GetTickRealTime;
unsigned long long (*dsda_TickElapsedTime)() = dsda_TickElapsedRealTime;

void dsda_ResetTimeFunctions(int fastdemo)
{
	if(fastdemo)
	{
		dsda_GetTick = dsda_GetTickFastDemo;
		dsda_TickElapsedTime = dsda_TickElapsedTimeFastDemo;
	}
	else if(dsda_GameSpeed() != 100)
	{
		dsda_GetTick = dsda_GetTickScaledTime;
		dsda_TickElapsedTime = dsda_TickElapsedScaledTime;
	}
	else
	{
		dsda_GetTick = dsda_GetTickRealTime;
		dsda_TickElapsedTime = dsda_TickElapsedRealTime;
	}
}
