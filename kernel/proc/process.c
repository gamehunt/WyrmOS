#include <proc/process.h>

#include "arch.h"
#include "dev/log.h"
#include "fcntl.h"
#include "fs/fs.h"
#include "mem/alloc.h"
#include "mem/mem.h"
#include "mem/mmap.h"
#include "mem/paging.h"
#include "proc/smp.h"
#include "proc/spinlock.h"
#include "types/list.h"
#include "types/tree.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static tree* __process_tree = NULL;
static list* __process_list = NULL;
static list* __ready_queue  = NULL;
static list* __sleep_queue  = NULL;

struct core cores[MAX_CORES] = {0};
int core_count = 0;

static lock __process_queue_lock = EMPTY_LOCK;
static lock __process_list_lock  = EMPTY_LOCK;
static lock __dump_lock          = EMPTY_LOCK;
static lock __sig_lock           = EMPTY_LOCK;
static lock __sleep_lock         = EMPTY_LOCK;
static lock __block_lock         = EMPTY_LOCK;

static _Atomic pid_t __pid = 0;

#define SIG_ACT_IGNORE    0
#define SIG_ACT_TERMINATE 1
static const uint8_t __signal_defaults[] = {
    [SIGKILL] = SIG_ACT_TERMINATE, 
    [SIGABRT] = SIG_ACT_TERMINATE, 
    [SIGTERM] = SIG_ACT_TERMINATE,
    [SIGSEGV] = SIG_ACT_TERMINATE,
	[SIGCHLD] = SIG_ACT_IGNORE
};

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
    process* proc = NULL; 
    do {
        list_node* node = list_pop_back(__ready_queue);
        if(!node) {
            proc = (process*) current_core->idle_process;
        } else {
            proc = node->value;
        }
    } while(proc->flags & PROCESS_FINISHED);
    assert((proc->flags & PROCESS_FINISHED) == 0);
    proc->flags |= PROCESS_RUNNING;
    UNLOCK(__process_queue_lock);
    return proc;
}

void k_process_make_ready(process* p) {
    LOCK(__process_queue_lock);
    p->sleep_seconds    = 0;
    p->sleep_subseconds = 0;
    list_prepend(__ready_queue, p->ready_node);
    UNLOCK(__process_queue_lock);
}

void k_process_schedule_next() {
    process* prc = k_process_get_ready();
    current_core->current_process = prc;
	__k_proc_load_context(&current_core->current_process->ctx);
}

void k_process_switch(int flags) {
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

    if(flags & SWITCH_RESCHEDULE) {
        k_process_make_ready((process*) current_core->current_process); 
    }

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
	prc->ctx.pml = k_mem_paging_clone_pml(NULL);
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
    p->fds        = list_create();
    p->mmap_start = MMAP_START;
    p->mmap       = list_create();
    p->list_node  = list_create_node(p);
    p->tree_node  = tree_create(p);
    p->ready_node = list_create_node(p);
    p->sleep_node = list_create_node(p);
    p->wait_queue = list_create();
	return p;
}

void k_process_spawn(process* p, process* parent) {
    LOCK(__process_list_lock);

	list_append(__process_list, p->list_node);
	p->pid       = __get_pid();

	if(parent) {
		tree_append_child(parent->tree_node, p->tree_node);
	} 

    k_debug("%s (%d) spawned", p->name, p->pid);

    UNLOCK(__process_list_lock);
}

pid_t k_process_fork() {
    process* cur   = (process*) current_core->current_process;
    process* fork  = k_process_create(cur->name);

    memcpy(fork->signals, cur->signals, sizeof(cur->signals));

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
    __sleep_queue  = list_create();

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
    k_debug("%s (%d) exited with code %d", 
            current_core->current_process->name, 
            current_core->current_process->pid,
            code);
    process* prc = current_core->current_process;
    prc->flags  = PROCESS_FINISHED;
    prc->status = code;
	for(int i = 0; i < prc->fds->size; i++) {
		k_process_close_file(i);
	}
	foreach(mbl, prc->mmap) {
		k_mem_unmap_block(mbl->value);
	}
	list_clear(prc->mmap);
	if(prc->tree_node->parent) {
		process* parent = prc->tree_node->parent->value;
		if(parent) {
			k_process_wakeup_queue(parent->wait_queue);
			k_process_send_signal(parent->pid, SIGCHLD);
		}
	}
    k_process_schedule_next();
}

