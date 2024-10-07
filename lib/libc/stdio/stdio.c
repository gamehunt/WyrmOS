#include <stdio.h>

FILE* _stdout = 0;
FILE* _stdin  = 0;
FILE* _stderr = 0;

int fflush(FILE *stream) {
    return 0;
}

FILE* fopen(const char*, const char*) {
    return NULL;
}

int fclose(FILE *fp) {
    return 0;
}

int fseek(FILE *stream, long offset, int origin) {
    return 0;
}

long ftell(FILE *stream) {
    return 0;
}

int vfprintf(FILE * stream, const char * format, va_list args) {
    return 0;
}

int fprintf(FILE * stream, const char * format, ...) {
    va_list argp;
    va_start(argp, format);
    int r = vfprintf(stream, format, argp);
    va_end(argp);
    return r;
}

size_t fread(void* b, size_t o, size_t s, FILE* f) {
    return 0;
}

size_t fwrite(const void* b, size_t o, size_t s, FILE* f) {
    return 0;
}

void setbuf(FILE* f, char* b) {
    
}

int feof(FILE *stream) {
    return 0;
}
