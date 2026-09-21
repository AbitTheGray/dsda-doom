// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:  none
 */

#ifndef __HULIB__
#define __HULIB__

#include "v_video.h"  //jff 2/16/52 include color range defs

#include "dsda/font.h"

/* background and foreground screen numbers
 * different from other modules. */
//e6y #define BG      1
#define FG      0

#define HU_MAXLINELENGTH  80

// Text Line widget
typedef struct
{
  // left-justified position of scrolling text window
  int   x;
  int   y;

  const patchnum_t* f;                    // font
  int   sc;                             // start character
  //const char *cr;                       //jff 2/16/52 output color range
  // Proff - Made this an int again. Needed for OpenGL
  int   cm;                         //jff 2/16/52 output color range

  // killough 1/23/98: Support multiple lines:
  #define MAXLINES 25

  int   linelen;
  char  l[HU_MAXLINELENGTH*MAXLINES+1]; // line of text
  int   len;                            // current line length

  // e6y: wide-res
  enum patch_translation_e flags;

  int line_height;
  int kerning; // Heretic/Hexen -1 kerning
  int space_width;
} hu_textline_t;

//
// textline code
//

// clear a line of text
void HUlib_clearTextLine(hu_textline_t *t);

void HUlib_initTextLine
(
  hu_textline_t *t,
  int x,
  int y,
  const dsda_font_t *f,
  int cm,    //jff 2/16/98 add color range parameter
  enum patch_translation_e flags
);

// returns success
dboolean HUlib_addCharToTextLine(hu_textline_t *t, char ch);

// draws tline
void HUlib_drawTextLine(hu_textline_t *l, dboolean drawcursor);
void HUlib_drawOffsetTextLine(hu_textline_t* l, int offset);

//e6y
void HUlib_setTextXCenter(hu_textline_t* t);

char HUlib_Color(int cm);

#endif
