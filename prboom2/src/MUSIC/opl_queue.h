// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//     OPL callback queue.

#pragma once

#include "opl.h"

typedef struct opl_callback_queue_s opl_callback_queue_t;

opl_callback_queue_t *OPL_Queue_Create(void);
int OPL_Queue_IsEmpty(opl_callback_queue_t *queue);
void OPL_Queue_Clear(opl_callback_queue_t *queue);
void OPL_Queue_Destroy(opl_callback_queue_t *queue);
void OPL_Queue_Push(opl_callback_queue_t *queue,
                    opl_callback_t callback, void *data,
                    uint64_t time);
int OPL_Queue_Pop(opl_callback_queue_t *queue,
                  opl_callback_t *callback, void **data);
uint64_t OPL_Queue_Peek(opl_callback_queue_t *queue);
void OPL_Queue_AdjustCallbacks(opl_callback_queue_t *queue,
                               uint64_t time, float factor);
