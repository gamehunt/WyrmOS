global __invoke_syscall
__invoke_syscall:
    mov rax, [rsp + 0x8]
    int 0x80
    ret
