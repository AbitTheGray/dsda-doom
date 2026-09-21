// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Globally defined strings.
 */

#include "dstrings.h"
#include "d_deh.h"

static const char* debug_quit_msg = "THIS IS NO MESSAGE!\nPage intentionally left blank.";

// killough 1/18/98: remove hardcoded limit, add const:
const char** endmsg[] =
{
	// DOOM1
	&s_QUITMSG,
	&s_QUITMSG1, // "please don't leave, there's more\ndemons to toast!",
	&s_QUITMSG2, // "let's beat it -- this is turning\ninto a bloodbath!",
	&s_QUITMSG3, // "i wouldn't leave if i were you.\ndos is much worse.",
	&s_QUITMSG4, // "you're trying to say you like dos\nbetter than me, right?",
	&s_QUITMSG5, // "don't leave yet -- there's a\ndemon around that corner!",
	&s_QUITMSG6, // "ya know, next time you come in here\ni'm gonna toast ya.",
	&s_QUITMSG7, // "go ahead and leave. see if i care.",  // 1/15/98 killough

	// QuitDOOM II messages
	&s_QUITMSG8,  // "you want to quit?\nthen, thou hast lost an eighth!",
	&s_QUITMSG9,  // "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
	&s_QUITMSG10, // "get outta here and go back\nto your boring programs.",
	&s_QUITMSG11, // "if i were your boss, i'd \n deathmatch ya in a minute!",
	&s_QUITMSG12, // "look, bud. you leave now\nand you forfeit your body count!",
	&s_QUITMSG13, // "just leave. when you come\nback, i'll be waiting with a bat.",
	&s_QUITMSG14, // "you're lucky i don't smack\nyou for thinking about leaving.",  // 1/15/98 killough

	// FinalDOOM?

	// Internal debug. Different style, too.
	&debug_quit_msg, // 1/15/98 killough
};

// killough 1/18/98: remove hardcoded limit and replace with var (silly hack):
const size_t NUM_QUITMESSAGES = sizeof(endmsg) / sizeof(*endmsg) - 1;
