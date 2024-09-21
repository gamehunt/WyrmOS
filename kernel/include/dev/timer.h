#ifndef __K_DEV_TIMER_H
#define __K_DEV_TIMER_H

#include <cpu/interrupt.h>
#include <symbols.h>

typedef void(*timer_callback)(regs*);

void     k_dev_timer_add_callback(timer_callback);
uint64_t k_dev_timer_ticks();

#endif
