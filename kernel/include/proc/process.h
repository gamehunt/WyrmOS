#ifndef __K_PROC_PROCESS_H
#define __K_PROC_PROCESS_H 1

#include <cpu/interrupt.h>
#include <types/tree.h>
#include <sys/types.h>

#define PROCESS_NAME_LENGTH 128

typedef struct {
	pid_t   pid;
	char    name[PROCESS_NAME_LENGTH];
	context ctx;
	tree*      tree_node;
	list_node* list_node;
	list_node* ready_node;
} process;

typedef int(*tasklet)(void);

struct core {
    volatile process*    current_process;
    volatile process*    idle_process;
	volatile union page* pml;
};

extern struct core cores[];
extern int core_count;
static struct core __seg_gs * const current_core = 0;

INTERNAL void k_process_init();
INTERNAL void k_process_init_core();

process* k_process_create(const char* name);
void     k_process_spawn(process* p, process* parent);
void     k_process_yield();
void     k_process_set_core(struct core* addr);

#endif
