#include "arch/x64/idt.h"
#include "arch/x64/memory.h"
#include "fors/printk.h"

#include <stdint.h>

struct idt_entry idt_table[IDT_N_ENTRIES];

// ISRs without CPU-pushed error codes
extern void *__isr_0;
extern void *__isr_1;
extern void *__isr_2;
extern void *__isr_3;
extern void *__isr_4;
extern void *__isr_5;
extern void *__isr_6;
extern void *__isr_7;
extern void *__isr_16;
extern void *__isr_18;
extern void *__isr_19;

// ISRs _with_ CPU-pushed error codes
extern void *__isr_8;
extern void *__isr_10;
extern void *__isr_11;
extern void *__isr_12;
extern void *__isr_13;
extern void *__isr_14;
extern void *__isr_17;

void idt_init() {
    idt_load(idt_table, IDT_N_ENTRIES);

    union segment_selector isr_seg;
    isr_seg.element.table = TABLE_GDT;
    isr_seg.element.rpl = 0;
    isr_seg.element.entry = 1;

    idt_attributes_t isr_attr = INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0);

    idt_attach_handler(0, isr_seg, isr_attr, &__isr_0);
    idt_attach_handler(1, isr_seg, isr_attr, &__isr_1);
    idt_attach_handler(2, isr_seg, isr_attr, &__isr_2);
    idt_attach_handler(3, isr_seg, isr_attr, &__isr_3);
    idt_attach_handler(4, isr_seg, isr_attr, &__isr_4);
    idt_attach_handler(5, isr_seg, isr_attr, &__isr_5);
    idt_attach_handler(6, isr_seg, isr_attr, &__isr_6);
    idt_attach_handler(7, isr_seg, isr_attr, &__isr_7);
    idt_attach_handler(16, isr_seg, isr_attr, &__isr_16);
    idt_attach_handler(18, isr_seg, isr_attr, &__isr_18);
    idt_attach_handler(19, isr_seg, isr_attr, &__isr_19);
    idt_attach_handler(8, isr_seg, isr_attr, &__isr_8);
    idt_attach_handler(10, isr_seg, isr_attr, &__isr_10);
    idt_attach_handler(11, isr_seg, isr_attr, &__isr_11);
    idt_attach_handler(12, isr_seg, isr_attr, &__isr_12);
    idt_attach_handler(13, isr_seg, isr_attr, &__isr_13);
    idt_attach_handler(14, isr_seg, isr_attr, &__isr_14);
    idt_attach_handler(17, isr_seg, isr_attr, &__isr_17);
}

void idt_load(struct idt_entry* table, int n_entries) {
    struct idt_descriptor idt_desc = INIT_IDT_DESCRIPTOR(table, n_entries);

    __asm__ ("lidt %0"
        : /* No outputs */
        : "m" (idt_desc)); /* Pointer to the descriptor as %0 */
}

void idt_attach_handler(int vector, union segment_selector seg, idt_attributes_t attr, void *handler) {
    idt_table[vector] = INIT_IDT_ENTRY(seg, attr, (uint64_t)handler);
}

void *interrupt_dispatch(interrupt_context_t *ctx) {
    printk("Handling interrupt vector %d.\n", ctx->vector);
    
    switch (ctx->vector) {
        case INT_PF:
            printk("Page fault!\n");
            break;
        case INT_DE:
            printk("Division by zero.\n");
            for (;;) __asm__("hlt");
            break;
    }

    return ctx;
}