#ifndef __K_PROC_PROCESS_H
#define __K_PROC_PROCESS_H 1

#include <cpu/interrupt.h>
#include <types/tree.h>
#include <sys/types.h>

#define PROCESS_NAME_LENGTH 128

typedef struct {
	regs        registers;
	union page* pml;
	void*       kernel_stack;
} context;

typedef struct {
	pid_t   pid;
	char    name[PROCESS_NAME_LENGTH];
	context ctx;
	tree*      tree_node;
	list_node* list_node;
} process;

typedef int(*tasklet)(void);

INTERNAL void k_process_init();


process* k_process_create(const char* name);
void     k_process_spawn(process* p, process* parent);

int k_process_spawn_tasklet(const char* name, tasklet t);

#endif
