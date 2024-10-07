global __setup_flags
__setup_flags:
    mov rax, cr0
    xor rax, (1 << 16)
    mov cr0, rax
    ret

global __get_pml
__get_pml:
	mov rax, cr3
	or  rax, rdi
	ret

global __set_pml
__set_pml:
	mov cr3, rdi
	ret

global __get_pagefault_address
__get_pagefault_address:
	mov rax, cr2
	ret
