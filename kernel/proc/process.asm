global __save_ctx
__save_ctx:
    mov [rdi + 0], esp ; Save ESP
    mov [rdi + 8], ebp ; Save EBP

    mov rax, [rsp]      ; Get EIP
    mov [rdi + 16], rax ; Save EIP

    xor rax, rax       ; return 0
	ret

global __load_ctx
__load_ctx:
    mov rsp, [rdi + 0] ; Restore ESP
    mov rbp, [rdi + 8] ; Restore EBP

    mov  rax, 1     ; Make save function return 1
    jmp [rdi + 16]  ; Jump
