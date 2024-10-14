#include "dev/log.h"
#include "mem/paging.h"
#include "panic.h"
#include "proc/spinlock.h"

#include <stdio.h>
#include <assert.h>

#include <mem/pmm.h>
#include <boot/limine.h>
#include <string.h>
#include <util.h>

#define FRAME_INDEX(frame)      (frame / 64)
#define FRAME_BIT(frame)        (frame % 64)
#define FRAME_FROM(index, bit)  (bit + index * 64)

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static uint64_t*  __pmm_mem_bitmap        = 0;
static size_t     __pmm_bitmap_size       = 0;
static size_t     __pmm_bitmap_free_index = 0;
static uint8_t    __pmm_initialized       = 0;
static lock       __pmm_lock              = EMPTY_LOCK;

static const char* mmap_type2str(int type) {
    switch(type) {
        case LIMINE_MEMMAP_USABLE:
            return "Usable";
        case LIMINE_MEMMAP_RESERVED:
            return "RESERVED";
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            return "ACPI RECLAIMABLE";
        case LIMINE_MEMMAP_ACPI_NVS:
            return "ACPI_NVS";
        case LIMINE_MEMMAP_BAD_MEMORY:
            return "BAD";
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            return "BOOTLOADER RECLAIMABLE";
        case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            return "KERNEL";
        case LIMINE_MEMMAP_FRAMEBUFFER:
            return "FRAMEBUFFER";
        default:
            return "UNKNOWN";
    }
}

static uint64_t* allocate_pmm_bitmap(uint64_t size) {
    assert(size % PAGE_SIZE == 0);
    uint64_t result = 0;
    for(uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        if(entry->type == LIMINE_MEMMAP_USABLE && entry->length >= size) {
            result = TO_VIRTUAL(entry->base);
            entry->base   += size;
            entry->length -= size;
            if(!entry->length) {
                entry->type = LIMINE_MEMMAP_KERNEL_AND_MODULES;
            }
            break;
        }
    }
    return (uint64_t*) result;
}

int k_mem_pmm_init() {
    if(memmap_request.response == NULL 
            || memmap_request.response->entry_count < 1) {
        k_warn("Memory map not found.");
        return -1;
    } else {
        k_info("Memory map found.");
        struct limine_memmap_entry* last_entry = memmap_request.response->entries[memmap_request.response->entry_count - 1];
        uint64_t required_bitmap_size = FRAME_INDEX(FRAME(last_entry->base + last_entry->length));
        uint64_t required_bytes = ALIGN(required_bitmap_size * 8, PAGE_SIZE);
        k_info("Required bitmap size: %ld bytes (%ld index).", required_bytes, required_bitmap_size);
        __pmm_mem_bitmap  = allocate_pmm_bitmap(required_bytes);
        memset(__pmm_mem_bitmap, 0, required_bytes);
        k_info("Allocated bitmap at %#.16lx.", __pmm_mem_bitmap);
        for(uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
            struct limine_memmap_entry* entry = memmap_request.response->entries[i];
            k_info("Mmap entry: %#.16lx - %#.16lx - %s (%ld pages)", entry->base, entry->base + entry->length, mmap_type2str(entry->type), entry->length / PAGE_SIZE);
            if(entry->type == LIMINE_MEMMAP_USABLE) {
                k_mem_pmm_mark_region(FRAME(entry->base), entry->length / 0x1000);
            }
        }
    }
    __pmm_initialized = 1;
    return 0;
}

int k_mem_pmm_mark_region(frame start, size_t count) {
    for(size_t i = 0; i < count; i++) {
        k_mem_pmm_mark_frame(start + i);
    }
    return 0;
}

int k_mem_pmm_mark_frame(frame frame) {
    LOCK(__pmm_lock);

    size_t index = FRAME_INDEX(frame);

    if(index > __pmm_bitmap_size) {
        if(!__pmm_initialized) {
            __pmm_bitmap_size = index + 1;
        } else {
            UNLOCK(__pmm_lock);
            return -1;
        }
    }

    __pmm_mem_bitmap[index] |= ((uint64_t)1 << FRAME_BIT(frame));
    if(!__pmm_mem_bitmap[__pmm_bitmap_free_index]) {
        __pmm_bitmap_free_index = index;
    }

    UNLOCK(__pmm_lock);

    return 0;
}

frame k_mem_pmm_alloc(size_t frames) {
    LOCK(__pmm_lock);

    if (__pmm_bitmap_free_index >= __pmm_bitmap_size) {
        panic(NULL, "Out of memory.");
    }

    size_t found_frames = 0;

    frame   frame_n = 0;
    uint8_t found   = 0;

    for (size_t i = __pmm_bitmap_free_index; i < __pmm_bitmap_size; i++) {
        for (size_t j = 0; j < 64; j++) {
            if (__pmm_mem_bitmap[i] & ((uint64_t)1 << j)) {
                if (!found_frames) {
                    frame_n = FRAME_FROM(i, j);
                }
                found_frames++;
            } else {
                found_frames = 0;
            }
            if (found_frames >= frames) {
                found = 1;
                break;
            }
        }
        if (found) {
            break;
        }
    }

    if (found) {
        for (size_t i = 0; i < frames; i++) {
            frame target_frame = frame_n + i;
            __pmm_mem_bitmap[FRAME_INDEX(target_frame)] &= ~((uint64_t)1 << FRAME_BIT(target_frame));
        }

        while (__pmm_bitmap_free_index < __pmm_bitmap_size 
                && !__pmm_mem_bitmap[__pmm_bitmap_free_index]) {
            __pmm_bitmap_free_index++;
        }
        
        if(__pmm_bitmap_free_index >= __pmm_bitmap_size) {
            __pmm_bitmap_free_index = 0;
        }

        while (__pmm_bitmap_free_index < __pmm_bitmap_size 
                && !__pmm_mem_bitmap[__pmm_bitmap_free_index]) {
            __pmm_bitmap_free_index++;
        }

        UNLOCK(__pmm_lock);

        return ADDR(frame_n);
    }

    panic(NULL, "Out of memory.");
}
