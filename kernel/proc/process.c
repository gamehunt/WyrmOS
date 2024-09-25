#include <proc/process.h>

#include "dev/log.h"
#include "mem/alloc.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "proc/smp.h"
#include "proc/spinlock.h"
#include "types/list.h"
#include "types/tree.h"

#include <stdlib.h>
#include <string.h>

static tree* __process_tree = NULL;
static list* __process_list = NULL;
static list* __ready_queue  = NULL;

struct core cores[MAX_CORES] = {0};
int core_count = 0;

static lock __process_queue_lock = EMPTY_LOCK;
static lock __process_list_lock = EMPTY_LOCK;
static lock __dump_lock = EMPTY_LOCK;

static void __dump_process(volatile context* ctx) {
    LOCK(__dump_lock);
    k_debug("---------------");
    k_debug("ctx   = %#.16lx", ctx);
    k_debug("core  = %d", current_core->id);
    k_debug("pml   = %#.16lx", ctx->pml);
    k_debug("stack = %#.16lx", ctx->kernel_stack);
    k_debug("rbp   = %#.16lx", ctx->rbp);
    k_debug("rsp   = %#.16lx", ctx->rsp);
    k_debug("rip   = %#.16lx", ctx->rip);
    UNLOCK(__dump_lock);
}

static void __k_proc_load_context(volatile context* ctx) {
	k_mem_paging_set_pml(ctx->pml);
	arch_set_kernel_stack((uintptr_t) ctx->kernel_stack);

	asm volatile ("" ::: "memory");

	arch_load_ctx(ctx);
	__builtin_unreachable();
}

process* k_process_get_ready() {
    LOCK(__process_queue_lock);
    list_node* node = list_pop_back(__ready_queue);
    process* proc = NULL; 
    if(!node) {
        proc = (process*) current_core->idle_process;
    } else {
        proc = node->value;
    }
    UNLOCK(__process_queue_lock);
    return proc;
}

void k_process_make_ready(process* p) {
    LOCK(__process_queue_lock);
    if(p->ready_node != NULL) {
        list_prepend(__ready_queue, p->ready_node);
    } else {
        p->ready_node = list_push_front(__ready_queue, p);
    }
    UNLOCK(__process_queue_lock);
}

void k_process_schedule_next() {
    current_core->current_process = k_process_get_ready();
	__k_proc_load_context(&current_core->current_process->ctx);
}

void k_process_yield() {
	if(current_core->current_process == current_core->idle_process) {
        k_warn("Preempted from idle process. Probably not a good thing.");
        goto next;
	}

	if(arch_save_ctx(&current_core->current_process->ctx)) {
		return; // Return from switch
	}

    k_process_make_ready((process*) current_core->current_process); 

next:
    k_process_schedule_next();
}

static void* __k_process_alloc_kernel_stack() {
	void* mem = (kmalloc_aligned(KERNEL_STACK_SIZE, 16));
	memset(mem, 0, KERNEL_STACK_SIZE);
	return mem + KERNEL_STACK_SIZE;
}

static process* __k_process_create_init() {
	process* prc = k_process_create("[init]");
	prc->ctx.pml = k_mem_paging_get_root_pml();
	return prc;
}

static void __k_process_idle_routine(void) {
	while(1) {
        asm volatile(
             "sti\n"
             "hlt\n"
             "cli\n"
        );
        k_process_schedule_next();
	}
}

process* k_process_create_idle() {
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
    LOCK(__process_list_lock);

	if(parent) {
		p->tree_node = tree_append(parent->tree_node, p);
	} else {
		p->tree_node = tree_create(p);
	}

	p->list_node = list_push_back(__process_list, p);
	p->pid       = __process_list->size;

    UNLOCK(__process_list_lock);
}

void k_process_init() {
	__process_list = list_create();
	__ready_queue  = list_create();

	process* init = __k_process_create_init();
	k_process_spawn(init, NULL);

	current_core->idle_process = k_process_create_idle();

	__process_tree  = init->tree_node;
	current_core->current_process = init;

    k_proc_init_cores();
}

void k_process_set_core(struct core* c) {
    arch_set_core_base(c);
}

EXPORT(k_process_init)
EXPORT(k_process_spawn)
EXPORT(k_process_yield)
EXPORT(k_process_get_ready)
EXPORT(k_process_set_core)
