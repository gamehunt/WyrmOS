#include "dev/log.h"
#include "proc/process.h"
#include <dev/timer.h>
#include <dev/pit.h>
#include <cpu/pic.h>
#include <cpu/interrupt.h>
#include <asm.h>

#include <types/list.h>

static list* __timer_callbacks;

static void __tick(regs* r) {
	foreach(cb, __timer_callbacks) {
		((timer_callback) cb->value)(r);
	}

	IRQ_ACK(0);

	if(r->cs != 0x08) {
		k_process_yield();
	}
}

void k_dev_timer_init() {
	k_dev_pit_init();
	__timer_callbacks = list_create();
	k_cpu_int_setup_irq_handler(0, __tick);
	sti();
}

void k_dev_timer_add_callback(timer_callback c) {
	cli();
	list_push_back(__timer_callbacks, c);
	sti();
}

EXPORT(k_dev_timer_add_callback)
