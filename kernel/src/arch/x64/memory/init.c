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

#define KHEAP_SIZE 65536

buddy_allocator kheap_alloc; // Defined in kheap.h

void arch_init_memory() {
    x64_init_physical_memory();
    x64_init_virtual_memory();
    x64_init_gdt();

    uint64_t hhdm_start = hhdm_request.response->offset;
    uint64_t kernel_start = kernel_address_request.response->virtual_base;
    uint64_t first = hhdm_start < kernel_start ? hhdm_start : kernel_start;

    uint64_t kheap_start = first - KHEAP_SIZE;
    uint64_t mask = 0xffffffffffffffff << __builtin_ctz(KHEAP_SIZE);
    kheap_start &= mask;

    printk("Kheap starts at %x (%d long/aligned)\n", kheap_start, KHEAP_SIZE);

    //buddy_init(KHEAP_SIZE, (void*)kheap_start, 5, &kheap_alloc);

    //printk("Initialised kernel heap.\n");
}