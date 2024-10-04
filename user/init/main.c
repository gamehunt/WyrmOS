#include "unistd.h"

extern int __invoke_syscall(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f, int number);
int main() {
    while(1) { 
        __invoke_syscall(0, 0, 0, 0, 0, 0, 0xFF);
        sleep(1);
    }
    return 0;
}
