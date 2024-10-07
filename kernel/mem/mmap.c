#include <mem/mmap.h>
#include <stdlib.h>
#include "dev/log.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "proc/process.h"
#include "sys/mman.h"
#include "types/list.h"
#include "util.h"

static list_node* __find_existing_mmap(uintptr_t start, size_t sz) {
    foreach(bl, current_core->current_process->mmap) {
        mmap_block* _bl = bl->value;
        if(_bl->start == start && _bl->size == sz) {
            return bl;
        }
    }    
    return NULL;
}

static uintptr_t __alloc_region(size_t pages) {
    uintptr_t s = current_core->current_process->mmap_start;
    current_core->current_process->mmap_start += pages * PAGE_SIZE;
    return s;
}

mmap_block* k_mem_mmap(uintptr_t start, size_t size, uint8_t flags, int prot, int fd, off_t offset) {
    mmap_block* bl = malloc(sizeof(mmap_block));
    bl->size = size;

    size_t pages = PAGES(size);
    
    if(!start) {
        if(flags & MAP_FIXED) {
            k_debug("mmap: start == 0 when MAP_FIXED");
            return NULL;
        } else if(flags & MAP_ANONYMOUS) {
            start = __alloc_region(pages);
        } else {
            // TODO do something
        }
    }

    bl->start  = start;
    bl->flags  = flags;
    bl->fd     = fd;
    bl->offset = offset;

    list_push_back(current_core->current_process->mmap, bl);

    uint8_t page_flags = PM_FL_USER;

    if(prot == PROT_NONE) {
        k_debug("mmap: not mapping PROT_NONE");
        return bl;
    } else if(prot & PROT_WRITE) {
        page_flags |= PM_FL_WRITABLE;
    }

    k_mem_paging_map_pages_ex(start, pages, 0, page_flags);

    if(!(flags & MAP_ANONYMOUS)) {
        // TODO setup COW
    }

    return bl;
}

void k_mem_munmap(uintptr_t start, size_t size) {
    list_node* bl = __find_existing_mmap(start, size);
    
    if(!bl) {
        return;
    }
    
    mmap_block* mbl = bl->value;
    list_delete(current_core->current_process->mmap, bl);

    for(size_t i = 0; i < PAGES(size); i++) {
        k_mem_paging_unmap(mbl->start + i * PAGE_SIZE);
    }

    free(mbl);
}
