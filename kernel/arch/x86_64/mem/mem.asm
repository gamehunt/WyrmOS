global load_descriptor_table
load_descriptor_table:
	lgdt [rdi]
	mov ax, 0x28 ; Load TSS
	ltr ax
	ret

global reload_segments
reload_segments:
	push 0x08
	lea rax, [rel .reload_cs]
	push rax
	retfq

.reload_cs:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret

global arch_get_stack
arch_get_stack:
	mov rax, rsp
	ret
