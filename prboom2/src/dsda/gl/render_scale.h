// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Data for rendering non-exclusive fullscreen in OpenGL
//  Original Author: elim

#pragma once

#include "SDL.h"

extern int gl_statusbar_height;
extern int gl_scene_width;
extern int gl_scene_height;
extern float gl_scale_x;
extern float gl_scale_y;
extern int gl_letterbox_clear_required;

void dsda_GLSetRenderViewportParams(void);
void dsda_GLSetRenderViewport(void);
void dsda_GLSetRenderViewportScissor(void);
void dsda_GLSetRenderSceneScissor(void);
void dsda_GLSetScreenSpaceScissor(int x, int y, int w, int h);
void dsda_GLUpdateStatusBarVisible(void);
void dsda_GLLetterboxClear(void);
void dsda_GLStartMeltRenderTexture(void);
void dsda_GLEndMeltRenderTexture(void);
void dsda_GLFullscreenOrtho2D(void);
