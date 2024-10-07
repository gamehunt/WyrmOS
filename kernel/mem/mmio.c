#include <mem/mmio.h>
#include <stdlib.h>
#include "dev/log.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "mem/pmm.h"
#include "proc/spinlock.h"
#include "types/list.h"
#include "util.h"
#include "assert.h"

uintptr_t __mmio_start = MMIO_START;

typedef struct {
    uintptr_t phys;
    void*     addr;
} iomap_region;

static list* __iomapped_regions = NULL;
static lock  __iomap_lock = EMPTY_LOCK;

static int8_t __region_comparator(iomap_region* data, uintptr_t phys) {
    if(data->phys < phys) {
        return -1;
    } else if(data->phys > phys) {
        return 1;
    } else {
        return 0;
    }
}

static void* __try_get_present(uintptr_t  addr) {
    assert(__iomapped_regions != NULL);
    list_node* reg = list_find_cmp(__iomapped_regions, (void*) addr, (comparator) __region_comparator);
    if(reg) {
        return ((iomap_region*)reg->value)->addr;
    }
    return NULL;
} 

void* k_mem_iomap(uintptr_t phys, size_t size) {
    if(__mmio_start >= MMIO_END) {
        return NULL;
    }
    LOCK(__iomap_lock);
    if(!__iomapped_regions) {
        __iomapped_regions = list_create();
    }
    void* r = __try_get_present(phys);
    if(r) {
        UNLOCK(__iomap_lock);
        return r;
    }
    size_t pages = PAGES(size);
    r = (void*) __mmio_start;
    k_mem_paging_map_pages_ex(__mmio_start, pages, phys, PM_FL_NOCACHE);
    __mmio_start += pages * PAGE_SIZE;
    iomap_region* region = malloc(sizeof(iomap_region));
    region->phys = phys;
    region->addr = r;
    list_push_back(__iomapped_regions, region);
    UNLOCK(__iomap_lock);
    return r;
}

void  k_mem_iounmap(void* mem) {
    k_verbose("k_mem_iounmap(): Unimplemented.");
}

void* k_mem_alloc_dma(size_t size, uintptr_t* phys) {
    uint64_t p = k_mem_pmm_alloc(PAGES(size));
    if(phys) {
        *phys = p;
    }
    return k_mem_iomap(p, size);
}

void k_mem_free_dma(void* mem, size_t size) {
    size_t pages = PAGES(size);
    k_mem_pmm_free_frames(k_mem_paging_get_physical((uintptr_t) mem), pages);
}
