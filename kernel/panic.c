#include "panic.h"
#include "asm.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define PANIC_WIDTH 20

__attribute__((noreturn)) void panic(const char* message, ...){
	printf("Kernel panic!\r\n");

	va_list ap;
	va_start(ap, message);
	vprintf(message, ap);

	hcf();
}
