#include "fors/memory.h"
#include "arch/x64/memory.h"
#include "arch/x64/uart.h"

volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

void arch_init_memory() {
    x64_init_physical_memory();
    x64_init_virtual_memory();
    x64_init_gdt();
}