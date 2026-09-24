// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      System specific interface stuff.
 */

#pragma once

#include <SDL_opengl.h>
#include "doomtype.hpp"
#include "v_video.hpp"
#include "SDL.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern SDL_Window* sdl_window;
extern SDL_Renderer* sdl_renderer;

extern SDL_Rect renderer_rect;
extern SDL_Rect window_rect;
extern SDL_Rect viewport_rect;

extern const char* screen_resolutions_list[];

extern const char* sdl_video_window_pos;

void I_PreInitGraphics();      /* CPhipps - do stuff immediately on start */
void I_InitScreenResolution(); /* init resolution */
void I_SetWindowCaption();     /* Set the window caption */
void I_SetWindowIcon();        /* Set the application icon */
void I_InitGraphics();
void I_UpdateVideoMode();
void I_ShutdownGraphics();

SDL_Window* I_GetSDLWindow();
SDL_Renderer* I_GetSDLRenderer();
void dsda_Shutdown();

/* Takes full 8 bit values. */
void I_SetPalette(int pal); /* CPhipps - pass down palette number */

void I_QueueFrameCapture();
void I_QueueScreenshot();
void I_HandleCapture();

void I_FinishUpdate();

int I_ScreenShot(const char* fname);
// NSM expose lower level screen data grab for vidcap
unsigned char* I_GrabScreen();

/* I_StartTic
 * Called by D_DoomLoop,
 * called before processing each tic in a frame.
 * Quick syncronous operations are performed here.
 * Can call D_PostEvent.
 */
void I_StartTic();

/* I_StartFrame
 * Called by D_DoomLoop,
 * called before processing any tics in a frame
 * (just after displaying a frame).
 * Time consuming syncronous operations
 * are performed here (joystick reading).
 * Can call D_PostEvent.
 */

void I_StartFrame();

extern int desired_fullscreen; //e6y
extern int exclusive_fullscreen;

void I_UpdateRenderSize(); // Handle potential
extern int renderW;            // resolution scaling
extern int renderH;            // - DTIED

extern dboolean window_focused;
dboolean I_WindowFocused();
void UpdateGrab();

void I_SetWindowRect();
void I_SetViewportRect();

#ifdef __cplusplus
}
#endif
