// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      Main startup and splash screenstuff.
 */

#pragma once

#include "m_fixed.h"
#include "d_event.h"
#include "w_wad.h"

/* CPhipps - removed wadfiles[] stuff to w_wad.h */

//jff 1/24/98 make command line copies of play modes available
extern dboolean clnomonsters;  // checkparm of -nomonsters
extern dboolean clrespawnparm; // checkparm of -respawn
extern dboolean clfastparm;    // checkparm of -fast
//jff end of external declaration of command line playmode

extern dboolean nosfxparm;
extern dboolean nomusicparm;

// Called by IO functions when input is detected.
void D_PostEvent(event_t* ev);

// Demo stuff
extern dboolean advancedemo;
void D_AdvanceDemo(void);
void D_DoAdvanceDemo(void);

//
// BASE LEVEL
//

void D_Display(fixed_t frac);
void D_PageTicker(void);
void D_StartTitle(void);
void D_DoomMain(void);
void D_AddFile(const char* file, wad_source_t source);

extern char* iwadlump;

void AddIWAD(const char* iwad);

extern const char* port_wad_file;

typedef struct
{
	void (*func)(const char*);
	const char* name;
} demostate_t;

void D_SetPage(const char* name, int tics, int music);
