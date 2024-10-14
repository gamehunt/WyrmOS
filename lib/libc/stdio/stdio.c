#include "fcntl.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wyrm/syscall.h>

static FILE _stdin = {
    .fd     = 0,
    .roffs  = 0,
    .woffs  = 0,
    .rbuf = NULL,
    .wbuf = NULL,
    .bufsz = BUFSIZ,
};

static FILE _stdout = {
    .fd     = 1,
    .roffs  = 0,
    .woffs  = 0,
    .rbuf = NULL,
    .wbuf = NULL,
    .bufsz = BUFSIZ,
};

static FILE _stderr = {
    .fd = 2,
    .roffs  = 0,
    .woffs  = 0,
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

int fflush(FILE* stream) {
    if(!stream->wbuf) {
        return 0;
    }
    if(stream->woffs) {
        uint32_t result = write(stream->fd, stream->wbuf, stream->woffs);
        stream->woffs = 0;
        return result;
    }
    return 0;
}

static int __flags(const char* access) {
    int len = strlen(access);
    if(len == 0) {
        return -1;
    } else {
        int fl = 0;
        int ex = 0;

        if((len >= 2 && access[1] == '+') || (len == 3 && access[2] == '+')) {
            ex = 1;
        }

        switch(access[0]) {
            case 'r':
                if(!ex) {
                    fl = O_RDONLY;
                } else {
                    fl = O_RDWR;
                }
                break;
            case 'w':
                if(!ex) {
                    fl = O_WRONLY;
                } else {
                    fl = O_RDWR | O_CREAT | O_TRUNC;
                }
                break;
            case 'a':
                fl = O_WRONLY | O_CREAT | O_APPEND;
                break;
        }

        return fl;
    }
}

FILE* fdopen(int fildes, const char *mode) {
  FILE* file   = calloc(1, sizeof(FILE));
  file->fd     = fildes;
  file->wbuf   = malloc(BUFSIZ);
  file->rbuf   = malloc(BUFSIZ);
  file->bufsz  = BUFSIZ;
  return file;
}

FILE* fopen(const char* path, const char* access) {
    int flags = __flags(access);
    if(flags == -1) {
        return NULL;
    }
    int fd = open(path, flags);
    return fdopen(fd, access);
}

int fclose(FILE* fp) {
    fflush(fp);
    int fd = fp->fd;
    if(fp->rbuf) {
        free(fp->rbuf);
    }
    if(fp->wbuf) {
        free(fp->wbuf);
    }
    free(fp);
    return close(fd);
}

int fseek(FILE *stream, long offset, int origin) {
    if(stream->roffs && origin == SEEK_CUR) {
        offset += stream->rloffs + stream->roffs;
        origin = SEEK_SET;  
    }

    if(stream->woffs){
        fflush(stream);
    }

    stream->roffs = 0;
    stream->woffs = 0;
    stream->available = 0;
    stream->eof       = 0;

    return __sys_seek(stream->fd, offset, origin);
}

long ftell(FILE* stream) {
    return fseek(stream, 0, SEEK_CUR);
}

int fprintf(FILE* stream, const char * format, ...) {
    va_list argp;
    va_start(argp, format);
    int r = vfprintf(stream, format, argp);
    va_end(argp);
    return r;
}

size_t fread(void* b, size_t s, size_t c, FILE* f) {
    size_t len = s * c;
    size_t read_bytes = 0;
    char* buf = (char*) b;
    if(!f->rbuf) {
        return read(f->fd, buf, len);
    }
    while(len > 0) {
        if(!f->available) {
            if(f->rbufoffs == f->bufsz) {
                f->rbufoffs = 0;
            }

            f->rloffs  = fseek(f, 0, SEEK_CUR);
            int r = read(f->fd, &f->rbuf[f->rbufoffs], f->bufsz - f->rbufoffs);

            if(r < 0){
                break;
            } else {
                f->roffs     = f->rbufoffs;
                f->rbufoffs += r;
                f->available = r;
            }
        }

        if(f->available) {
            size_t to_copy = (len > f->available ? f->available : len);
            memcpy(buf, f->rbuf + f->roffs, to_copy);
            f->roffs     += to_copy;
            f->available -= to_copy;
            buf += to_copy;
            len -= to_copy;
            read_bytes += to_copy;
        } else {
            f->eof = 1;
            break;
        }
    }
    return read_bytes;
}

size_t fwrite(const void* b, size_t s, size_t c, FILE* f) {
    uint32_t len = s * c;
    uint32_t written = len;
    char* buf = (char*) b;
    if(!f->wbuf) {
        return write(f->fd, buf, len);
    } else {
        while(len > 0) {
            f->wbuf[f->woffs++] = *buf;
            if(f->woffs == f->bufsz || (*buf == '\n')) {
                fflush(f);
            }
            buf++;
            len--;
        }
    }
    return written; 
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

int fileno(FILE* stream) {
    return stream->fd;
}

void clearerr(FILE *stream) {
    stream->eof = 0;
}

char fgetc(FILE* s) {
    char r;
    size_t read = fread(&r, 1, 1, s);
    if(read == 0) {
        s->eof = 1;
        return EOF;
    }
    return r;
}

int fputc(int ch, FILE *stream) {
    size_t written = fwrite(&ch, 1, 1, stream);
    if(written == 0) {
        stream->eof = 1;
        return EOF;
    }
    return ch;
}

char*  fgets(char* s, int size, FILE* stream) {
    if(size <= 1) {
        return NULL;
    }

    char* ptr = s;
    char  c;

    size -= 1;

    while((c = fgetc(stream)) != EOF) {
        *s = c;
        s++;
        size--;
        if(!size || c == '\n') {
            *s = '\0';
            return ptr;
        }
    }

    if(c == EOF) {
        if(ptr == s) {
            return NULL;
        } else {
            return ptr;
        }
    }

    return NULL;
}

void rewind(FILE* stream) {
    fseek(stream, 0, SEEK_SET);
    stream->rbufoffs = 0;
    stream->available = 0;
    stream->eof = 0;
    stream->ungetc = 0;
}