int k_process_open_file(fs_node* node) {
    fd_entry* f = malloc(sizeof(fd_entry));
    f->node   = node;
    f->offset = 0;
    for(size_t i = 0; i < current_core->current_process->fds->size; i++) {
        list_node* free_fd = list_get(current_core->current_process->fds, i);
        if(!free_fd->value) {
            free_fd->value = f;
            f->id = i;
            return i;
        }
    }
    f->id = current_core->current_process->fds->size;
    list_push_back(current_core->current_process->fds, f);
    return f->id;
}

int k_process_close_file(unsigned int fd) {
    if(current_core->current_process->fds->size <= fd) {
        return -1;
    } 
    list_node* _fd = list_get(current_core->current_process->fds, fd);
    if(!_fd || !_fd->value) {
        return -1;
    }
    fd_entry* f = _fd->value;
    k_fs_close(f->node);
    free(f);
    _fd->value = NULL;
    return 0;
}

fd_entry* k_process_get_file(unsigned int fd) {
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
	return a->pid != p;
}

process* k_process_get_by_pid(pid_t pid) {
    LOCK(__process_list_lock);
	list_node* prc = list_find_cmp(__process_list, (void*) pid, (comparator) __pid_comparator);
    UNLOCK(__process_list_lock);
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

    if(sig < 0 || sig >= NSIG) {
        return -1;
    }

    if(!target->signals[sig].handler && __signal_defaults[sig] == SIG_ACT_IGNORE) {
        return 0;
    }

    LOCK(__sig_lock);
    set_sig_pending(target, sig);
    UNLOCK(__sig_lock);

    if(target == current_core->current_process) {
        return 0;
    }
    
    if(target->ready_node->owner == NULL && !(target->flags & PROCESS_RUNNING)) {
        k_process_make_ready(target);
    }

    return 0;
}

int k_process_handle_signal(int sig, regs* r) {
    volatile process* cur = current_core->current_process;
    
    if(cur->flags & PROCESS_FINISHED) {
        return 1;
    }

    signal s = cur->signals[sig];
    if(!s.handler) {
        switch(__signal_defaults[sig]) {
            case SIG_ACT_IGNORE:
                return current_pending() == 0;
            case SIG_ACT_TERMINATE:
                k_process_exit((128 + sig) << 8);
                __builtin_unreachable();
        }
    }

    arch_enter_signal(s.handler, sig, r);
    return 1;
}

void k_process_exit_signal(regs* r) {
    arch_exit_signal(r);
}

void k_process_invoke_signals(regs* r) {
start:
    LOCK(__sig_lock);
    int sigset = current_pending();
    int signal = 0;
    while(sigset && signal < NSIG) {
        if(sigset & 1) {
            clear_sig_pending(current_core->current_process, signal);
            UNLOCK(__sig_lock);
            if(k_process_handle_signal(signal, r)) {
                return;
            }
            goto start;
        } 
        sigset >>= 1;
        signal++;
    }
    UNLOCK(__sig_lock);
}

