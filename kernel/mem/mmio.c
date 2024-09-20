#include <mem/mmio.h>
#include <stdlib.h>
#include "dev/log.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "types/list.h"
#include "util.h"
#include "assert.h"

uintptr_t __mmio_start = MMIO_START;

typedef struct {
    uintptr_t phys;
    void*     addr;
} iomap_region;

static list* __iomapped_regions = NULL;

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
    if(!__iomapped_regions) {
        __iomapped_regions = list_create();
    }
    void* r = __try_get_present(phys);
    if(r) {
        return r;
    }
    size_t pages = PAGES(size);
    r = (void*) __mmio_start;
    k_mem_paging_map_pages(__mmio_start, pages, phys);
    __mmio_start += pages * PAGE_SIZE;
    iomap_region* region = malloc(sizeof(iomap_region));
    region->phys = phys;
    region->addr = r;
    list_push_back(__iomapped_regions, region);
    return r;
}

void  k_mem_iounmap(void* mem) {
    k_verbose("k_mem_iounmap(): Unimplemented.");
}
