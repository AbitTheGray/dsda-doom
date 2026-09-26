// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *      System interface, sound.
 */

#pragma once

#include <utility>

#include "doomtype.hpp"
#include "doomdata.hpp"
#include "p_maputl.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct overrun_param_s
{
	int warn;
	int emulate;
	int footer;
	int footer_emulate;
	int promted;
	int happened;
} overrun_param_t;

enum struct OverrunList : int32_t
{
	Spechit,
	Reject,
	Intercept,
	Playeringame,
	Donut,
	Missedbackside,

	Max //last
};

extern int overflows_enabled;
extern overrun_param_t overflows[std::to_underlying(OverrunList::Max)];
extern const char* overflow_cfgname[std::to_underlying(OverrunList::Max)];

#define EMULATE(overflow) (overflows_enabled && (overflows[std::to_underlying(overflow)].footer ? overflows[std::to_underlying(overflow)].footer_emulate : overflows[std::to_underlying(overflow)].emulate))
#define PROCESS(overflow) (overflows_enabled && (overflows[std::to_underlying(overflow)].warn || EMULATE(overflow)))

// e6y
//
// intercepts overrun emulation
// See more information on:
// doomworld.com/vb/doom-speed-demos/35214-spechits-reject-and-intercepts-overflow-lists
//
// Thanks to Simon Howard (fraggle) for refactor the intercepts
// overrun code so that it should work properly on big endian machines
// as well as little endian machines.

#define MAXINTERCEPTS_ORIGINAL 128

typedef struct
{
	int len;
	void* addr;
	void* addr2;
} intercepts_overrun_t;

extern intercepts_overrun_t intercepts_overrun[];
void InterceptsOverrun(int num_intercepts, intercept_t* intercept);

//
// playeringame overrun emulation
//

int PlayeringameOverrun(const mapthing_t* mthing);

//
// spechit overrun emulation
//

// Spechit overrun magic value.
#define DEFAULT_SPECHIT_MAGIC 0x01C09C98

typedef struct spechit_overrun_param_s
{
	line_t* line;

	line_t*** spechit;
	int* numspechit;

	fixed_t* tmbbox;
	fixed_t* tmfloorz;
	fixed_t* tmceilingz;

	int* crushchange;
	dboolean* nofit;
} spechit_overrun_param_t;

extern unsigned int spechit_baseaddr;

void SpechitOverrun(spechit_overrun_param_t* params);

//
// reject overrun emulation
//

void RejectOverrun(unsigned int length, const byte** rejectmatrix, int totallines);

//
// donut overrun emulation (linedef action 9)
//

int DonutOverrun(fixed_t* pfloorheight, short* pfloorpic);

int MissedBackSideOverrun(line_t* line);
sector_t* GetSectorAtNullAddress();

#ifdef __cplusplus
}
#endif
