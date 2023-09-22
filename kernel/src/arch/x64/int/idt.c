#include "arch/x64/idt.h"
#include "arch/x64/memory.h"

#include <stdint.h>

struct idt_entry idt_table[IDT_N_ENTRIES];

void idt_init() {
    idt_load(idt_table, IDT_N_ENTRIES);

    union segment_selector isr_seg;
    isr_seg.element.table = TABLE_GDT;
    isr_seg.element.rpl = 0;
    isr_seg.element.entry = 1;

    idt_attach_handler(0x02, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_INTERRUPT, 0), &isr_NMI);
    idt_attach_handler(0x32, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_INTERRUPT, 0), &isr_SYSCALL);

    idt_attach_handler(0x00, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_DE);
    idt_attach_handler(0x03, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_BP);
    idt_attach_handler(0x04, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_OF);
    idt_attach_handler(0x05, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_BR);
    idt_attach_handler(0x06, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_UD);
    idt_attach_handler(0x08, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_DF);
    idt_attach_handler(0x0a, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_TS);
    idt_attach_handler(0x0b, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_NP);
    idt_attach_handler(0x0c, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_SS);
    idt_attach_handler(0x0d, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_GP);
    idt_attach_handler(0x0e, isr_seg, INIT_IDT_ATTRIBUTES(0, IDT_ATTRIBUTES_TYPE_TRAP, 0), &isr_PF);
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