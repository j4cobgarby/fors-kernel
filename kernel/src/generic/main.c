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

    printk("Allocated frames:\n%x\n%x\n%x\n", f1, f2, f3);

    pffree_one(f1);
    pffree_one(f2);
    pffree_one(f3);

    printk("Freed 3 blocks.\n");

    f1 = pfalloc_consecutive(10);

    printk("Allocated 10 blocks starting at %x\n", f1);

    pffree_consecutive(f1, 10);

    printk("Freed 10 blocks.\n");
    
    for (;;) {
        __asm__("hlt");
    }
}