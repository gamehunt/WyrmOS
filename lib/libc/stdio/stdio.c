#include <stdio.h>
#include <stdlib.h>
#include <wyrm/syscall.h>

static FILE _stdin = {
    .fd     = 0,
    .offset = 0,
    .rbuf = NULL,
    .wbuf = NULL,
    .bufsz = BUFSIZ,
};

static FILE _stdout = {
    .fd     = 1,
    .offset = 0,
    .rbuf = NULL,
    .wbuf = NULL,
    .bufsz = BUFSIZ,
};

static FILE _stderr = {
    .fd = 2,
    .offset = 0,
    .rbuf = NULL,
    .wbuf = NULL,
    .bufsz = 0,
};

FILE* stdout = &_stdout;
FILE* stdin  = &_stdin;
FILE* stderr = &_stderr;

void __init_stdio() {
    _stdout.wbuf = malloc(BUFSIZ); 
    _stdin.rbuf  = malloc(BUFSIZ); 
}

int fflush(FILE *stream) {
    return 0;
}

FILE* fopen(const char* path, const char* access) {
    int flags = 0;
    int fd = __sys_open(path, flags);
    FILE* f = calloc(1, sizeof(FILE));
    f->fd = fd;
    return f;
}

int fclose(FILE* fp) {
    int fd = fp->fd;
    free(fp);
    return __sys_close(fd);
}

int fseek(FILE *stream, long offset, int origin) {
    return 0;
}

long ftell(FILE* stream) {
    return stream->offset;
}

int fprintf(FILE* stream, const char * format, ...) {
    va_list argp;
    va_start(argp, format);
    int r = vfprintf(stream, format, argp);
    va_end(argp);
    return r;
}

size_t fread(void* b, size_t s, size_t c, FILE* f) {
    return __sys_read(f->fd, f->offset, c * s, (uintptr_t) b);
}

size_t fwrite(const void* b, size_t s, size_t c, FILE* f) {
    return __sys_write(f->fd, f->offset, c * s, (uintptr_t) b);
}

void setbuf(FILE* f, char* b) {
    if(!b) {
        if(f->rbuf) {
            free(f->rbuf);
        }
        if(f->wbuf) {
            free(f->wbuf);
        }
        f->bufsz = 0;
    } else {
        f->bufsz = BUFSIZ;
    }
    f->rbuf  = b; 
    f->wbuf  = b;
}

int feof(FILE* stream) {
    return stream->eof;
}
