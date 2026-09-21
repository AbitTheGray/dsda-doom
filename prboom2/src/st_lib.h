// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  The status bar widget definitions and prototypes
 */

#ifndef __STLIB__
#define __STLIB__

// We are referring to patches.
#include "r_defs.h"
#include "v_video.h"  // color ranges

//
// Background and foreground screen numbers
//
#define BG 4
#define FG 0

//
// Typedefs of widgets
//

// Number widget

typedef struct
{
  // upper right-hand corner
  //  of the number (right-justified)
  int   x;
  int   y;

  // max # of digits in number
  int width;

  // last number value
  int   oldnum;

  // pointer to current value
  int*  num;

  // pointer to dboolean stating
  //  whether to update number
  dboolean*  on;

  // list of patches for 0-9
  const patchnum_t* p;

  // user data
  int data;
} st_number_t;

// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
typedef struct
{
  // number information
  st_number_t   n;

  // percent sign graphic
  const patchnum_t*    p;
} st_percent_t;

// Multiple Icon widget
typedef struct
{
  // center-justified location of icons
  int     x;
  int     y;

  // last icon number
  int     oldinum;

  // pointer to current icon
  int*    inum;

  // pointer to dboolean stating
  //  whether to update icon
  dboolean*    on;

  // list of icons
  const patchnum_t*   p;

  // user data
  int     data;

} st_multicon_t;

//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
void STlib_init(void);

// Number widget routines
void STlib_initNum
( st_number_t* n,
  int x,
  int y,
  const patchnum_t* pl,
  int* num,
  dboolean* on,
  int width );

void STlib_updateNum
( st_number_t* n,
  int cm,
  dboolean refresh );


// Percent widget routines
void STlib_initPercent
( st_percent_t* p,
  int x,
  int y,
  const patchnum_t* pl,
  int* num,
  dboolean* on,
  const patchnum_t* percent );


void STlib_updatePercent
( st_percent_t* per,
  int cm,
  int refresh );


// Multiple Icon widget routines
void STlib_initMultIcon
( st_multicon_t* mi,
  int x,
  int y,
  const patchnum_t*   il,
  int* inum,
  dboolean* on );


void STlib_updateMultIcon
( st_multicon_t* mi,
  dboolean refresh );

#endif
