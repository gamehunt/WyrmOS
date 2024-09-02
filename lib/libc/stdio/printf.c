#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef __LIBK
#include <globals.h>
#include <dev/log.h>
#endif

int putchar(int ch) {
#ifdef __LIBK
	k_print("%c", ch);
	return ch;
#endif
	return EOF;
}

int puts(const char *s) {
	int i = 0;
	while(s[i]) {
		if(putchar(s[i]) == EOF) {
			return EOF;
		}
		i++;
	}
	if(putchar('\n') == EOF) {
		return EOF;
	}
	return i + 1;
}

int printf(const char* format, ...) {
	va_list args;

	va_start(args, format);
	int r = vprintf(format, args);
	va_end(args);

	return r;
}

int sprintf(char* buf, const char* format, ...) {
	va_list args;

	va_start(args, format);
	int r = vsprintf(buf, format, args);
	va_end(args);

	return r;
}

int snprintf(char* buf, size_t size, const char* format, ...) {
	va_list args;

	va_start(args, format);
	int r = vsnprintf(buf, size, format, args);
	va_end(args);

	return r;
}

#define PRINTF_AWAIT_FLAG  (1 << 0)
#define PRINTF_AWAIT_WIDTH (1 << 1)
#define PRINTF_AWAIT_PREC  (1 << 2)
#define PRINTF_AWAIT_LEN   (1 << 3)
#define PRINTF_AWAIT_SPEC  (1 << 4)

#define PARAM_FL_PAD_RIGHT  (1 << 0)
#define PARAM_FL_ALWAYS_SGN (1 << 1)
#define PARAM_FL_INS_BLANK  (1 << 2)
#define PARAM_FL_SHARP      (1 << 3)
#define PARAM_FL_PAD_ZERO   (1 << 4)

#define PAR_LEN_NONE   0
#define PAR_LEN_L      1
#define PAR_LEN_LL     2
#define PAR_LEN_H      3
#define PAR_LEN_HH     4
#define PAR_LEN_SIGNED (1 << 7)

typedef int(*__char_emitter)(int, void*);

typedef struct {
	int flags, width, precision, length;
} __printf_params;

static void __printf_clear_params(__printf_params* p) {
	p->flags     = 0;
	p->width     = 0;
	p->precision = 0;
	p->length    = PAR_LEN_SIGNED;
}

static int __emit_str(const char* str, __printf_params* par, uint8_t is_raw,  __char_emitter emitter, void* data) {
	if(!(par->flags & PARAM_FL_PAD_RIGHT)) {
		int l = strlen(str);
		while(l < par->width) {
			if(!is_raw && par->flags & PARAM_FL_PAD_ZERO) {
				emitter('0', data);
			} else {
				emitter(' ', data);
			}
			l++;
		}
	}

	int i = 0;
	while(str[i]) {
		emitter(str[i], data);
		i++;
		if(is_raw && par->precision && i > par->precision) {
			return i;
		}
	}

	if(par->flags & PARAM_FL_PAD_RIGHT) {
		while(i < par->width) {
			if(par->flags & PARAM_FL_PAD_ZERO) {
				emitter('0', data);
			} else {
				emitter(' ', data);
			}
			i++;
		}
	}

	return i;
}

#define def_format_length(c, type, signed) \
static int __put_number_##c(type number, unsigned int base, uint8_t uppercase, __printf_params* par, __char_emitter emitter, void* data) { \
	char stack[64];	\
	stack[63] = '\0'; \
	char* esp = &stack[62]; \
	int w = 0; \
	int s = 0; \
	if(signed && number < 0) { \
		number = -number; \
		s = 1; \
	} \
	if(number == 0) { \
		*esp = '0'; \
		esp--; \
		w++; \
	} \
	while(number) { \
		int remainder = number % base; \
		if(remainder >= 10) { \
			remainder = 'a' + remainder - 10; \
			if(uppercase) { \
				remainder = toupper(remainder); \
			} \
		} else { \
			remainder += '0'; \
		} \
		*esp = remainder; \
		number /= base; \
		esp--; \
		w++; \
	} \
	while(w < par->precision && esp >= &stack[2]) { \
		*esp = '0'; \
		esp--; \
		w++; \
	} \
	if(par->flags & PARAM_FL_SHARP) { \
		if(base == 16) { \
			if(uppercase) { \
				*esp = 'X'; \
			} else { \
				*esp = 'x'; \
			} \
			esp--; \
			*esp = '0'; \
			esp--; \
		} else if(base == 8) { \
			*esp = '0'; \
			esp--; \
		} \
	} \
	if(signed) { \
		if(s) { \
			*esp = '-'; \
			esp--; \
		} else if(par->flags & PARAM_FL_ALWAYS_SGN) { \
			*esp = '+'; \
			esp--; \
		} else if(par->flags & PARAM_FL_INS_BLANK) { \
			*esp = ' '; \
			esp--; \
		} \
	} \
	return __emit_str(esp + 1, par, 0, emitter, data); \
} 

