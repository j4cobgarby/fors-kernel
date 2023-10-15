extern interrupt_dispatch

isr_general:
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi
    push    rbp
    push    r8
    push    r9
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15

    mov     rdi, rsp    ; RDI is the first argument to the C function interrupt_dispatch
                        ; which expects a `register_ctx_x64 *` type. That structure is
                        ; defined so that it matches the stack at this point in execution,
                        ; so the stack pointer can be interpreted as this type.
    call    interrupt_dispatch
    mov     rsp, rax    ; Set registers to whatever interrupt_dispatch returns (again as
                        ; a `register_ctx_x64 *` type.

    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rbp
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax

    add     rsp, 16     ; At this point the interrupt vector number and error code are still
                        ; on the stack, but iret doesn't know or care about these. This line
                        ; just takes those two things away so that iret sees what the CPU
                        ; originally pushed.

    xchg bx, bx

    iretq

%macro  isr_errorcode 1
extern __isr_%1
__isr_%1:
    ; (When the particular interrupt type has an error code, this is pushed by the CPU before entering here.)
    push qword %1       ; Just push the vector number
    jmp isr_general     ; isr_general does most of the work getting to the C code
%endmacro

%macro isr_noerror 1
extern __isr_%1
__isr_%1:
    push qword 0        ; Fake error code, to account for the CPU not pushing one
    push qword %1
    jmp isr_general
%endmacro

; For some interrupt vectors, the CPU pushes an error code onto the stack after
; the standard registers (SS, RSP, RFLAGS, CS, RIP). Examples of those that do
; include #GP and #PF.
; For those that do _not_, we actually push a fake error code of 0 to the stack
; so that as far as the isr_general procedure is concerned all vectors work in
; the same way.

isr_noerror 0
isr_noerror 1
isr_noerror 2
isr_noerror 3
isr_noerror 4
isr_noerror 5
isr_noerror 6
isr_noerror 7
isr_noerror 16
isr_noerror 18
isr_noerror 19

isr_errorcode 8
isr_errorcode 10
isr_errorcode 11
isr_errorcode 12
isr_errorcode 13
isr_errorcode 14
isr_errorcode 17

; PIC mapping starts at 32
isr_noerror 33 ; keyboard
