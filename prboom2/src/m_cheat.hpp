// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Cheat code checking.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "d_event.hpp"

/* The cheat handlers do not share a parameter list, so the table entry casts
 * each one to the common signature, the same way the state tables do.
 */
#define CHEAT(cheat, deh_cheat, when, func, arg, repeatable) \
  { cheat, deh_cheat, static_cast<cheat_when_t>(when), reinterpret_cast<void (*)()>(func), \
    arg, repeatable, 0, 0, 0, 0, 0, "" }

#define CHEAT_ARGS_MAX 8  /* Maximum number of args at end of cheats */

/* killough 4/16/98: Cheat table structure */

typedef enum
{
	cht_always       = 0,
	not_demo         = 1,
	not_menu         = 2,
	not_classic_demo = 4, // allowed in dsda demo format
} cheat_when_t;

typedef struct cheatseq_s
{
	const char* cheat;
	const char* const deh_cheat;
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

void M_CheatGod();
void M_CheatNoClip();
void M_CheatIDDT();
dboolean M_CheatResponder(event_t* ev);
dboolean M_CheatEntered(const char* element, const char* value);

#ifdef __cplusplus
}
#endif
