#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

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
