extern gdtr
extern load_gdt

extern reload_seg_registers

[BITS 64]
[SECTION .text]

load_gdt:
    cli
    lgdt [gdtr]
    ret

reload_seg_registers:
    push 0x08
    lea rax, [rel .reload_ret_to]
    push rax
    retfq
.reload_ret_to:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret