// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

extern int cap_fps;
extern int cap_frac;
extern int cap_wipescreen;

// true if we're capturing video
extern int capturing_video;

// init and open sound, video pipes
// fn is filename passed from command line, typically final output file
void I_CapturePrep(const char* fn);

// capture a single frame of video (and corresponding audio length)
// and send it to pipes
void I_CaptureFrame();

// close pipes, call muxcommand, finalize
void I_CaptureFinish();

#ifdef __cplusplus
}
#endif