#define __put_number_with_length_internal(l, n, base, up, param, emitter, data) __put_number_##l(n, base, up, param, emitter, data)

def_format_length(signed,      int, 1)
def_format_length(signed_h,    short int, 1)
def_format_length(signed_hh,   signed char, 1)
def_format_length(signed_l,    long, 1)
def_format_length(signed_ll,   long long, 1)
def_format_length(unsigned,    unsigned int, 0)
def_format_length(unsigned_h,  unsigned short int, 0)
def_format_length(unsigned_hh, unsigned char, 0)
def_format_length(unsigned_l,  unsigned long, 0)
def_format_length(unsigned_ll, unsigned long long, 0)

int __put_number(va_list args, unsigned int base, uint8_t uppercase, __printf_params* par, __char_emitter emitter, void* data) {
	int l = par->length;
	switch(l) {
		default:
		case 0:
			return __put_number_unsigned(va_arg(args, unsigned int), base, uppercase,par, emitter, data);
		case PAR_LEN_L:
			return __put_number_unsigned_l(va_arg(args, unsigned long), base, uppercase,par, emitter, data);
		case PAR_LEN_LL:
			return __put_number_unsigned_ll(va_arg(args, unsigned long long), base, uppercase,par, emitter, data);
		case PAR_LEN_H:
			return __put_number_unsigned_h(va_arg(args, unsigned int), base, uppercase,par, emitter, data);
		case PAR_LEN_HH:
			return __put_number_unsigned_hh(va_arg(args, unsigned int), base, uppercase,par, emitter, data);
		case PAR_LEN_SIGNED:
			return __put_number_signed(va_arg(args, int), base, uppercase,par, emitter, data);
		case PAR_LEN_L | PAR_LEN_SIGNED:
			return __put_number_signed_l(va_arg(args, long), base, uppercase,par, emitter, data);
		case PAR_LEN_LL | PAR_LEN_SIGNED:
			return __put_number_signed_ll(va_arg(args, long long), base, uppercase,par, emitter, data);
		case PAR_LEN_H | PAR_LEN_SIGNED:
			return __put_number_signed_h(va_arg(args, int), base, uppercase,par, emitter, data);
		case PAR_LEN_HH | PAR_LEN_SIGNED:
			return __put_number_signed_hh(va_arg(args,int), base, uppercase,par, emitter, data);
	}
}
	

#ifndef __LIBK
#define __PRINTF_DBL_UPPERCASE  (1 << 0)
#define __PRINTF_DBL_SCIENTIFIC (2 << 0)
static int __put_number_float(double number, unsigned int base, uint8_t flags, __char_emitter emitter, void* data) {
	//TODO
	return 0;
}
#endif

