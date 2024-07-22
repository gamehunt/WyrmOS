#include <mem/pmm.h>

#include <stdio.h>
#include <boot/limine.h>

#define FRAME_INDEX(frame) (frame / 64)
#define FRAME_BIT(frame)   (frame % 64)
#define FRAME_FROM(index, bit)  (bit + index * 64)

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

extern void* _kernel_end;

static uint64_t* __pmm_mem_bitmap        = 0;
static size_t    __pmm_bitmap_size       = 0;
static size_t    __pmm_bitmap_free_index = 0;
static uint8_t   __pmm_initialized       = 0;

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

int k_mem_pmm_init() {
	if(memmap_request.response == NULL 
			|| memmap_request.response->entry_count < 1) {
		printf("Memory map not found.\r\n");
		return -1;
	} else {
		printf("Memory map found.\r\n");
		for(uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
			struct limine_memmap_entry* entry = memmap_request.response->entries[i];
			printf("Mmap entry: %#.16lX - %#.16lX - %s\r\n", entry->base, entry->base + entry->length, mmap_type2str(entry->type));
			
			if(entry->type == LIMINE_MEMMAP_USABLE) {
				k_mem_pmm_mark_region(FRAME(entry->base), entry->length / 0x1000);
			}
		}
	}

	__pmm_initialized = 1;

	printf("Bitmap size: %dB\r\n", __pmm_bitmap_size * sizeof(uint64_t));

	return 0;
}

int k_mem_pmm_mark_region(frame start, size_t count) {
	for(size_t i = 0; i < count; i++) {
		k_mem_pmm_mark_frame(start + i);
	}
	return 0;
}

int k_mem_pmm_mark_frame(frame frame) {
	size_t index = FRAME_INDEX(frame);

	if(index > __pmm_bitmap_size) {
		if(!__pmm_initialized) {
			for(size_t i = __pmm_bitmap_size; i < index + 1; i++) {
				__pmm_mem_bitmap[i] = 0;
			}
			__pmm_bitmap_size = index + 1;
		} else {
			return -1;
		}
	}

	__pmm_mem_bitmap[index] |= (1 << FRAME_BIT(frame));
	return 0;
}

frame k_mem_pmm_alloc(size_t frames) {

    if (__pmm_bitmap_free_index >= __pmm_bitmap_size) {
		//OUT OF MEM
		return 0;
    }

    size_t found_frames = 0;

    frame   frame_n = 0;
    uint8_t found   = 0;

    for (size_t i = __pmm_bitmap_free_index; i < __pmm_bitmap_size; i++) {
        for (size_t j = 0; j < sizeof(__pmm_mem_bitmap[0]) * 8; j++) {
            if (__pmm_mem_bitmap[i] & (1 << j)) {
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
            __pmm_mem_bitmap[FRAME_INDEX(target_frame)] &= ~(1 << FRAME_BIT(target_frame));
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

        return ADDR(frame_n);
    }

	//OUT OF MEM
	return 0;
}
