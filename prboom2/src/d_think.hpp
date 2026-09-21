// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  MapObj data. Map Objects or mobjs are actors, entities,
 *  thinker, take-your-pick... anything that moves, acts, or
 *  suffers state changes of more or less violent nature.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Experimental stuff.
 * To compile this as "ANSI C with classes"
 *  we will need to handle the various
 *  action functions cleanly.
 */
// killough 11/98: convert back to C instead of C++
typedef void (*actionf_t)();

//e6y: for boom's friction code
typedef void (*actionf_v)();
typedef void (*actionf_p1)(void*);
typedef void (*actionf_p2)(void*, void*);

/* Note: In d_deh.c you will find references to these
 * wherever code pointers and function handlers exist
 */
/*
typedef union
{
  actionf_p1    acp1;
  actionf_v     acv;
  actionf_p2    acp2;

} actionf_t;
*/

/* Historically, "think_t" is yet another
 *  function pointer to a routine to handle
 *  an actor.
 */
typedef actionf_t think_t;


/* Doubly linked list of actors. */
typedef struct thinker_s
{
	struct thinker_s* prev;
	struct thinker_s* next;
	think_t function;

	/* killough 8/29/98: we maintain thinkers in several equivalence classes,
	* according to various criteria, so as to allow quicker searches.
	*/

	struct thinker_s *cnext, *cprev; /* Next, previous thinkers in same class */

	/* killough 11/98: count of how many other objects reference
	* this one using pointers. Used for garbage collection.
	*/
	unsigned references;
} thinker_t;

#ifdef __cplusplus
}
#endif
