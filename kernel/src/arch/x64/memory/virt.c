#include "arch/x64/memory.h"
#include "fors/kerrno.h"
#include "fors/memory.h"
#include "fors/printk.h"
#include "fors/thread.h"
#include "limine.h"

#include <stddef.h>
#include <stdlib.h>

/* *Virtual* address of the kernel's PML4 table 

This is the virtual address of the kernel's PML4 table. The address will be
within the higher-half memory direct map of physical memory.*/
pml4_entry_t *kernel_pml4_table = NULL;

uint64_t hhdm_offset;

static inline cr3_image get_cr3() {
    cr3_image ret;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(ret));
    return ret;
}

static inline void set_cr3(cr3_image val) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(val) : "memory");
};

static inline void flush_tlb(unsigned long virt) {
   __asm__ volatile("invlpg (%0)" : : "r" (virt) : "memory");
}

void zero_paging_table(uint64_t table[512]) {
    for (int i = 0; i < 512; i++) {
        table[i] = 0x0;
    }
}

void x64_init_virtual_memory() {
    kernel_pml4_table = (pml4_entry_t *)(hhdm_request.response->offset + get_cr3());
    hhdm_offset = hhdm_request.response->offset;

    printk("Direct map starts at virt %p\n", hhdm_offset);
    printk("Limine's loaded pml4 table = %p (CR3 = %#x)\n", kernel_pml4_table, get_cr3());
}

int map_page_4k(pml4_entry_t *pml4_table, uintptr_t phys, uintptr_t virt, unsigned int flags) {
    unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
    unsigned int pml3_index = EXTRACT_PML3_INDEX(virt);
    unsigned int pml2_index = EXTRACT_PML2_INDEX(virt);
    unsigned int pml1_index = EXTRACT_PML1_INDEX(virt);

    // printk("==== Mapping 4k page ====\n");
    // printk("Indices: PML4[%d] -> PML3[%d] -> PML2[%d] -> PML1[%d]\n",
    //     pml4_index, pml3_index, pml2_index, pml1_index);

    pml4_entry_t *pml4_entry = &(pml4_table[pml4_index]);

    if (!(*pml4_entry & PSE_PRESENT)) {
        // If the pml4 entry was not set as present, then create a new pml3 table.
        pml3_entry_t *new_pml3 = pfalloc_one();

        if (!new_pml3) {
            printk("Failed to allocate page frame for PML3 table.\n");
            return ENOMEM;
        }

        zero_paging_table(new_pml3);

        *pml4_entry = PSE_PTR(new_pml3);
    }
    
    *pml4_entry |= flags | PSE_PRESENT;

    pml3_entry_t *pml3_table = (pml3_entry_t *)PSE_GET_PTR(*pml4_entry);
    pml3_entry_t *pml3_entry = &(pml3_table[pml3_index]); 

    if (*pml3_entry & PSE_PAGESIZE) {
        return EGENERIC;
    }

    if (!(*pml3_entry & PSE_PRESENT)) {
        pml2_entry_t *new_pml2 = pfalloc_one();

        if (!new_pml2) {
            printk("Failed to allocate page frame for PML2 table.\n");
            return ENOMEM;
        }

        zero_paging_table(new_pml2);
        
        *pml3_entry = PSE_PTR(new_pml2);
    }

    *pml3_entry |= flags | PSE_PRESENT;

    pml2_entry_t *pml2_table = (pml2_entry_t *)PSE_GET_PTR(*pml3_entry);
    pml2_entry_t *pml2_entry = &(pml2_table[pml2_index]); // An entry in the pdt which points to one page table

    if (*pml2_entry & PSE_PAGESIZE) {
        return EGENERIC;
    }

    if (!(*pml2_entry & PSE_PRESENT)) {
        pml1_entry_t *new_pml1 = pfalloc_one();

        if (!new_pml1) {
            printk("Failed to allocate page frame for PML1 table.\n");
            return ENOMEM;
        }

        zero_paging_table(new_pml1);

        *pml2_entry = PSE_PTR(new_pml1);
    }

    *pml2_entry |= flags | PSE_PRESENT;

    pml1_entry_t *pml1_table = (pml1_entry_t *)PSE_GET_PTR(*pml2_entry);
    pml1_entry_t *pml1_entry = &(pml1_table[pml1_index]); // An entry in the page table, which points to one 4K page

    // Here we don't allocate any memory for the 4K page, because the physical address is already
    // specified in 'phys'.
    // The caller should allocate a frame for the page.

    // printk("[map] pml1 table at %p\n", pml1_table);
    // printk("[map] pml1 entry at %p\n", pml1_entry);

    *pml1_entry = PSE_PTR(phys) | flags;

    flush_tlb(virt);

    // printk("==== Finished Mapping ====\n");

    return 0;
}

