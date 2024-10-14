#include "arch.h"
#include "proc/spinlock.h"
#include <panic.h>
#include <asm.h>
#include <dev/log.h>
#include <symbols.h>

#include <stdarg.h>
#include <stdio.h>

#define PANIC_WIDTH 20

extern const char* __log_prefixes[];
extern lock __log_global_lock;
__attribute__((noreturn)) void panic(regs* r, const char* message, ...){
    arch_prepare_panic();
    cli();
    UNLOCK(__log_global_lock);
    k_crit("Kernel panic!");
    k_print("%s", __log_prefixes[CRITICAL]);
    va_list ap;
    va_start(ap, message);
    vprintf(message, ap);
    k_print("\r\n");
    k_crit("Dump:");
    if(r) {
        arch_dump(r);
    } else {
        k_crit("Not available.");
    }
    k_crit("Stacktrace:");
    arch_stacktrace(r);
    hcf();
}
