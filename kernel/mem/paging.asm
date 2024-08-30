global __get_pml
__get_pml:
	mov rax, cr3
	or  rax, rdi
	ret