void k_process_update_timings() {
    uint64_t clock_ticks = arch_get_ticks() / arch_get_cpu_speed();
    uint64_t seconds    = clock_ticks / SUBSECONDS_PER_SECOND;
    uint64_t subseconds = clock_ticks % SUBSECONDS_PER_SECOND;
    LOCK(__sleep_lock);
    if(__sleep_queue->head) {
        process* p = ((process*)__sleep_queue->head->value); 
        while(p) {
            uint64_t end_secs    = p->sleep_seconds;
            uint64_t end_subsecs = p->sleep_subseconds;
            if((seconds > end_secs) || 
                    (seconds == end_secs && 
                     subseconds >= end_subsecs)) {
                if(!is_ready(p)) {
                    k_process_make_ready(p);
                }
                list_pop_front(__sleep_queue);
                if(__sleep_queue->head) {
                    p = __sleep_queue->head->value;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
    } 
    UNLOCK(__sleep_lock);
}

void k_process_sleep_on_queue(list* queue) {
    if(is_locked(current_core->current_process)) {
        goto end;
    }
    LOCK(__block_lock);
    current_core->current_process->flags = PROCESS_SLEEPING;
    list_append(queue, current_core->current_process->sleep_node);
    UNLOCK(__block_lock);
end:
    k_process_switch(0);
}

void k_process_wakeup_queue(list* queue) {
    assert(queue != NULL);
    LOCK(__block_lock);
    while(queue->size) {
        process* p = list_pop_back(queue)->value;
        if(!(p->flags & PROCESS_FINISHED)) {
            k_process_make_ready(p);
        }
    }
    UNLOCK(__block_lock);
}

void k_process_sleep(uint64_t seconds, uint64_t subseconds) {
    LOCK(__sleep_lock);

    if(is_locked(current_core->current_process)) {
        goto end;
    }

    volatile process* cur = current_core->current_process;

    cur->sleep_seconds    = seconds;
    cur->sleep_subseconds = subseconds;
    cur->flags = PROCESS_SLEEPING;

    list_node* par = NULL;
    foreach(node, __sleep_queue) {
        process* _par = node->value;
        if(_par->sleep_seconds > cur->sleep_seconds || 
                (_par->sleep_seconds == cur->sleep_seconds 
                 && _par->sleep_subseconds > cur->sleep_subseconds)) {
            break;
        }
        par = node; 
    }

    if(!par) {
        list_prepend(__sleep_queue, cur->sleep_node); 
    } else {
        list_insert_after(__sleep_queue, par, (void*) cur); 
    }

end:
    UNLOCK(__sleep_lock);
}

uint8_t __waitpid_can_pick(process* proc, process* parent, int pid) {
	if(pid < -1) {
		return proc->pid == -pid;
	} else if(pid == 0) {
		return 0; // -- TODO process->group_id == parent->group_id; 
	} else if(pid > 0) {
		return proc->pid == pid;
	} else {
		return 1;
	}
}

pid_t k_process_waitpid(pid_t pid, int* status, int options) {
	if(options > PROCESS_WAITPID_WUNTRACED) {
		return -1;
	}

    volatile process* proc = current_core->current_process;

	do {
		process* child = NULL;
		uint8_t was = 0;
		LOCK(__process_list_lock);
		foreach(c, proc->tree_node->children) {
			process* candidate = ((tree*)c->value)->value;
			if(__waitpid_can_pick(candidate, proc, pid)) {
				was = 1;
				if(candidate->flags & PROCESS_FINISHED) {
					child = candidate;
					break;
				}	
			}
		}
		UNLOCK(__process_list_lock);

		if(!was) {
			return -2;
		}

		if(child) {
			if(status && validate_ptr(status, sizeof(uintptr_t))){
				*status = child->status;
			}
			pid_t cp = child->pid;
			// k_process_destroy(child);
			return cp;
		} else if(!(options & PROCESS_WAITPID_WNOHANG)) {
			k_process_sleep_on_queue(proc->wait_queue);
		} else{
			return -3;
		}
	} while(1);
}

void k_process_destroy(process* prc) {
	LOCK(__process_list_lock);
	list_free(prc->wait_queue);
	list_free(prc->fds);
	list_free(prc->mmap);
	free(prc->ready_node);
	free(prc->sleep_node);
	free(prc->ctx.kernel_stack);
	tree_free(prc->tree_node);
	list_delete(__process_list, prc->list_node);
	k_mem_paging_free_pml(prc->ctx.pml);
	free(prc);
	UNLOCK(__process_list_lock);
}
