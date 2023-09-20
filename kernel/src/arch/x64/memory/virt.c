#include "arch/x64/memory.h"
#include "fors/kerrno.h"
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

int map_page_4k(pml4_entry_t *pml4_table, uintptr_t phys, uintptr_t virt, unsigned int flags) {
    unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
    unsigned int pdpt_index = EXTRACT_PDPT_INDEX(virt);
    unsigned int pdt_index  = EXTRACT_PDT_INDEX(virt);
    unsigned int pt_index   = EXTRACT_PT_INDEX(virt);

    pml4_entry_t *pml4_entry = &(pml4_table[pml4_index]); // An entry in the pml4 table which points to one page directory pointer table

    // Allocate space for a new page directory pointer table, if needed
    if (!(*pml4_entry & PSE_PRESENT)) {
        pdpt_entry_t *new_pdpt = pfalloc_one();
        zero_paging_table(new_pdpt);

        *pml4_entry = PSE_PTR(new_pdpt) | flags;
    }

    pdpt_entry_t *pdpt_table = (pdpt_entry_t *)PSE_PTR(pml4_entry);
    pdpt_entry_t *pdpt_entry = &(pdpt_table[pdpt_index]); // An entry in the pdpt which points to one page directory table

    if (*pdpt_entry & PSE_PAGESIZE) {
        return EGENERIC;
    }

    if (!(*pdpt_entry & PSE_PRESENT)) {
        pdt_entry_t *new_pdt = pfalloc_one();
        zero_paging_table(new_pdt);
        
        *pdpt_entry = PSE_PTR(new_pdt) | flags;
    }

    pdt_entry_t *pdt_table = (pdt_entry_t *)PSE_PTR(pdpt_entry);
    pdt_entry_t *pdt_entry = &(pdt_table[pdt_index]); // An entry in the pdt which points to one page table

    if (*pdt_entry & PSE_PAGESIZE) {
        return EGENERIC;
    }

    if (!(*pdt_entry & PSE_PRESENT)) {
        pt_entry_t *new_pt = pfalloc_one();
        zero_paging_table(new_pt);

        *pdt_entry = PSE_PTR(new_pt) | flags;
    }

    pt_entry_t *page_table = (pt_entry_t *)PSE_PTR(pdt_entry);
    pt_entry_t *pt_entry = &(page_table[pt_index]); // An entry in the page table, which points to one 4K page

    // Here we don't allocate any memory for the 4K page, because the physical address is already
    // specified in 'phys'.
    // The caller should allocate a frame for the page.

    *pt_entry = PSE_PTR(phys) | flags;

    return 0;
}

/* Walk the page tables in software to work out where the virtual address `virt` maps to.
pml4_table: Virtual address of the root pml4 table to check within.
virt:       The virtual address to look up.
phys_ret:   Pointer to a uint64_t in which to place the result of the lookup. Set to NULL if lookup fails.
(return):   -1 on failure, 0 on success. */
int map_lookup(pml4_entry_t *pml4_table, uintptr_t virt, uintptr_t *phys_ret) {
    unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
    unsigned int pdpt_index = EXTRACT_PDPT_INDEX(virt);
    unsigned int pdt_index  = EXTRACT_PDT_INDEX(virt);
    unsigned int pt_index   = EXTRACT_PT_INDEX(virt);
    unsigned int page_offset;

    unsigned int page_startaddr = 0;

    printctrl(PRINTCTRL_LEADING_HEX | PRINTCTRL_RADIX_PREFIX);
    printk("==== Looking up mapping from %x\n", virt);
    printctrl_unset(PRINTCTRL_LEADING_HEX);

    if (!(pml4_table[pml4_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return ENOMAP;
    }

    printk("\tPDPT @ %x\n", PSE_PTR(pml4_table[pml4_index]));
    pdpt_entry_t *pdpt_table = (pdpt_entry_t*)(hhdm_offset + PSE_PTR(pml4_table[pml4_index]));

    if (!(pdpt_table[pdpt_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return ENOMAP;
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
        return ENOMAP;
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
        return ENOMAP;
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

int vmap(int pid, void *pa, void *va, int size, int flags) {
    pml4_entry_t *root_table;

    if (pid == -1) {
        root_table = kernel_pml4_table;
    } else {
        return EIMPL;
    }

    if (flags & VMAP_4K) {
        return map_page_4k(root_table, (uintptr_t)pa, (uintptr_t)va, 0);
    } else {
        return EIMPL;
    }
}

void *valloc(int pid, int size, unsigned int flags, size_t align) {
    if (pid == -1) {
        // Kernel allocation
        return NULL;
    } else if (pid >= 0) {
        // Process allocation not implemented yet
        return NULL;
    } else {
        // Invalid PID.
        return NULL;
    }
}