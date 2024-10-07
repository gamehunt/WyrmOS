#ifndef __K_PROC_PROCESS_H
#define __K_PROC_PROCESS_H 1

#include <cpu/interrupt.h>
#include <types/tree.h>
#include <sys/types.h>
#include "fs/fs.h"
#include "sys/signal.h"

#define PROCESS_NAME_LENGTH 128
#define MAX_CORES 32

#define PROCESS_RUNNING  (1 << 0)
#define PROCESS_FINISHED (1 << 1)
#define PROCESS_SLEEPING (1 << 2)

#define is_ready(p)  (p->ready_node->owner != NULL)
#define is_locked(p) (p->sleep_node->owner != NULL)

typedef struct {
    unsigned int id;
    fs_node* node;
} fd;

typedef struct {
    uintptr_t handler;
} signal;

typedef struct {
	pid_t      pid;
	char       name[PROCESS_NAME_LENGTH];
    _Atomic uint16_t flags;
    int        status;
	context    ctx;
	signal     signals[NSIG + 1];
	sigset_t   pending_signals;
	sigset_t   blocked_signals;
	sigset_t   awaiting_signals;
    regs*      syscall_state;
    uint64_t   sleep_seconds;
    uint64_t   sleep_subseconds;
    list*      fds;
    uintptr_t  mmap_start;
    list*      mmap;
	tree*      tree_node;
	list_node* list_node;
	list_node* ready_node;
    list_node* sleep_node;
} process;

#define pending(prc) prc->pending_signals
#define current_pending() pending(current_core->current_process)
#define set_sig_pending(prc, n) prc->pending_signals |= (1ULL << n)
#define clear_sig_pending(prc, n) prc->pending_signals &= ~(1ULL << n)

typedef int(*tasklet)(void);

struct core {
    uint32_t id;
    uint32_t lapic_id;
    volatile process*    current_process;
    volatile process*    idle_process;
	volatile union page* pml;
};

#define SWITCH_RESCHEDULE (1 << 0)

extern struct core cores[MAX_CORES];
extern int core_count;
static struct core __seg_gs * const current_core = 0;

INTERNAL void     k_process_init();
INTERNAL process* k_process_create_idle();

process* k_process_get_ready();
void     k_process_make_ready(process* p);
process* k_process_create(const char* name);
process* k_process_get_by_pid(pid_t pid);
void     k_process_spawn(process* p, process* parent);
pid_t    k_process_fork();
void     k_process_switch(int flags);
#define  k_process_yield() k_process_switch(SWITCH_RESCHEDULE);
void     k_process_schedule_next();
void     k_process_set_core(struct core* addr);
void     k_process_exit(int code);
int      k_process_open_file(fs_node* node);
int      k_process_close_file(unsigned int fd);
fs_node* k_process_get_file(unsigned int fd);
int      k_process_send_signal(pid_t pid, int sig);
int      k_process_handle_signal(int sig, regs* r);
void     k_process_exit_signal(regs* r);
void     k_process_invoke_signals(regs* r);
void     k_process_update_timings();

void     k_process_sleep_on_queue(list* queue);
void     k_process_wakeup_queue(list* queue);
void     k_process_sleep(uint64_t seconds, uint64_t subseconds);

#endif
