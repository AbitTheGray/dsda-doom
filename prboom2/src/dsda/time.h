// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Time

#pragma once

typedef enum {
  dsda_timer_realtime,
  dsda_timer_fps,
  dsda_timer_key_frame,
  dsda_timer_brute_force,
  dsda_timer_render_stats,
  dsda_timer_temp,
  DSDA_TIMER_COUNT
} dsda_timer_t;

extern int (*dsda_GetTick)(void);
extern unsigned long long (*dsda_TickElapsedTime)(void);

void dsda_StartTimer(int timer);
unsigned long long dsda_ElapsedTime(int timer);
unsigned long long dsda_ElapsedTimeMS(int timer);
void dsda_PrintElapsedTime(int timer, const char* message);
void dsda_LimitFPS(void);
int dsda_GetTickRealTime(void);
void dsda_ResetTimeFunctions(int fastdemo);
