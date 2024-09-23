#include <proc/process.h>

#include "mem/alloc.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "proc/smp.h"
#include "types/list.h"
#include "types/tree.h"

#include <stdlib.h>
#include <string.h>

#define KERNEL_STACK_SIZE PAGE_SIZE

static tree* __process_tree;
static list* __process_list;
static list* __ready_queue;

struct core cores[MAX_CORES] = {0};
int core_count = 0;


static void __k_proc_load_context(volatile context* ctx) {
	k_mem_paging_set_pml(ctx->pml);
	arch_set_kernel_stack((uintptr_t) ctx->kernel_stack);

	asm volatile ("" ::: "memory");

	arch_load_ctx(ctx);
	__builtin_unreachable();
}

// FIXME crashes with -O2
void __attribute__((optimize("O1")))  k_process_yield() {
	volatile process* old = current_core->current_process;
	list_node* new = list_pop_back(__ready_queue);
	if(new) {
		current_core->current_process = new->value;	
	} else {
		current_core->current_process = current_core->idle_process;
	}
	if(old != current_core->idle_process) {
		list_prepend(__ready_queue, old->ready_node);
		if(arch_save_ctx(&old->ctx)) {
			return; // Return from switch
		}
	}
	__k_proc_load_context(&current_core->current_process->ctx);
}

static void* __k_process_alloc_kernel_stack() {
	void* mem = (kmalloc_aligned(KERNEL_STACK_SIZE, 16));
	memset(mem, 0, KERNEL_STACK_SIZE);
	return mem + KERNEL_STACK_SIZE;
}

static process* __k_process_create_init() {
	process* prc = k_process_create("[init]");
	prc->ctx.pml = k_mem_paging_clone_pml(NULL);
    k_mem_paging_set_pml(prc->ctx.pml);
	return prc;
}

static void __k_process_idle_routine(void) {
	while(1) {
		asm volatile ("sti");
		asm volatile ("hlt");
		asm volatile ("cli");
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

	p->ready_node = list_push_back(__ready_queue, p);
}

void k_process_init() {
	__process_list = list_create();
	__ready_queue  = list_create();

	process* init = __k_process_create_init();
	k_process_spawn(init, NULL);

	current_core->idle_process = __k_process_create_idle();

	__process_tree  = init->tree_node;
	current_core->current_process = init;

    k_proc_init_cores();
}

void k_process_init_core() {
	current_core->idle_process    = __k_process_create_idle();
	current_core->current_process = current_core->idle_process;
    k_process_yield();
}

void k_process_set_core(struct core* c) {
    arch_set_core_base(c);
}

EXPORT(k_process_init)
EXPORT(k_process_spawn)
EXPORT(k_process_yield)
