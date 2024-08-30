#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((noreturn)) void __assert_fail(const char* file, int line, const char* expr) {
	printf("Assertion failed: in %s:%d: %s\r\n", file, line, expr);
	abort();
}
