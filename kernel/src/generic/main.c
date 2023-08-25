#include "fors/printk.h"
#include "limine.h"

#include <stdint.h>
#include <stddef.h>

#include "fors/init.h"
#include "fors/memory.h"

void _start(void) {
    arch_early_setup();
    arch_init_memory();
    
    for (;;) {
        __asm__("hlt");
    }
}