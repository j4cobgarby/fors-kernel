#include "arch/x64/memory.h"
#include "fors/memory.h"
#include "fors/printk.h"
#include "limine.h"

#include <stddef.h>
#include <stdlib.h>

pml4_entry_t *kernel_pml4_table = NULL;

volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

union cr3_image get_cr3() {
    union cr3_image ret;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(ret.as_u64));
    return ret;
}

void set_cr3(union cr3_image val) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(val.as_u64));
};

void zero_paging_table(union paging_structure table[512]) {
    for (int i = 0; i < 512; i++) {
        table[i].as_u64 = 0x0;
    }
}

void setup_kernel_map() {
    struct limine_kernel_address_response *kern_addr = kernel_address_request.response;
    struct limine_hhdm_response *hhdm = hhdm_request.response;
    struct limine_memmap_response *memmap = memmap_req.response;

    kernel_pml4_table = pfalloc_one();

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entr = memmap->entries[i];
    }
}

void x64_init_virtual_memory() {
    printctrl(PRINTCTRL_LEADING_HEX | PRINTCTRL_RADIX_PREFIX);
    printk("Direct map starts at virt %x\n", hhdm_request.response->offset);
    printk("Limine's loaded cr3 = %x\n", get_cr3().as_u64);

    kernel_pml4_table = (pml4_entry_t *)(hhdm_request.response->offset + get_cr3().pml4_address);
}

void map_page(pml4_entry_t *pml4_table, uint64_t phys, uint64_t virt, struct page_map_settings flags) {
    unsigned int pml4_index = EXTRACT_PML4_INDEX(virt);
    unsigned int pdpt_index = EXTRACT_PDPT_INDEX(virt);
    unsigned int pdt_index  = EXTRACT_PDT_INDEX(virt);
    unsigned int pt_index   = EXTRACT_PT_INDEX(virt);

    pml4_entry_t *pml4_entry = &(pml4_table[pml4_index]); // An entry in the pml4 table which points to one page directory pointer table

    // Allocate space for a new page directory pointer table, if needed
    if (!pml4_entry->present) {
        pdpt_entry_t *new_pdpt = pfalloc_one();
        zero_paging_table(new_pdpt);

        *pml4_entry = (pml4_entry_t){
            .target_physaddr = (unsigned long int)new_pdpt,
            .present = 1,
            .writable = flags.writable,         .user_accessable = flags.user_accessable,
            .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
            .hlat_restart = flags.hlat_restart, .xd = flags.xd
        };
    }

    pdpt_entry_t *pdpt_table = (pdpt_entry_t *)pml4_entry->target_physaddr;
    pdpt_entry_t *pdpt_entry = &(pdpt_table[pdpt_index]); // An entry in the pdpt which points to one page directory table

    if (!pdpt_entry->present) {
        pdt_entry_t *new_pdt = pfalloc_one();
        zero_paging_table(new_pdt);
        
        *pdpt_entry = (pdt_entry_t){
            .target_physaddr = (unsigned long int)new_pdt,
            .present = 1,
            .writable = flags.writable,         .user_accessable = flags.user_accessable,
            .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
            .hlat_restart = flags.hlat_restart, .xd = flags.xd
        };
    }

    pdt_entry_t *pdt_table = (pdt_entry_t *)pdpt_entry->target_physaddr;
    pdt_entry_t *pdt_entry = &(pdt_table[pdt_index]); // An entry in the pdt which points to one page table

    if (!pdt_entry->present) {
        pt_entry_t *new_pt = pfalloc_one();
        zero_paging_table((union paging_structure*)new_pt);

        *pdt_entry = (pdt_entry_t){
            .target_physaddr = (unsigned long int)new_pt,
            .present = 1,
            .writable = flags.writable,         .user_accessable = flags.user_accessable,
            .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
            .hlat_restart = flags.hlat_restart, .xd = flags.xd
        };
    }

    pt_entry_t *page_table = (pt_entry_t *)pdt_entry->target_physaddr;
    pt_entry_t *pt_entry = &(page_table[pt_index]); // An entry in the page table, which points to one 4K page

    // Here we don't allocate any memory for the 4K page, because the physical address is already
    // specified in 'phys'.
    // The caller should allocate a frame for the page.

    *pt_entry = (pt_entry_t){
        .target_physaddr = phys,            .present = 1,
        .writable = flags.writable,         .user_accessable = flags.user_accessable,
        .pwt = flags.pagelvl_write_through, .pcd = flags.pagelvl_cache_disable,
        .hlat_restart = flags.hlat_restart, .xd = flags.xd,
        .global = flags.global,             .pat = flags.pat
    };
}