#include "arch.h"
#include "util.h"

int arch_enter_signal(uintptr_t entry, int sig, regs* r) {
    uintptr_t rsp = ALIGN(r->rsp, 16);
    PUSH(rsp, regs, *r);
    PUSH(rsp, long, sig);
    arch_user_jmp(entry, rsp);
    return -1;
}

int arch_exit_signal(regs* r) {
    long sig   = POP(r->rsp, long);
    regs saved = POP(r->rsp, regs);
    *r = saved;
    return sig;
}
