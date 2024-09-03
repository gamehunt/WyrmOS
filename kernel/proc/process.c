#include <proc/process.h>

#include "cpu/interrupt.h"
#include "cpu/pic.h"
#include "dev/log.h"
#include "dev/pit.h"
#include "mem/paging.h"
#include "types/list.h"
#include "types/tree.h"

#include <stdlib.h>
#include <string.h>

#define KERNEL_STACK_SIZE PAGE_SIZE

static tree* __process_tree;
static list* __process_list;

static process* __current_process;

void __schedule(regs* r) {
	k_debug("__schedule()");
	IRQ_ACK(0);
}

int k_process_spawn_tasklet(const char* name, tasklet t) {
	return -1;
}

static void* __k_process_alloc_kernel_stack() {
	return malloc(KERNEL_STACK_SIZE);
}

static process* __k_process_create_init() {
	process* prc = k_process_create("init");

	return prc;
}

process* k_process_create(const char* name) {
	process* p = malloc(sizeof(process));
	memset(p, 0, sizeof(process));
	strncpy(p->name, name, PROCESS_NAME_LENGTH);
	p->ctx.kernel_stack = __k_process_alloc_kernel_stack();
	return p;
}

void k_process_spawn(process* p, process* parent) {
	if(parent) {
		p->tree_node = tree_append(parent->tree_node, p);
	} else {
		p->tree_node = tree_create(p);
	}

	p->list_node = list_push_back(__process_list, p);
	p->pid       = __process_list->size;
}

void k_process_init() {
	__process_list = list_create();

	process* init = __k_process_create_init();
	k_process_spawn(init, NULL);

	__process_tree  = init->tree_node;

	k_dev_pit_init();
	k_cpu_int_setup_handler(IRQ_TO_INT(0), __schedule);

	asm("sti");
}
