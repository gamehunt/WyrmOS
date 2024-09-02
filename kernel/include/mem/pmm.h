#ifndef __K_MEM_PMM_H
#define __K_MEM_PMM_H 1

#include <stdint.h>
#include <stddef.h>

#include <mem/mem.h>

typedef uint64_t frame;

#define FRAME(addr) ((addr) / 0x1000)
#define ADDR(frame) ((frame) * 0x1000)

#define palloc(frames) ADDR(k_mem_pmm_alloc(frames))

INTERNAL int k_mem_pmm_init();
int   k_mem_pmm_mark_region(frame start, size_t count);
int   k_mem_pmm_mark_frame(frame frame);

frame k_mem_pmm_alloc(size_t frames);

#endif
