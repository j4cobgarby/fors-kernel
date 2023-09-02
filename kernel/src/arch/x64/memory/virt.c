#include "arch/x64/memory.h"
#include "fors/memory.h"
#include "fors/printk.h"
#include "limine.h"

#include <stddef.h>
#include <stdlib.h>

/* *Virtual* address of the kernel's PML4 table 

This is the virtual address of the kernel's PML4 table. The address will be
within the higher-half memory direct map of physical memory.*/
pml4_entry_t *kernel_pml4_table = NULL;

uint64_t hhdm_offset;

volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

cr3_image get_cr3() {
    cr3_image ret;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(ret));
    return ret;
}

void set_cr3(cr3_image val) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(val));
};

void zero_paging_table(uint64_t table[512]) {
    for (int i = 0; i < 512; i++) {
        table[i] = 0x0;
    }
}

void x64_init_virtual_memory() {
    kernel_pml4_table = (pml4_entry_t *)(hhdm_request.response->offset + get_cr3());
    hhdm_offset = hhdm_request.response->offset;

    printctrl(PRINTCTRL_LEADING_HEX | PRINTCTRL_RADIX_PREFIX);
    printk("Direct map starts at virt %x\n", hhdm_offset);
    printk("Limine's loaded pml4 table = %x\n", kernel_pml4_table);

    uint64_t phys;
    if (map_lookup(kernel_pml4_table, (uint64_t)kernel_pml4_table, &phys) < 0) {
        printk("Failed to lookup mapping.\n");
    }
}

// void map_page(pml4_entry_t *pml4_table, uint64_t phys, uint64_t virt, struct page_map_settings flags) {
//     unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
//     unsigned int pdpt_index = EXTRACT_PDPT_INDEX(virt);
//     unsigned int pdt_index  = EXTRACT_PDT_INDEX(virt);
//     unsigned int pt_index   = EXTRACT_PT_INDEX(virt);

//     pml4_entry_t *pml4_entry = &(pml4_table[pml4_index]); // An entry in the pml4 table which points to one page directory pointer table

//     // Allocate space for a new page directory pointer table, if needed
//     if (!pml4_entry->present) {
//         pdpt_entry_t *new_pdpt = pfalloc_one();
//         zero_paging_table(new_pdpt);

//         *pml4_entry = (pml4_entry_t){
//             .target_physaddr = (unsigned long int)new_pdpt,
//             .present = 1,
//             .writable = flags.writable,         .user_accessable = flags.user_accessable,
//             .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
//             .hlat_restart = flags.hlat_restart, .xd = flags.xd
//         };
//     }

//     pdpt_entry_t *pdpt_table = (pdpt_entry_t *)pml4_entry->target_physaddr;
//     pdpt_entry_t *pdpt_entry = &(pdpt_table[pdpt_index]); // An entry in the pdpt which points to one page directory table

//     if (!pdpt_entry->present) {
//         pdt_entry_t *new_pdt = pfalloc_one();
//         zero_paging_table(new_pdt);
        
//         *pdpt_entry = (pdt_entry_t){
//             .target_physaddr = (unsigned long int)new_pdt,
//             .present = 1,
//             .writable = flags.writable,         .user_accessable = flags.user_accessable,
//             .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
//             .hlat_restart = flags.hlat_restart, .xd = flags.xd
//         };
//     }

//     pdt_entry_t *pdt_table = (pdt_entry_t *)pdpt_entry->target_physaddr;
//     pdt_entry_t *pdt_entry = &(pdt_table[pdt_index]); // An entry in the pdt which points to one page table

//     if (!pdt_entry->present) {
//         pt_entry_t *new_pt = pfalloc_one();
//         zero_paging_table((union paging_structure*)new_pt);

//         *pdt_entry = (pdt_entry_t){
//             .target_physaddr = (unsigned long int)new_pt,
//             .present = 1,
//             .writable = flags.writable,         .user_accessable = flags.user_accessable,
//             .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
//             .hlat_restart = flags.hlat_restart, .xd = flags.xd
//         };
//     }

