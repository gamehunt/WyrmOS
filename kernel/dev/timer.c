#include "arch/arch.h"
#include "proc/process.h"
#include <dev/timer.h>
#include <dev/pit.h>
#include <cpu/pic.h>
#include <cpu/interrupt.h>
#include <asm.h>

#include <types/list.h>

static list* __timer_callbacks = NULL;

static void __tick(regs* r) {

    if(__timer_callbacks) {
	    foreach(cb, __timer_callbacks) {
	    	((timer_callback) cb->value)(r);
	    }
    }

	IRQ_ACK(0);

	if(r->cs != 0x08) {
		k_process_yield();
	}
}

void k_dev_timer_add_callback(timer_callback c) {
	cli();
    if(!__timer_callbacks) {
        __timer_callbacks = list_create();
    }
	list_push_back(__timer_callbacks, c);
	sti();
}

uint64_t k_dev_timer_ticks() {
    return arch_get_ticks();
}

EXPORT(k_dev_timer_add_callback)
