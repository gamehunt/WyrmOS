#ifndef __K_SYMBOLS_H
#define __K_SYMBOLS_H 1

#include <stdint.h>

struct _symbol{
	const char* name;
	uintptr_t   address;
	uint64_t    internal;
	uint64_t    pad;
} __attribute__((packed));

typedef struct _symbol symbol;

#define EXPORT(func) __attribute__((used, section(".kernel_exported"))) \
	static const symbol __export_##func = {.name = #func, .address = (uintptr_t) &func, .internal = 0, .pad = 0};
#define EXPORT_ADDR(func, addr) __attribute__((used, section(".kernel_exported"))) \
	static const symbol __export_##func = {.name = #func, .address = addr, .internal = 0, .pad = 0};

#ifndef NO_INTERNAL_EXPORTS
#define EXPORT_INTERNAL(func, addr) __attribute__((used, section(".kernel_exported"))) \
	static const symbol __export_##func = {.name = #func, .address = addr, .internal = 1, .pad = 0};
#else
#define EXPORT_INTERNAL(func)
#endif

#ifndef __KERNEL
#define INTERNAL __attribute__((unavailable("This symbol is internal and not exported")))
#else
#define INTERNAL
#endif

INTERNAL void    k_setup_symbols();
INTERNAL symbol* k_find_nearest_symbol(uintptr_t address);
symbol*          k_lookup_symbol(const char* name);

#endif
