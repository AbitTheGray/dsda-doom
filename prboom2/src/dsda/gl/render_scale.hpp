// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	Data for rendering non-exclusive fullscreen in OpenGL
//  Original Author: elim

#pragma once

#include "SDL.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int gl_statusbar_height;
extern int gl_scene_width;
extern int gl_scene_height;
extern float gl_scale_x;
extern float gl_scale_y;
extern int gl_letterbox_clear_required;

void dsda_GLSetRenderViewportParams();
void dsda_GLSetRenderViewport();
void dsda_GLSetRenderViewportScissor();
void dsda_GLSetRenderSceneScissor();
void dsda_GLSetScreenSpaceScissor(int x, int y, int w, int h);
void dsda_GLUpdateStatusBarVisible();
void dsda_GLLetterboxClear();
void dsda_GLStartMeltRenderTexture();
void dsda_GLEndMeltRenderTexture();
void dsda_GLFullscreenOrtho2D();

#ifdef __cplusplus
}
#endif
