.extern interrupt_dispatch

isr_general:
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rax, %cr3
    pushq %rax         ; push cr3

    movq %rdi, %rsp    ; RDI is the first argument to the C function interrupt_dispatch
                        ; which expects a `register_ctx_x64 *` type. That structure is
                        ; defined so that it matches the stack at this point in execution,
                        ; so the stack pointer can be interpreted as this type.
    call interrupt_dispatch
    movq rsp, rax    ; Set registers to whatever interrupt_dispatch returns (again as
                        ; a `register_ctx_x64 *` type.

    popq %rax
    movq %cr3, %rax

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    addq %rsp, $16     ; At this point the interrupt vector number and error code are still
                        ; on the stack, but iret doesn't know or care about these. This line
                        ; just takes those two things away so that iret sees what the CPU
                        ; originally pushq %ed.

    iretq