//     pt_entry_t *page_table = (pt_entry_t *)pdt_entry->target_physaddr;
//     pt_entry_t *pt_entry = &(page_table[pt_index]); // An entry in the page table, which points to one 4K page

//     // Here we don't allocate any memory for the 4K page, because the physical address is already
//     // specified in 'phys'.
//     // The caller should allocate a frame for the page.

//     *pt_entry = (pt_entry_t){
//         .target_physaddr = phys,            .present = 1,
//         .writable = flags.writable,         .user_accessable = flags.user_accessable,
//         .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
//         .hlat_restart = flags.hlat_restart, .xd = flags.xd,
//         .global = flags.global,             .pat = flags.pat
//     };
// }

/* Walk the page tables in software to work out where the virtual address `virt` maps to.
pml4_table: Virtual address of the root pml4 table to check within.
virt:       The virtual address to look up.
phys_ret:   Pointer to a uint64_t in which to place the result of the lookup. Set to NULL if lookup fails.
(return):   -1 on failure, 0 on success. */
int map_lookup(pml4_entry_t *pml4_table, uint64_t virt, uint64_t *phys_ret) {
    unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
    unsigned int pdpt_index = EXTRACT_PDPT_INDEX(virt);
    unsigned int pdt_index  = EXTRACT_PDT_INDEX(virt);
    unsigned int pt_index   = EXTRACT_PT_INDEX(virt);
    unsigned int page_offset;

    unsigned int page_startaddr = 0;

    printctrl(PRINTCTRL_LEADING_HEX | PRINTCTRL_RADIX_PREFIX);
    printk("==== Looking up mapping from %x\n", virt);
    printctrl_unset(PRINTCTRL_LEADING_HEX);
    printk("\tIndexes: PML4[%x], PDPT[%x], PDT[%x], PT[%x]\n", pml4_index, pdpt_index, pdt_index, pt_index);

    if (!(pml4_table[pml4_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return -1;
    }

    printk("\tPDPT @ %x\n", PSE_PTR(pml4_table[pml4_index]));
    pdpt_entry_t *pdpt_table = (pdpt_entry_t*)(hhdm_offset + PSE_PTR(pml4_table[pml4_index]));

    if (!(pdpt_table[pdpt_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return -1;
    }

    // If this pdpt entry maps a 1GB page instead of pointing to a pdt
    if (pdpt_table[pdpt_index] & PSE_PAGESIZE) {
        page_startaddr = PSE_PTR(pdpt_table[pdpt_index]);
        page_offset = EXTRACT_1G_PAGE_OFFSET(virt);
        goto set_addr;
    }

    printk("\tPDT @ %x\n", PSE_PTR(pdpt_table[pdpt_index]));
    pdt_entry_t *pdt_table = (pdt_entry_t*)(hhdm_offset + PSE_PTR(pdpt_table[pdpt_index]));

    if (!(pdt_table[pdt_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return -1;
    }

    if (pdt_table[pdt_index] & PSE_PAGESIZE) {
        page_startaddr = PSE_PTR(pdt_table[pdt_index]);
        page_offset = EXTRACT_2M_PAGE_OFFSET(virt);
        goto set_addr;
    }

    printk("\tPT @ %x\n", PSE_PTR(pdt_table[pdt_index]));
    pt_entry_t *page_table = (pt_entry_t*)(hhdm_offset + PSE_PTR(pdt_table[pdt_index]));
    if (!(page_table[pt_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return -1;
    }

    page_startaddr = PSE_PTR(page_table[pt_index]);
    page_offset = EXTRACT_4K_PAGE_OFFSET(virt);

set_addr:
    printk("\tPage @ %x\n", page_startaddr);
    *phys_ret = page_startaddr + page_offset;
    printk("\tPage offset = %x\n", page_offset);
    printk("\tPhysical address = %x\n", *phys_ret);
    return 0;
}