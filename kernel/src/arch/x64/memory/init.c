#include "arch/x64/idt.h"
#include "fors/memory.h"
#include "arch/x64/memory.h"
#include "fors/kheap.h"
#include "fors/printk.h"
#include <stdint.h>

volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

// The largest size that the kernel heap can ever grow to be. Must be a power of 2
#define KHEAP_MAXSIZE       65536 // 2^16

buddy_allocator kheap_alloc; // Defined in kheap.h

void arch_init_memory() {
    x64_init_physical_memory();
    x64_init_virtual_memory();

    x64_init_gdt();

    idt_init();

    // Find the next memory after the end of kernel memory that is a multiple of KHEAP_MAXSIZE.
    uint64_t kheap_start = (uint64_t)&_FORS_HEAP_START;
    
    printk("Kheap starts at %p (%d long/aligned)\n", kheap_start, KHEAP_MAXSIZE);

    if (vmap(-1, pfalloc_one(), (void*)kheap_start, ARCH_PAGE_SIZE, VMAP_4K | VMAP_WRIT) < 0) {
        printk("Failed to map in kernel heap.\n");
    }

    printk("Mapping in kernel heap.\n");

    buddy_init(ARCH_PAGE_SIZE, (void*)kheap_start, 5, &kheap_alloc);
}