#include <stdio.h>
#include <symbols.h>
#include <mem/mem.h>
#include <mem/paging.h>

#include <stddef.h>
#include <string.h>
#include <dev/log.h>

extern void* __kernel_exports_start;
extern void* __kernel_exports_end;

static symbol* __symtable = (symbol*) SYMTABLE;
static size_t  __symtable_size = 0;

void k_setup_symbols() {
	size_t bytes = (size_t) ((uintptr_t)&__kernel_exports_end - (uintptr_t)&__kernel_exports_start);
	size_t pages = (bytes / PAGE_SIZE) + !!(bytes % PAGE_SIZE);

	k_mem_paging_map_pages(SYMTABLE, pages, 0);

	__symtable_size = bytes / sizeof(symbol);
	memcpy(__symtable, &__kernel_exports_start, bytes);

	k_verbose("Symtable occupies %d bytes (%d symbols)", bytes, __symtable_size);
}

symbol* k_lookup_symbol(const char* name) {
	for(size_t i = 0; i < __symtable_size; i++) {
		if(!strcmp(__symtable[i].name, name) && !__symtable[i].internal) {
			return &__symtable[i];
		}
	}
	return NULL;
}

symbol* k_find_nearest_symbol(uintptr_t address) {
	if(address < VIRTUAL_BASE) {
		return NULL;
	}	
	symbol* nearest = NULL;
	uintptr_t  nearest_distance = 0xFFFFffffFFFFffff; 
	for(size_t i = 0; i < __symtable_size; i++) {
		if(__symtable[i].address > address) {
			continue;
		}
		uintptr_t distance = address - __symtable[i].address;
		if(distance < nearest_distance) {
			nearest = &__symtable[i];
			nearest_distance = distance;
			if(!nearest_distance) {
				break;
			}
		}
	}
	return nearest;
}

EXPORT(k_lookup_symbol)

EXPORT_INTERNAL(k_setup_symbols)
EXPORT_INTERNAL(k_find_nearest_symbol)

