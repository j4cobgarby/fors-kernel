#ifndef __INCLUDE_X64_IDT_H__
#define __INCLUDE_X64_IDT_H__

#include <stdint.h>

#include "arch/x64/memory.h"

#define INT_DE  0  // Divide by zero
#define INT_DB  1  // Debug
#define INT_NMI 2  // Non-maskable
#define INT_BP  3  // Breakpoint
#define INT_OF  4  // Overflow
#define INT_BR  5  // Bound range exceeded
#define INT_UD  6  // Invalid opcode
#define INT_NM  7  // Device not available
#define INT_DF  8  // Double fault
#define INT_TS  10 // Invalid TSS
#define INT_NP  11 // Segment not present
#define INT_SS  12 // Stack segment fault
#define INT_GP  13 // General protection
#define INT_PF  14 // Page fault
#define INT_MF  16 // x87 FPU error
#define INT_AC  17 // Alignment check
#define INT_MC  18 // Machine check
#define INT_XF  19 // SIMD error

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
#define INIT_IDT_ATTRIBUTES(dpl, type, ist)                                              \
    (1 << 15 | (ist & 0x07) | ((type & 0xf) << 8) | ((dpl & 0x03) << 13))
#define IDT_ATTRIBUTES_IST(attributes)     ((attributes) & 0x07)
#define IDT_ATTRIBUTES_TYPE(attributes)    ((attributes >> 8) & 0x0f)
#define IDT_ATTRIBUTES_DPL(attributes)     ((attributes >> 13) & 0x03)
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

#define INIT_IDT_ENTRY(seg, attr, offset64)                                              \
    ((struct idt_entry) {                                                                \
        .segment = (seg),                                                                \
        .attributes = (attr),                                                            \
        .offset_a = (offset64 & 0xffff),                                                 \
        .offset_b = ((offset64 >> 16) & 0xffff),                                         \
        .offset_c = ((offset64 >> 32) & 0xffffffff),                                     \
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
    void *offset;      // The address of the IDT. Paging applies!
} __attribute__((packed));

#define INIT_IDT_DESCRIPTOR(idt_offset, n_entries)                                       \
    ((struct idt_descriptor) {                                                           \
        .offset = (idt_offset),                                                          \
        .idt_size = ((n_entries) * sizeof(struct idt_entry)) - 1,                        \
    })

struct isr_frame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
} __attribute__((packed));

void idt_init();
void idt_load(struct idt_entry *table, int n_entries);

void idt_attach_handler(
    int vector, union segment_selector seg, idt_attributes_t attr, void *handler);

#endif /* __INCLUDE_X64_IDT_H__ */
