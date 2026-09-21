// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Cheat code checking.
 */

#pragma once

#include "d_event.h"

#define CHEAT(cheat, deh_cheat, when, func, arg, repeatable) \
  { cheat, deh_cheat, when, func, arg, repeatable, 0, 0, 0, 0, 0, "" }

#define CHEAT_ARGS_MAX 8  /* Maximum number of args at end of cheats */

/* killough 4/16/98: Cheat table structure */

typedef enum {
  cht_always = 0,
  not_demo = 1,
  not_menu = 2,
  not_classic_demo = 4, // allowed in dsda demo format
} cheat_when_t;

typedef struct cheatseq_s {
  const char *	cheat;
  const char *const deh_cheat;
  const cheat_when_t when;
  void (*const func)();
  const int arg;
  const int repeatable;
  uint64_t code, mask;
  size_t sequence_len;

  // state used during the game
  size_t chars_read;
  int param_chars_read;
  char parameter_buf[CHEAT_ARGS_MAX];
} cheatseq_t;

extern cheatseq_t cheat[];

void M_CheatGod(void);
void M_CheatNoClip(void);
void M_CheatIDDT(void);
dboolean M_CheatResponder(event_t *ev);
dboolean M_CheatEntered(const char* element, const char* value);
