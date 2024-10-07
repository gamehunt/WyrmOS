#include <stddef.h>
#include <stdlib.h>

extern void __init_stdio();

void __libc_init(int argc, const char** argv, char** envp, int(*main)(int, const char**)) {
    __init_stdio();
    exit(main(argc, argv));
}
