#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

int abs(int n) {
    return n >= 0 ? n : -n;
}

int atoi(const char* str) {
	const char* end = str;	

	uint8_t neg = 0;

	if(*end == '-') {
		neg = 1;	
		end++;
	}

	while(*end != '\0' && isdigit(*end)) {
		end++;
	}

	end -= 1;

	int r = 0;
	int e = 1;
	while(end != str - 1) {
		r += (*end - '0') * e;
		e *= 10;
		end--;
	}

	if(neg) {
		r = -r;
	}

	return r;
}

void abort(void) {
#ifdef __LIBK
#include "panic.h"
	panic(NULL, "abort() called.");
#endif
}

#ifndef __LIBK
#define ATEXIT_MAX 32

static int __atexit_registered = 0;
static void(*__atexit_handlers)(void)[ATEXIT_MAX] = {0};

int atexit(void (*c)(void)) {
    if(__atexit_registered >= ATEXIT_MAX) {
        return 1;
    }
    __atexit_handlers[__atexit_registered] = c;
    __atexit_registered++;
    return 0;
}

void __attribute__((noreturn)) exit(int exit_code) {
    for(int i = 0; i < __atexit_registered; i++) {
        __atexit_handlers[i]();
    }
    __sys_exit(code);
}

char* getenv(const char* env) {
    return "";
}
#endif