/* Walk the page tables in software to work out where the virtual address `virt` maps to.
pml4_table: Virtual address of the root pml4 table to check within.
virt:       The virtual address to look up.
phys_ret:   Pointer to a uint64_t in which to place the result of the lookup. Set to NULL if lookup fails.
(return):   -1 on failure, 0 on success. */
int map_lookup(pml4_entry_t *pml4_table, uintptr_t virt, uintptr_t *phys_ret) {
    unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
    unsigned int pdpt_index = EXTRACT_PML3_INDEX(virt);
    unsigned int pdt_index  = EXTRACT_PML2_INDEX(virt);
    unsigned int pt_index   = EXTRACT_PML1_INDEX(virt);
    unsigned int page_offset;

    unsigned int page_startaddr = 0;

    // printk("==== Looking up mapping from %p\n", virt);

    if (!(pml4_table[pml4_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return ENOMAP;
    }

    printk("\tPDPT @ %p\n", PSE_PTR(pml4_table[pml4_index]));
    pml3_entry_t *pdpt_table = (pml3_entry_t*)(hhdm_offset + PSE_PTR(pml4_table[pml4_index]));

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

    printk("\tPDT @ %p\n", PSE_PTR(pdpt_table[pdpt_index]));
    pml2_entry_t *pdt_table = (pml2_entry_t*)(hhdm_offset + PSE_PTR(pdpt_table[pdpt_index]));

    if (!(pdt_table[pdt_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return ENOMAP;
    }

    if (pdt_table[pdt_index] & PSE_PAGESIZE) {
        page_startaddr = PSE_PTR(pdt_table[pdt_index]);
        page_offset = EXTRACT_2M_PAGE_OFFSET(virt);
        goto set_addr;
    }

    printk("\tPT @ %p\n", PSE_PTR(pdt_table[pdt_index]));
    pml1_entry_t *page_table = (pml1_entry_t*)(hhdm_offset + PSE_PTR(pdt_table[pdt_index]));
    if (!(page_table[pt_index] & PSE_PRESENT)) {
        *phys_ret = 0;
        return ENOMAP;
    }

    page_startaddr = PSE_PTR(page_table[pt_index]);
    page_offset = EXTRACT_4K_PAGE_OFFSET(virt);

set_addr:
    // printk("\tPage @ %p\n", page_startaddr);
    *phys_ret = page_startaddr + page_offset;
    // printk("\tPage offset = %x\n", page_offset);
    // printk("\tPhysical address = %x\n", *phys_ret);
    return 0;
}

void *tmpmap(void *pa) {
    if (map_page_4k(kernel_pml4_table, (uintptr_t)pa, (uintptr_t)&_FORS_KERNEL_TEMP_MAP_PAGE, PSE_PRESENT | PSE_WRITABLE) < 0) {
        return NULL;
    }

    return (void*)&_FORS_KERNEL_TEMP_MAP_PAGE;
}

int vmap(int pid, void *pa, void *va, int size, int flags) {
    pml4_entry_t *root_table;

    uint64_t mapping_flags = 0;
    if (flags & VMAP_USER) mapping_flags |= PSE_USER;
    if (flags & VMAP_WRIT) mapping_flags |= PSE_WRITABLE;
    if (!(flags & VMAP_EXEC)) mapping_flags |= PSE_XD;

    if (pid == -1) {
        root_table = kernel_pml4_table;
    } else {
        thread th = threads[pid];
        if (!th.present) return EINVARG;
        root_table = (pml4_entry_t*)th.ctx.cr3;
    }

    if (flags & VMAP_4K) {
        return map_page_4k(root_table, (uintptr_t)pa, (uintptr_t)va, mapping_flags | PSE_PRESENT);
    } else {
        return EIMPL;
    }
}

void *allocate_stack() {
    // TODO: Make this stack larger than this. It's far too small.
    // Also, at the moment it's not mapped anywhere. I just wanna get it usable.

    return hhdm_offset + pfalloc_one() + ARCH_PAGE_SIZE;
}
