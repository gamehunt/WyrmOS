#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

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