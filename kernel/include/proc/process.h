#ifndef __K_PROC_PROCESS_H
#define __K_PROC_PROCESS_H 1

#include <cpu/interrupt.h>
#include <types/tree.h>
#include <sys/types.h>
#include "fs/fs.h"

#define PROCESS_NAME_LENGTH 128
#define MAX_CORES 32

#define PROCESS_RUNNING  (1 << 0)
#define PROCESS_FINISHED (1 << 1)
#define PROCESS_SLEEPING (1 << 2)

typedef struct {
    unsigned int id;
    fs_node* node;
} fd;

typedef struct {
	pid_t      pid;
	char       name[PROCESS_NAME_LENGTH];
    _Atomic uint16_t flags;
    int        status;
	context    ctx;
    regs*      syscall_state;
    list*      fds;
	tree*      tree_node;
	list_node* list_node;
	list_node* ready_node;
} process;

typedef int(*tasklet)(void);

struct core {
    uint32_t id;
    uint32_t lapic_id;
    volatile process*    current_process;
    volatile process*    idle_process;
	volatile union page* pml;
};

extern struct core cores[MAX_CORES];
extern int core_count;
static struct core __seg_gs * const current_core = 0;

INTERNAL void     k_process_init();
INTERNAL process* k_process_create_idle();

process* k_process_get_ready();
void     k_process_make_ready(process* p);
process* k_process_create(const char* name);
void     k_process_spawn(process* p, process* parent);
pid_t    k_process_fork();
void     k_process_yield();
void     k_process_schedule_next();
void     k_process_set_core(struct core* addr);
void     k_process_exit(int code);
int      k_process_open_file(fs_node* node);
int      k_process_close_file(unsigned int fd);
fs_node* k_process_get_file(unsigned int fd);

#endif
