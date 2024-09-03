#include <proc/process.h>

#include "dev/log.h"
#include "dev/timer.h"
#include "mem/alloc.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "types/list.h"
#include "types/tree.h"

#include <stdlib.h>
#include <string.h>

#define KERNEL_STACK_SIZE PAGE_SIZE

static tree* __process_tree;
static list* __process_list;

static process* __current_process;
static process* __idle_process;

static list* __ready_queue;

extern __attribute__((noreturn))      void __load_ctx(context* ctx);
extern __attribute__((returns_twice)) int  __save_ctx(context* ctx);

static void __k_proc_load_context(context* ctx) {
	k_mem_paging_set_pml(ctx->pml);
	k_mem_set_kernel_stack((uintptr_t) ctx->kernel_stack);
	__load_ctx(ctx);
}

void k_process_yield() {
	process* old = __current_process;
	list_node* new = list_pop_back(__ready_queue);
	if(new) {
		__current_process = new->value;	
	} else {
		__current_process = __idle_process;
	}
	if(old != __idle_process) {
		list_push_front(__ready_queue, old);
		if(__save_ctx(&old->ctx)) {
			return; // Return from switch
		}
	}
	__k_proc_load_context(&__current_process->ctx);
}

int k_process_spawn_tasklet(const char* name, tasklet t) {
	return -1;
}

static void* __k_process_alloc_kernel_stack() {
	return (kmalloc_aligned(KERNEL_STACK_SIZE, 16));
}

static process* __k_process_create_init() {
	process* prc = k_process_create("[init]");
	prc->ctx.pml = k_mem_paging_get_root_pml();
	return prc;
}

static void __k_process_idle_routine(void) {
	while(1) {
		asm("hlt");
	}
}

static process* __k_process_create_idle() {
	process* prc = k_process_create("[idle]");

	prc->pid     = -1;
	prc->ctx.rip = (uintptr_t) &__k_process_idle_routine;
	prc->ctx.rsp = (uintptr_t) prc->ctx.kernel_stack;
	prc->ctx.rbp = (uintptr_t) prc->ctx.kernel_stack;
	prc->ctx.pml = k_mem_paging_get_root_pml();

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

	list_push_back(__ready_queue, p);
}

void k_process_init() {
	__process_list = list_create();
	__ready_queue  = list_create();

	process* init = __k_process_create_init();
	k_process_spawn(init, NULL);

	__idle_process = __k_process_create_idle();

	__process_tree  = init->tree_node;
	__current_process = init;

	k_dev_timer_init();
}

EXPORT(k_process_init)
EXPORT(k_process_spawn)
EXPORT(k_process_spawn_tasklet)
EXPORT(k_process_yield)
