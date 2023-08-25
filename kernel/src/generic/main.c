#include "fors/printk.h"
#include "limine.h"

#include <stdint.h>
#include <stddef.h>

#include "fors/init.h"
#include "fors/memory.h"

void _start(void) {
    arch_early_setup();
    arch_init_memory();
    

    printctrl_set(PRINTCTRL_RADIX_PREFIX);

    void *f1 = pfalloc_one();
    void *f2 = pfalloc_one();
    void *f3 = pfalloc_one();

    pffree_one(f1);
    pffree_one(f2);
    pffree_one(f3);

    f1 = pfalloc_consecutive(10);
    f2 = pfalloc_consecutive(20);

    pffree_consecutive(f1, 10);
    pffree_consecutive(f2, 20);
    
    for (;;) {
        __asm__("hlt");
    }
}