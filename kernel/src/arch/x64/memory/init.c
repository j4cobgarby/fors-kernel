#include "fors/memory.h"
#include "arch/x64/memory.h"
#include "arch/x64/uart.h"

void arch_init_memory() {
    x64_init_physical_memory();
    x64_init_virtual_memory();
    x64_init_gdt();
}