#ifndef __K_MEM_PMM_H
#define __K_MEM_PMM_H 1

#include "symbols.h"
#include <stdint.h>
#include <stddef.h>

#include <mem/mem.h>

typedef uint64_t frame;

#define FRAME(addr) ((addr) >> 12)
#define ADDR(frame) ((frame) << 12)

#define palloc(frames) ADDR(k_mem_pmm_alloc(frames))

INTERNAL int k_mem_pmm_init();
int   k_mem_pmm_mark_region(frame start, size_t count);
int   k_mem_pmm_mark_frame(frame frame);

#define k_mem_pmm_free(f) k_mem_pmm_mark_frame(f)
#define k_mem_pmm_free_frames(f, s) k_mem_pmm_mark_region(f, s)

frame k_mem_pmm_alloc(size_t frames);

#endif
