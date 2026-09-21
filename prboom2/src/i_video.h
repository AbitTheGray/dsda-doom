// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      System specific interface stuff.
 */

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL_opengl.h>

#include "doomtype.h"
#include "v_video.h"
#include "SDL.h"

extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;

extern SDL_Rect renderer_rect;
extern SDL_Rect window_rect;
extern SDL_Rect viewport_rect;

extern const char *screen_resolutions_list[];

extern const char *sdl_video_window_pos;

void I_PreInitGraphics(void); /* CPhipps - do stuff immediately on start */
void I_InitScreenResolution(void); /* init resolution */
void I_SetWindowCaption(void); /* Set the window caption */
void I_SetWindowIcon(void); /* Set the application icon */
void I_InitGraphics (void);
void I_UpdateVideoMode(void);
void I_ShutdownGraphics(void);

void *I_GetSDLWindow(void);
void *I_GetSDLRenderer(void);
void dsda_Shutdown(void);

/* Takes full 8 bit values. */
void I_SetPalette(int pal); /* CPhipps - pass down palette number */

void I_QueueFrameCapture(void);
void I_QueueScreenshot(void);
void I_HandleCapture(void);

void I_FinishUpdate (void);

int I_ScreenShot (const char *fname);
// NSM expose lower level screen data grab for vidcap
unsigned char *I_GrabScreen (void);

/* I_StartTic
 * Called by D_DoomLoop,
 * called before processing each tic in a frame.
 * Quick syncronous operations are performed here.
 * Can call D_PostEvent.
 */
void I_StartTic (void);

/* I_StartFrame
 * Called by D_DoomLoop,
 * called before processing any tics in a frame
 * (just after displaying a frame).
 * Time consuming syncronous operations
 * are performed here (joystick reading).
 * Can call D_PostEvent.
 */

void I_StartFrame (void);

extern int desired_fullscreen; //e6y
extern int exclusive_fullscreen;

void I_UpdateRenderSize(void);	// Handle potential
extern int renderW;		// resolution scaling
extern int renderH;		// - DTIED

extern dboolean window_focused;
dboolean I_WindowFocused(void);
void UpdateGrab(void);

void I_SetWindowRect(void);
void I_SetViewportRect(void);
