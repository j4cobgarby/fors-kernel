#include "arch/x64/idt.h"
#include "arch/x64/memory.h"
#include "arch/x64/pic.h"
#include "arch/x64/io.h"
#include "arch/x64/cpu.h"
#include "arch/x64/pit.h"

#include "fors/memory.h"
#include "fors/panic.h"
#include "fors/printk.h"
#include "fors/process.h"
#include "fors/timer.h"
#include "fors/syscall.h"

#include "forslib/string.h"

#include <stdint.h>

struct idt_entry idt_table[IDT_N_ENTRIES];
tss_t tss;

// ISRs without CPU-pushed error codes
extern void *__isr_0x00;
extern void *__isr_0x01;
extern void *__isr_0x02;
extern void *__isr_0x03;
extern void *__isr_0x04;
extern void *__isr_0x05;
extern void *__isr_0x06;
extern void *__isr_0x07;
extern void *__isr_0x10;
extern void *__isr_0x12;
extern void *__isr_0x13;

extern void *__isr_0x20;
extern void *__isr_0x21; // Keyboard

extern void *__isr_0xf0; // 0xf0 syscall

// ISRs _with_ CPU-pushed error codes
extern void *__isr_0x08;
extern void *__isr_0x0a;
extern void *__isr_0x0b;
extern void *__isr_0x0c;
extern void *__isr_0x0d;
extern void *__isr_0x0e;
extern void *__isr_0x11;

void idt_init()
{
    idt_load(idt_table, IDT_N_ENTRIES);

    union segment_selector isr_seg;
    isr_seg.element.table = TABLE_GDT;
    isr_seg.element.rpl = 0;
    isr_seg.element.entry = 1;

    idt_attributes_t isr_attr
        = INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0);

    idt_attach_handler(0x00, isr_seg, isr_attr, &__isr_0x00);
    idt_attach_handler(0x01, isr_seg, isr_attr, &__isr_0x01);
    idt_attach_handler(0x02, isr_seg, isr_attr, &__isr_0x02);
    idt_attach_handler(0x03, isr_seg, isr_attr, &__isr_0x03);
    idt_attach_handler(0x04, isr_seg, isr_attr, &__isr_0x04);
    idt_attach_handler(0x05, isr_seg, isr_attr, &__isr_0x05);
    idt_attach_handler(0x06, isr_seg, isr_attr, &__isr_0x06);
    idt_attach_handler(0x07, isr_seg, isr_attr, &__isr_0x07);
    idt_attach_handler(0x10, isr_seg, isr_attr, &__isr_0x10);
    idt_attach_handler(0x12, isr_seg, isr_attr, &__isr_0x12);
    idt_attach_handler(0x13, isr_seg, isr_attr, &__isr_0x13);

    idt_attach_handler(0x20, isr_seg, isr_attr, &__isr_0x20); // Timer
    idt_attach_handler(0x21, isr_seg, isr_attr, &__isr_0x21); // Keyboard

    idt_attach_handler(0xf0, isr_seg,
        INIT_IDT_ATTRIBUTES(3, IDT_ATTRIBUTES_TYPE_TRAP, 0), &__isr_0xf0);

    idt_attach_handler(0x08, isr_seg, isr_attr, &__isr_0x08);
    idt_attach_handler(0x0a, isr_seg, isr_attr, &__isr_0x0a);
    idt_attach_handler(0x0b, isr_seg, isr_attr, &__isr_0x0b);
    idt_attach_handler(0x0c, isr_seg, isr_attr, &__isr_0x0c);
    idt_attach_handler(0x0d, isr_seg, isr_attr, &__isr_0x0d);
    idt_attach_handler(0x0e, isr_seg, isr_attr, &__isr_0x0e);
    idt_attach_handler(0x11, isr_seg, isr_attr, &__isr_0x11);

    memset(&tss, 0, sizeof(tss_t));
    tss.rsp0 = (uint64_t)allocate_stack();
}

void idt_load(struct idt_entry *table, int n_entries)
{
    struct idt_descriptor idt_desc = INIT_IDT_DESCRIPTOR(table, n_entries);

    __asm__("lidt %0" : : "m"(idt_desc));
}

void idt_attach_handler(int vector, union segment_selector seg,
    idt_attributes_t attr, void *handler)
{
    idt_table[vector] = INIT_IDT_ENTRY(seg, attr, (uint64_t)handler);
}

void *interrupt_dispatch(register_ctx_x64 *ctx)
{
    int sc, sysret;
    long proc_save;
    switch (ctx->vector) {
    case INT_PF:
        KPANIC_VA("Page Fault (%d)", ctx->error_code);
        /* TODO:
        1) Check if the page trying to be accessed is currently mapped
           within the kernel PML4.
        2) If so, make the same mapping in the
           current PML4, because it must be an interrupt which happened during
           usermode, that is trying to run kernel interrupt handling code, or
           trying to access the kernel heap, or something like that.
        */
    case INT_GP:
        KPANIC_VA("General Protection Fault (%d)", ctx->error_code);
    case INT_DE:
        KPANIC("Division by zero encounters!");

    case PIC_FIRST_VECTOR: // PIT timer
        proc_save = current_proc;

        // Tick the timer, which may call some timer handle callbacks.
        // These callbacks may do scheduling work, so after timer_tick
        // the current_proc may have changed.
        timer_tick();

        if (proc_save != current_proc) { // current_proc has been changed!
            if (proc_save >= 0 && ctx->rip < 0xffff800000000000) {
                // Save context if a process is running
                // If the check for rip being in user memory is not made, the
                // situation can happen where a timer interrupt occurs at the
                // moment when a syscall is sent by a user process, which messes
                // up the process' execution.
                procs[proc_save].ctx = *ctx;
            }
            /* After this interrupt finishes, the system registers are all set
             * according to ctx, so by this mechanism we switch to a different
             * task */
            ctx = &procs[current_proc].ctx;
        }

        pic_eoi(0);
        break;
    case PIC_FIRST_VECTOR + 1:
        sc = inb(0x60);

        if (!(sc & 0x80)) {
            printk("Keystroke (%#x)\n", sc);
        }
        /* TODO: Buffer keypresses to some circular queue */

        pic_eoi(1);

        break;

    case 0xf0:
        sysret = syscall_dispatch(ctx->rax, ctx->rsi, ctx->rbx, ctx->rcx);
        // ctx->rax = sysret;
        if (ctx->rax == 0) {
            printk(") (Task %d dbg) %s", current_proc, (const char *)ctx->rsi);
        } else {
            printk("Syscall made by task %d\n", current_proc);
        }
        break;
    default:
        printk("Unhandled interrupt <%#x>\n", ctx->vector);
        for (;;) __asm__("hlt");
    }

    return ctx;
}
