#include <proc/process.h>

#include "arch.h"
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

static _Atomic pid_t __pid = 0;

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

static pid_t __get_pid() {
    return ++__pid;
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
    proc->flags |= PROCESS_RUNNING;
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
    process* prc = NULL;
    do {
        prc = k_process_get_ready();
    } while(prc->flags & PROCESS_FINISHED);
    current_core->current_process = prc;
	__k_proc_load_context(&current_core->current_process->ctx);
}

void k_process_yield() {
	if(current_core->current_process == current_core->idle_process) {
        k_warn("Preempted from idle process. Probably not a good thing.");
        goto next;
	}

    if(current_core->current_process->flags & PROCESS_FINISHED) {
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
    prc->flags   = PROCESS_RUNNING;
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

    prc->flags   = PROCESS_RUNNING;

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
    p->fds = list_create();
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
	p->pid       = __get_pid();

    k_debug("%s (%d) spawned", p->name, p->pid);

    UNLOCK(__process_list_lock);
}

pid_t k_process_fork() {
    process* cur   = (process*) current_core->current_process;
    process* fork  = k_process_create(cur->name);

    fork->ctx.pml  = k_mem_paging_clone_pml(cur->ctx.pml);
    fork->ctx.rsp  = (uintptr_t) fork->ctx.kernel_stack;
    fork->ctx.rbp  = (uintptr_t) fork->ctx.kernel_stack;
    fork->ctx.rip  = (uintptr_t) &arch_fork_ret;

    regs r;
    memcpy(&r, cur->syscall_state, sizeof(regs));
    r.rax = 0;
    PUSH(fork->ctx.rsp, regs, r)

    fork->syscall_state = (void*) fork->ctx.rsp;

    k_process_spawn(fork, cur);
    k_process_make_ready(fork);

    return fork->pid;
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

void k_process_exit(int code) {
    current_core->current_process->flags  = PROCESS_FINISHED;
    current_core->current_process->status = code;
    k_process_schedule_next();
}

int k_process_open_file(fs_node* node) {
    for(size_t i = 0; i < current_core->current_process->fds->size; i++) {
        list_node* free_fd = list_get(current_core->current_process->fds, i);
        if(!free_fd->value) {
            free_fd->value = node;
            return i;
        }
    }
    int s = current_core->current_process->fds->size;
    list_push_back(current_core->current_process->fds, node);
    return s;
}

int k_process_close_file(unsigned int fd) {
    if(current_core->current_process->fds->size <= fd) {
        return -1;
    } 
    list_node* _fd = list_get(current_core->current_process->fds, fd);
    if(!_fd || !_fd->value) {
        return -1;
    }
    k_fs_close(_fd->value);
    _fd->value = NULL;
    return 0;
}

fs_node* k_process_get_file(unsigned int fd) {
    if(current_core->current_process->fds->size <= fd) {
        return NULL;
    } 
    list_node* _fd = list_get(current_core->current_process->fds, fd);
    if(!_fd) {
        return NULL;
    }
    return _fd->value;
}

static int __pid_comparator(process* a, pid_t p) {
	return a->pid == p;
}

process* k_process_get_by_pid(pid_t pid) {
	list_node* prc = list_find_cmp(__process_list, (void*) pid, (comparator) __pid_comparator);
	if(prc) {
		return prc->value;
	} else {
		return NULL;
	}
}

int k_process_send_signal(pid_t pid, int sig) {
	process* target = k_process_get_by_pid(pid);
	if(!target) {
		return -1;
	}

}

EXPORT(k_process_init)
EXPORT(k_process_spawn)
EXPORT(k_process_yield)
EXPORT(k_process_get_ready)
EXPORT(k_process_set_core)
