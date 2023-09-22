#ifndef __INCLUDE_X64_IDT_H__
#define __INCLUDE_X64_IDT_H__

#include <stdint.h>

#include "arch/x64/memory.h"

/* An idt_attributes_t type represents the attribute field within an IDT table
 * entry. It contains information about the type of entry this is, which rings
 * can invoke the interrupt, and information relating to the task state segment.
 *
 * Bits 0-2 are the IST value. This refers to an index into the interrupt stack
 * table, if used. These entries can be used to load stacks when switching to
 * and from interrupt routines. This can be set to 0 if unneeded.
 *
 * Bits 8-11 are the gate type. In "long" (64-bit) mode, which is what this
 * header file deals with, this can be either:
 *  0xe, for a 64-bit interrupt gate, or
 *  0xf, for a 64-bit trap gate.
 *
 * Bits 13-14 are a 2-bit value representing the minimum privilege level needed
 * to invoke this interrupt from software.
 *
 * Bit 15 is 1 if this IDT entry is present in the IDT, or 0 otherwise.
 */

typedef uint16_t idt_attributes_t;
#define INIT_IDT_ATTRIBUTES(dpl, type, ist) \
    (1<<15 | (ist & 0x07) | ((type & 0xf) << 8) | ((dpl & 0x03) << 13))
#define IDT_ATTRIBUTES_IST(attributes) ((attributes) & 0x07)
#define IDT_ATTRIBUTES_TYPE(attributes) ((attributes >> 8) & 0x0f)
#define IDT_ATTRIBUTES_DPL(attributes) ((attributes >> 13) & 0x03)
#define IDT_ATTRIBUTES_PRESENT(attributes) ((attributes >> 15) & 0x01)

#define IDT_ATTRIBUTES_TYPE_INTERRUPT 0xe
#define IDT_ATTRIBUTES_TYPE_TRAP      0xf

/* An idt_entry refers to one entry in the IDT table.
 */
struct idt_entry {
    uint16_t offset_a; // Bits 0-15 of the address of the ISR.
    union segment_selector segment; 
    idt_attributes_t attributes;
    uint16_t offset_b; // Bits 16-31 of the address of the ISR.
    uint32_t offset_c; // Bits 32-63 of the address of the ISR.
    uint32_t reserved; // Reserved according to intel's specs. Set to 0.
} __attribute__((packed));

#define INIT_IDT_ENTRY(seg, attr, offset64) ((struct idt_entry) { \
    .segment = (seg), \
    .attributes = (attr), \
    .offset_a = (offset64 & 0xffff), \
    .offset_b = ((offset64 >> 16) & 0xffff), \
    .offset_c = ((offset64 >> 32) & 0xffffffff), \
})

/* The IDT table is a 4096 byte (1 4KB page) long table of 256 IDT entries.
 * When an interrupt (hardware or software) is invoked (for example a keypress
 * or an instruction like `INT 50`), or an exception (like a page fault) occurs
 * then the CPU will look up the entry in this table corresponding to the 
 * interrupt vector number. Vectors for exceptions can be found at:
 *  https://wiki.osdev.org/Exceptions.
 * Once the IDT entry is found, and is present, the CPU will jump to the
 * specified service routine and start executing there.
 */

#define IDT_N_ENTRIES 256
extern struct idt_entry idt_table[IDT_N_ENTRIES];

/* An IDT descriptor is a structure that's passed to the LIDT instruction, in
 * order to tell the CPU to start using the IDT which this descriptor refers
 * to.
 */
struct idt_descriptor {
    uint16_t idt_size; // Size (bytes) of the IDT, minus 1.
    void* offset; // The address of the IDT. Paging applies!
} __attribute__((packed));

#define INIT_IDT_DESCRIPTOR(idt_offset, n_entries) ((struct idt_descriptor) { \
    .offset = (idt_offset), \
    .idt_size = ((n_entries) * sizeof(struct idt_entry)) - 1, \
})

struct isr_frame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} __attribute__((packed));

void idt_init();
void idt_load(struct idt_entry* table, int n_entries);

#define INTERRUPT_HANDLER __attribute__((interrupt)) void

void idt_attach_handler(int vector, union segment_selector seg, idt_attributes_t attr, void *handler);

/* Here we declare functions for many of the possible exceptions that can occur, as
 * well as other interrupts. Documentation about exceptions can be found in Vol. 3A,
 * Section 6 of the Intel Developer Manual, or at https://wiki.osdev.org/Exceptions.
 */
INTERRUPT_HANDLER isr_DE(struct isr_frame *frame);  // 0x00 Division Error
INTERRUPT_HANDLER isr_NMI(struct isr_frame *frame); // 0x02 Non-Maskable Interrupt
INTERRUPT_HANDLER isr_BP(struct isr_frame *frame);  // 0x03 Breakpoint
INTERRUPT_HANDLER isr_OF(struct isr_frame *frame);  // 0x04 Overflow
INTERRUPT_HANDLER isr_BR(struct isr_frame *frame);  // 0x05 Bound Range Exceeded
INTERRUPT_HANDLER isr_UD(struct isr_frame *frame);  // 0x06 Invalid Opcode
INTERRUPT_HANDLER isr_DF(struct isr_frame *frame, uint64_t error_code); // 0x08 Double Fault
INTERRUPT_HANDLER isr_TS(struct isr_frame *frame, uint64_t error_code); // 0x0a Invalid TSS
INTERRUPT_HANDLER isr_NP(struct isr_frame *frame, uint64_t error_code); // 0x0b Segment Not Present
INTERRUPT_HANDLER isr_SS(struct isr_frame *frame, uint64_t error_code); // 0x0c Stack Segment Fault
INTERRUPT_HANDLER isr_GP(struct isr_frame *frame, uint64_t error_code); // 0x0d General Protection Fault
INTERRUPT_HANDLER isr_PF(struct isr_frame *frame, uint64_t error_code); // 0x0e Page Fault

INTERRUPT_HANDLER isr_SYSCALL(struct isr_frame *frame); // 0x32 Syscall

#endif /* __INCLUDE_X64_IDT_H__ */