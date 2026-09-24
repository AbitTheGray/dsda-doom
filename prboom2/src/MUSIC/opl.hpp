// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//     OPL interface.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*opl_callback_t)(void* data);

enum struct OplPort : int32_t
{
	RegisterPort      = 0,
	DataPort          = 1,
	RegisterPortOpl3 = 2
};

#define OPL_NUM_OPERATORS   21
#define OPL_NUM_VOICES      9

#define OPL_REG_WAVEFORM_ENABLE   0x01
#define OPL_REG_TIMER1            0x02
#define OPL_REG_TIMER2            0x03
#define OPL_REG_TIMER_CTRL        0x04
#define OPL_REG_FM_MODE           0x08
#define OPL_REG_NEW               0x105

// Operator registers (21 of each):

#define OPL_REGS_TREMOLO          0x20
#define OPL_REGS_LEVEL            0x40
#define OPL_REGS_ATTACK           0x60
#define OPL_REGS_SUSTAIN          0x80
#define OPL_REGS_WAVEFORM         0xE0

// Voice registers (9 of each):

#define OPL_REGS_FREQ_1           0xA0
#define OPL_REGS_FREQ_2           0xB0
#define OPL_REGS_FEEDBACK         0xC0

// Times

#define OPL_SECOND ((uint64_t) 1000 * 1000)

//
// Low-level functions.
//

// Initialize the OPL subsystem.

int OPL_Init(unsigned int port_base);

// Shut down the OPL subsystem.

void OPL_Shutdown();


// Write to one of the OPL I/O ports:

void OPL_WritePort(OplPort port, unsigned int value);

// Read from one of the OPL I/O ports:

unsigned int OPL_ReadPort(OplPort port);

//
// Higher-level functions.
//

// Read the cuurrent status byte of the OPL chip.

unsigned int OPL_ReadStatus();

// Write to an OPL register.

void OPL_WriteRegister(int reg, int value);

// Perform a detection sequence to determine that an
// OPL chip is present.

int OPL_Detect();

// Initialize all registers, performed on startup.

void OPL_InitRegisters(int opl3);


// Block until the specified number of milliseconds have elapsed.

void OPL_Delay(unsigned int ms);

// Pause the OPL callbacks.

void OPL_SetPaused(int paused);


extern unsigned int opl_sample_rate;

void OPL_Render_Samples(void* dest, unsigned nsamp);


void OPL_SetCallback(uint64_t us, opl_callback_t callback, void* data);

void OPL_ClearCallbacks();

void OPL_AdjustCallbacks(float tempo);

#ifdef __cplusplus
}
#endif
