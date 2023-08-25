#ifndef __INCLUDE_X64_MEMORY_H__
#define __INCLUDE_X64_MEMORY_H__

#include <assert.h>
#include <stdint.h>

#define ARCH_PAGE_SIZE 4096 // The size in bytes of page frames and virtual pages

struct frame_marker {
    struct frame_marker *next;
    struct frame_marker *prev;
};

void x64_init_physical_memory();
void x64_init_virtual_memory();
void x64_init_gdt();

extern void load_gdt(void);
extern void reload_seg_registers(void);

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

union cr3_image {
    uint64_t as_u64;
    struct __attribute__((packed)) {
        unsigned int _0 : 3;
        unsigned int pwt : 1; // Page-level write-through (see SDM 4.9.2)
        unsigned int pcd : 1; // Page-level cache disable (see SDM 4.9.2)
        unsigned int _1 : 7;
        unsigned long int pml4_address : 40;
    } element;
};
static_assert(sizeof(union cr3_image) == 8, "");

union paging_structure {
    uint64_t as_u64;
    struct __attribute__((packed)) {
        unsigned int present : 1;
        unsigned int writable : 1;
        unsigned int user_accessable : 1;
        unsigned int pwt : 1;
        unsigned int pcd : 1;
        unsigned int accessed : 1;
        unsigned int _0: 1;
        unsigned int zero0: 1;
        unsigned int _1 : 3;
        unsigned int hlat_restart : 1;
        unsigned long int target_physaddr : 40;
        unsigned int _3 : 11;
        unsigned int xd : 1;
    };
};
static_assert(sizeof(union paging_structure) == 8, "");

typedef union paging_structure pml4_entry_t; // References a page directory pointer table
typedef union paging_structure pdpt_entry_t; // References a page directory
typedef union paging_structure pd_entry_t; // References a page table

union page_table_entry {
    uint64_t as_u64;
    struct __attribute__((packed)) {
        unsigned int present : 1;
        unsigned int writable : 1;
        unsigned int user_accessable : 1;
        unsigned int pwt : 1;
        unsigned int pcd : 1;
        unsigned int accessed : 1;
        unsigned int dirty: 1;
        unsigned int pat: 1;
        unsigned int global : 1;
        unsigned int _0 : 2;
        unsigned int hlat_restart : 1;
        unsigned long int target_physaddr : 40;
        unsigned int _3 : 7;
        unsigned int protection_key : 4;
        unsigned int xd : 1;
    };
};
static_assert(sizeof(union page_table_entry) == 8, "");

typedef union page_table_entry pt_entry_t;

#define EXTRACT_PML4_INDEX(vaddr)   ((vaddr >> 39) & 0x1ff)
#define EXTRACT_PDPT_INDEX(vaddr)   ((vaddr >> 30) & 0x1ff)
#define EXTRACT_PDT_INDEX(vaddr)    ((vaddr >> 21) & 0x1ff)
#define EXTRACT_PT_INDEX(vaddr)     ((vaddr >> 12) & 0x1ff)
#define EXTRACT_PAGE_OFFSET(vaddr)  (vaddr & 0xfff)

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

#define SEG_AB_ACCESSED 1 << 0
#define SEG_AB_RW       1 << 1
#define SEG_AB_DC       1 << 2
#define SEG_AB_EXEC     1 << 3  // If set, this segment is executable.
#define SEG_AB_CODE_DATA 1 << 4 // If set, this segment is code/data, otherwise it's some sort of system segment
#define SEG_AB_DPL(n)   ((n & 0x3) << 5)
#define SEG_AB_PRESENT  1 << 7 // Must be set for any valid segment

#define SEG_FLAG_LONG       1 << 1 // Set for a 64 bit code segment
#define SEG_FLAG_DB         1 << 2 // Size of segment, set for 32 bit
#define SEG_FLAG_4K_BLOCKS  1 << 3

#endif /* __INCLUDE_X64_MEMORY_H__ */