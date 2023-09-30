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

#define KHEAP_SIZE 4096

buddy_allocator kheap_alloc; // Defined in kheap.h

extern const void *_FORS_KERNEL_END; // Defined in linker.ld as the end of the virtual memory the kernel's loaded at

void arch_init_memory() {
    x64_init_physical_memory();
    x64_init_virtual_memory();
    x64_init_gdt();

    idt_init();

    uint64_t kheap_start = (uint64_t)&_FORS_KERNEL_END + KHEAP_SIZE;
    uint64_t mask = 0xffffffffffffffff << __builtin_ctz(KHEAP_SIZE);
    kheap_start &= mask;
    
    printk("Kheap starts at %x (%d long/aligned)\n", kheap_start, KHEAP_SIZE);
    void *pageframe = pfalloc_one();
    printk("Mapping virt %x to phys %x\n", kheap_start, pageframe);
    vmap(-1, pageframe, (void*)kheap_start, 4096, VMAP_4K | VMAP_WRIT);

    buddy_init(KHEAP_SIZE, (void*)kheap_start, 5, &kheap_alloc);
}