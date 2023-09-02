#ifndef __INCLUDE_X64_MEMORY_H__
#define __INCLUDE_X64_MEMORY_H__

#include <assert.h>
#include <stdint.h>

#include "limine.h"

#define ARCH_PAGE_SIZE 4096 // The size in bytes of page frames and virtual pages

extern volatile struct limine_memmap_request memmap_req;

struct frame_marker {
    struct frame_marker *next;
    struct frame_marker *prev;
};

/* Memory Structures */

/* A segment_selector union represents a segment selector, which is used all
 * over the place when dealing with memory segmentation.
 * 
 * Bits 0-1 make up the requested privilege level. This is the "ring".
 *  0 = Kernel mode, 1,2 = Drivers, 3 = Userspace
 * The flux kernel uses only rings 0 and 3.
 *
 * Bit 2 specifies the descriptor table that the segment being referred to is
 *  in. 0 = GDT, 1 = LDT.
 *
 * Bits 3-15 contain the index of the entry in the specified table (GDT or LDT).
 */
union segment_selector {
    uint16_t as_u16;
    struct __attribute__((packed)) {
        unsigned int rpl : 2;
        unsigned int table : 1;
        unsigned int entry : 13;
    } element;
};
static_assert(sizeof(union segment_selector) == 2, "");

#define TABLE_GDT 0
#define TABLE_LDT 1

typedef uint64_t cr3_image;

// Paging structure entry macros
#define PSE_PTR(val) (val & 0x000ffffffffff000)
#define PSE_PRESENT     1 << 0
#define PSE_WRITABLE    1 << 1
#define PSE_USER        1 << 2
#define PSE_PWT         1 << 3
#define PSE_PCD         1 << 4
#define PSE_ACCESSED    1 << 5
#define PSE_DIRTY       1 << 6
#define PSE_PAGESIZE    1 << 7
#define PSE_PAT         1 << 7
#define PSE_GLOBAL      1 << 8
#define PSE_HLATRESTART 1 << 11
#define PSE_PT_PAT      1 << 12 // PAT bit is in a different place specifically for page table entries
#define PSE_XD          1 << 63

typedef uint64_t pml4_entry_t; // References a page directory pointer table
typedef uint64_t pdpt_entry_t; // References a page directory
typedef uint64_t pdt_entry_t; // References a page table

typedef uint64_t pt_entry_t;

#define EXTRACT_4K_PAGE_OFFSET(vaddr) (vaddr & 0xfff)
#define EXTRACT_2M_PAGE_OFFSET(vaddr) (vaddr & 0x1fffff)
#define EXTRACT_1G_PAGE_OFFSET(vaddr) (vaddr & 0x3fffffff)

#define EXTRACT_PML4_INDEX(vaddr)   ((vaddr >> 39) & 0x1ff)
#define EXTRACT_PDPT_INDEX(vaddr)   ((vaddr >> 30) & 0x1ff)
#define EXTRACT_PDT_INDEX(vaddr)    ((vaddr >> 21) & 0x1ff)
#define EXTRACT_PT_INDEX(vaddr)     ((vaddr >> 12) & 0x1ff)
#define EXTRACT_PAGE_OFFSET(vaddr)  EXTRACT_4K_PAGE_OFFSET(vaddr)


struct __attribute__((packed)) gdt_descriptor_long {
    uint16_t limit_0_15;
    uint16_t base_0_15;
    uint8_t  base_16_23;
    uint8_t  access_byte;
    uint8_t  limit_16_19_and_flags;
    uint8_t  base_24_31;
};

extern struct gdt_descriptor_long gdt_table[];

#define INIT_SEG_DESCRIPTOR(base, limit, access, flags) \
    (struct gdt_descriptor_long) { \
        .limit_0_15 = (limit) & 0xffff, \
        .base_0_15  = (base) & 0xffff, \
        .base_16_23 = ((base)>>16) & 0xff, \
        .access_byte = access, \
        .limit_16_19_and_flags = (((limit)>>16) & 0xf) | (((flags) & 0xf) << 4), \
        .base_24_31 = ((base)>>24) & 0xff, \
    }

struct __attribute__((packed)) gdtr_image {
    uint16_t limit;
    struct gdt_descriptor_long *base;
};

#define SEG_AB_ACCESSED     1 << 0
#define SEG_AB_RW           1 << 1
#define SEG_AB_DC           1 << 2
#define SEG_AB_EXEC         1 << 3  // If set, this segment is executable.
#define SEG_AB_CODE_DATA    1 << 4 // If set, this segment is code/data, otherwise it's some sort of system segment
#define SEG_AB_DPL(n)       ((n & 0x3) << 5)
#define SEG_AB_PRESENT      1 << 7 // Must be set for any valid segment

#define SEG_FLAG_LONG       1 << 1 // Set for a 64 bit code segment
#define SEG_FLAG_DB         1 << 2 // Size of segment, set for 32 bit
#define SEG_FLAG_4K_BLOCKS  1 << 3

// Map the virtual page 'virt' to the page frame at 'phys'.
void map_page(pml4_entry_t *pml4_table, uint64_t phys, uint64_t virt, struct page_map_settings flags);
int map_lookup(pml4_entry_t *pml4_table, uint64_t virt, uint64_t *phys_ret);

void x64_init_physical_memory();
void x64_init_virtual_memory();
void x64_init_gdt();

// These two implemented in assembly
extern void load_gdt(void);
extern void reload_seg_registers(void);

#endif /* __INCLUDE_X64_MEMORY_H__ */