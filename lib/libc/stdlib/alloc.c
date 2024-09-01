#include <stdlib.h>
#include <string.h>

#ifdef __LIBK
#include <mem/alloc.h>
#endif

void* __attribute__ ((malloc)) malloc(size_t size) {
#ifdef __LIBK
	return kmalloc(size);
#else
	return NULL;
#endif
}

void* __attribute__ ((malloc)) realloc(void* old, size_t new) {
	void* r = malloc(new);
	memmove(r, old, new);
	free(old);
	return r;
}


void free(void* ptr) {
#ifdef __LIBK
	kfree(ptr);
#endif
}
