#ifndef __INCLUDE_X64_MEMORY_H__
#define __INCLUDE_X64_MEMORY_H__

#include <assert.h>
#include <stdint.h>

#include "limine.h"

#define ARCH_PAGE_SIZE 4096 // The size in bytes of page frames and virtual pages

extern volatile struct limine_memmap_request memmap_req;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_kernel_address_request kernel_address_request;

extern const void *_FORS_KERNEL_START;
extern const void *_FORS_KERNEL_END; // Defined in linker.ld as the end of the virtual memory the kernel's loaded at

extern const void *_FORS_KERNEL_CODE_START;
extern const void *_FORS_KERNEL_CODE_END;

extern const void *_FORS_KERNEL_RO_START;
extern const void *_FORS_KERNEL_RO_END;

extern const void *_FORS_KERNEL_RW_START;
extern const void *_FORS_KERNEL_RW_END;

extern const void *_FORS_KERNEL_TEMP_MAP_PAGE;
extern const void *_FORS_HEAP_START;

#define FORS_CODE_OFFSET ((uint64_t)&_FORS_KERNEL_CODE_START - (uint64_t)&_FORS_KERNEL_START)
#define FORS_CODE_END_OFFSET ((uint64_t)&_FORS_KERNEL_CODE_END - (uint64_t)&_FORS_KERNEL_START)

#define FORS_RO_OFFSET ((uint64_t)&_FORS_KERNEL_RO_START - (uint64_t)&_FORS_KERNEL_START)
#define FORS_RO_END_OFFSET ((uint64_t)&_FORS_KERNEL_RO_END - (uint64_t)&_FORS_KERNEL_START)

#define FORS_RW_OFFSET ((uint64_t)&_FORS_KERNEL_RW_START - (uint64_t)&_FORS_KERNEL_START)
#define FORS_RW_END_OFFSET ((uint64_t)&_FORS_KERNEL_RW_END - (uint64_t)&_FORS_KERNEL_START)

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

#define TABLE_GDT   0
#define TABLE_LDT   1

#define KERNEL_CS   0x08 // RPL=0, TABLE=GDT, ENTRY=0x08 (index 1)
#define KERNEL_SS   0x10 // RPL=0, TABLE=GDT, ENTRY=0x10 (index 0x10/8 = 2)
#define USER_CS     0x1b
#define USER_SS     0x23

typedef uint64_t cr3_image;

// Paging structure entry macros
#define PSE_PTR(val) ((unsigned long)val & 0x000f'ffff'ffff'f000)
#define PSE_PRESENT     1L << 0
#define PSE_WRITABLE    1L << 1
#define PSE_USER        1L << 2
#define PSE_PWT         1L << 3
#define PSE_PCD         1L << 4
#define PSE_ACCESSED    1L << 5
#define PSE_DIRTY       1L << 6
#define PSE_PAGESIZE    1L << 7
#define PSE_PAT         1L << 7
#define PSE_GLOBAL      1L << 8
#define PSE_HLATRESTART 1L << 11
#define PSE_PT_PAT      1L << 12 // PAT bit is in a different place specifically for page table entries
#define PSE_XD          1L << 63

#define HH_MASK 0xffff'8000'0000'0000
#define ENFORCE_HH_CANONICAL(ptr)   ((ptr) | HH_MASK)
#define ENFORCE_LH_CANONICAL(ptr)   ((ptr) & ~HH_MASK)

#define PSE_GET_PTR(val) ENFORCE_HH_CANONICAL(PSE_PTR(val))

typedef uint64_t pml4_entry_t; // References a page directory pointer table
typedef uint64_t pml3_entry_t; // References a page directory
typedef uint64_t pml2_entry_t; // References a page table
typedef uint64_t pml1_entry_t; // References a physical page

extern pml4_entry_t *kernel_pml4_table;

#define EXTRACT_4K_PAGE_OFFSET(vaddr) (vaddr & 0xfff)
#define EXTRACT_2M_PAGE_OFFSET(vaddr) (vaddr & 0x1fffff)
#define EXTRACT_1G_PAGE_OFFSET(vaddr) (vaddr & 0x3fffffff)

#define EXTRACT_PML4_INDEX(vaddr)   ((vaddr >> 39) & 0x1ff)
#define EXTRACT_PML3_INDEX(vaddr)   ((vaddr >> 30) & 0x1ff)
#define EXTRACT_PML2_INDEX(vaddr)    ((vaddr >> 21) & 0x1ff)
#define EXTRACT_PML1_INDEX(vaddr)     ((vaddr >> 12) & 0x1ff)

struct __attribute__((packed)) gdt_descriptor_long {
    uint16_t limit_0_15;
    uint16_t base_0_15;
    uint8_t  base_16_23;
    uint8_t  access_byte;
    uint8_t  limit_16_19_and_flags;
    uint8_t  base_24_31;
};

struct __attribute__((packed)) tss_second_half {
    uint32_t base_32_63;
    uint32_t reserved;
};

typedef union gdt_table_entry {
    struct {
        uint16_t limit_0_15;
        uint16_t base_0_15;
        uint8_t  base_16_23;
        uint8_t  access_byte;
        uint8_t  limit_16_19_and_flags;
        uint8_t  base_24_31;
    };
    struct {
        uint32_t base_32_63;
        uint32_t reserved;
    };
} gdt_table_entry;

extern gdt_table_entry gdt_table[];

#define INIT_SEG_DESCRIPTOR(base, limit, access, flags) \
    (gdt_table_entry) { \
        .limit_0_15 = (limit) & 0xffff, \
        .base_0_15  = (base) & 0xffff, \
        .base_16_23 = ((base)>>16) & 0xff, \
        .access_byte = access, \
        .limit_16_19_and_flags = (((limit)>>16) & 0xf) | (((flags) & 0xf) << 4), \
        .base_24_31 = ((base)>>24) & 0xff, \
    }

#define INIT_TSS_DESCRIPTOR_FIRST_HALF(base, limit) \
    (gdt_table_entry) { \
        .limit_0_15 = (limit) & 0xffff, \
        .base_0_15  = (base) & 0xffff, \
        .base_16_23 = ((base)>>16) & 0xff, \
        .access_byte = 0b10001001, \
        .limit_16_19_and_flags = (((limit)>>16) & 0xf) | ((0b1000) << 4), \
        .base_24_31 = ((base)>>24) & 0xff, \
    }

#define INIT_TSS_DESCRIPTOR_SECOND_HALF(base) \
    (gdt_table_entry) { \
        .base_32_63 = ((base)>>32) & 0xffffffff, \
        .reserved = 0 }

struct __attribute__((packed)) gdtr_image {
    uint16_t limit;
    gdt_table_entry *base;
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
int map_page_4k(pml4_entry_t *pml4_table, uintptr_t phys, uintptr_t virt, unsigned int flags);

int map_lookup(pml4_entry_t *pml4_table, uintptr_t virt, uintptr_t *phys_ret);

void x64_init_physical_memory();
void x64_init_virtual_memory();
void x64_init_gdt();

// These two implemented in assembly
extern void load_gdt(void);
extern void reload_seg_registers(void);

#endif /* __INCLUDE_X64_MEMORY_H__ */