static int __vprintf_generic(const char* format, va_list arg_ptr, __char_emitter emitter, void* data) {
	uint8_t flags = 0;
	int written   = 0;
	__printf_params params;
	__printf_clear_params(&params);
	while(*format) {
		char c = *format;

		if(c == '%') {
			if(flags & PRINTF_AWAIT_SPEC) {
				written += emitter(c, data);
				flags = 0;
				__printf_clear_params(&params);
			} else if(!flags) {
				flags |= PRINTF_AWAIT_FLAG;
			} else {
				flags <<= 1;
				continue;
			}
			goto next;
		} else if(!flags) {
			written += emitter(c, data);
			goto next;
		}

		if(flags & PRINTF_AWAIT_FLAG) {
			switch(c) {
				case '-':
					params.flags |= PARAM_FL_PAD_RIGHT;
					break;
				case '+':
					params.flags |= PARAM_FL_ALWAYS_SGN;
					break;
				case '#':
					params.flags |= PARAM_FL_SHARP;
					break;
				case ' ':
					params.flags |= PARAM_FL_INS_BLANK;
					break;
				case '0':
					params.flags |= PARAM_FL_PAD_ZERO;
					break;
				default:
					flags <<= 1;
					continue;
			}
			flags <<= 1;
			goto next;
		}

		if(flags & PRINTF_AWAIT_WIDTH) {
			if(c == '*') {
				params.width = va_arg(arg_ptr, int);
			} else if(isdigit(c)) {
				params.width = atoi(format);
				while(isdigit(*(format + 1)) && *(format + 1) != '\0') {
					format++;
				}
			} else {
				flags <<= 1;
				continue;
			}
			flags <<= 1;
			goto next;
		}

		if(flags & PRINTF_AWAIT_PREC) {
			if(c != '.') {
				flags <<= 1;
				continue;
			}

			format += 1;

			c = *format;

			if(c == '\0') {
				break;
			} else if(c == '*') {
				params.precision = va_arg(arg_ptr, int);
			} else if(isdigit(c)) {
				params.precision = atoi(format);	
				while(isdigit(*(format + 1)) && *(format + 1) != '\0') {
					format++;
				}
			} else {
				params.precision = 0;
			}
			
			flags <<= 1;
			goto next;
		}

		if(flags & PRINTF_AWAIT_LEN) {
			switch(c) {
				case 'l':
					params.length = PAR_LEN_L;
					if(*(format + 1) == 'l') {
						params.length = PAR_LEN_LL;
						format++;
					}
					break;
				case 'h':
					params.length = PAR_LEN_H;
					if(*(format + 1) == 'h') {
						params.length = PAR_LEN_HH;
						format++;
					}
					break;
				default:
					flags <<= 1;
					continue;
			}
			flags <<= 1;
			goto next;
		}

		if(flags & PRINTF_AWAIT_SPEC) {
			switch(c) {
				case 'c':
					written += emitter(va_arg(arg_ptr, int), data);
					break;
				case 's':
					written += __emit_str(va_arg(arg_ptr, const char*),&params, 1, emitter, data);
					break;
				case 'd':
				case 'i':
					params.length |= PAR_LEN_SIGNED;
				case 'u':
					written += __put_number(arg_ptr, 10, 0, &params, emitter, data);
					break;
				case 'o':
					written += __put_number(arg_ptr, 8, 0, &params, emitter, data);
					break;
				case 'x':
					written += __put_number(arg_ptr, 16, 0, &params, emitter, data);
					break;
				case 'X':
					written += 	__put_number(arg_ptr, 16, 1, &params, emitter, data);
					break;
				case 'p':
					params.length = PAR_LEN_L;
					written += __put_number(arg_ptr, 16, 0, &params, emitter, data);
					break;
#ifndef __LIBK
				case 'f':
					written += __put_number_float(va_arg(arg_ptr, double), 10, 0, emitter, data);
					break;
				case 'F':
					written += __put_number_float(va_arg(arg_ptr, double), 10, __PRINTF_DBL_UPPERCASE, emitter, data);
					break;
				case 'a':
					written += __put_number_float(va_arg(arg_ptr, double), 16, 0, emitter, data);
					break;
				case 'A':
					written += __put_number_float(va_arg(arg_ptr, double), 16, __PRINTF_DBL_UPPERCASE, emitter, data);
					break;
				case 'e':
					written += __put_number_float(va_arg(arg_ptr, double), 10, __PRINTF_DBL_SCIENTIFIC, emitter, data);
					break;
				case 'E':
					written += __put_number_float(va_arg(arg_ptr, double), 10, __PRINTF_DBL_SCIENTIFIC | __PRINTF_DBL_UPPERCASE, emitter, data);
					break;
#endif
				case 'n':
					*va_arg(arg_ptr, int*) = written;
					break;
			}

			flags = 0;
			__printf_clear_params(&params);
		}

next:
		format++;
	} 

	return written;
}

static int __stdio_putchar(int c, void* unused) {
	(void) unused;
	return putchar(c);
}

int vprintf(const char* format, va_list arg_ptr) {
	return __vprintf_generic(format, arg_ptr, __stdio_putchar, NULL);
}

typedef struct {
	int   position;
	int   limit;
	char* buffer;
} positioned_buffer;

static int __buffer_putchar(int c, void* data) {
	positioned_buffer* buffer = data;

	if(buffer->position >= buffer->limit) {
		return EOF;
	}

	buffer->buffer[buffer->position] = c;
	buffer->position++;

	return 1;
}

int vsprintf(char* buf, const char *format, va_list arg_ptr) {
	positioned_buffer buf_wrapper = {0, 0, buf};
	return __vprintf_generic(format, arg_ptr, __buffer_putchar, &buf_wrapper);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	positioned_buffer buf_wrapper = {0, size, str};
	return __vprintf_generic(format, ap, __buffer_putchar, &buf_wrapper);
}
