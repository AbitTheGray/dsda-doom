// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Demo

#pragma once

#include "d_ticcmd.hpp"
#include "dsda/utility.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

const char* dsda_DemoNameBase();
void dsda_SetDemoBaseName(const char* name);
dboolean dsda_StartDemoSegment(const char* demo_name);
const byte* dsda_EvaluateDemoStartPoint(const byte* demo_p);
void dsda_ExportDemo(const char* name);
void dsda_MarkCompatibilityLevelUnspecified();
int dsda_BytesPerTic();
int dsda_DemoTic();
void dsda_EvaluateBytesPerTic();
void dsda_RestoreCommandHistory();
void dsda_InitDemoRecording();
void dsda_WriteToDemo(const void* buffer, size_t length);
void dsda_WriteQueueToDemo(const void* buffer, size_t length);
void dsda_WriteTicToDemo(const void* buffer, size_t length);
void dsda_WriteDemoToFile();
dboolean dsda_CopyPendingCmd(ticcmd_t* cmd, int delta);
void dsda_CopyPriorCmd(ticcmd_t* cmd, int delta);
void dsda_QueueJoin();
dboolean dsda_PendingJoin();
void dsda_JoinDemoCmd(ticcmd_t* cmd);
const byte* dsda_StripDemoVersion255(const byte* demo_p, const byte* header_p, size_t size);
void dsda_WriteDSDADemoHeader(byte** p);
void dsda_ApplyDSDADemoFormat(byte** demo_p);
void dsda_GetDemoCheckSum(dsda_cksum_t* cksum, byte* features, size_t feature_slots, byte* demo, size_t demo_size);
void dsda_GetDemoRecordingCheckSum(dsda_cksum_t* cksum);
void dsda_EndDemoRecording();
int dsda_DemoDataSize(byte complete);
void dsda_StoreDemoData(byte complete);
void dsda_RestoreDemoData(byte complete);
int dsda_DemoTicsCount(const byte* p, const byte* demobuffer, int demolength);
const byte* dsda_DemoMarkerPosition(byte* buffer, size_t file_size);

#ifdef __cplusplus
}
#endif
