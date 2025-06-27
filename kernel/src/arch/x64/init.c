#include "fors/init.h"
#include "arch/x64/memory.h"
#include "arch/x64/uart.h"
#include "fors/printk.h"
#include "arch/x64/pic.h"

void arch_early_setup()
{
    if (x64_uart_init() < 0) {
        for (;;) __asm__ volatile("hlt");
    }
}

void arch_initialise()
{
    if (x64_uart_init() < 0) {
        for (;;) __asm__ volatile("hlt");
    }

    memory_initialise();

    pic_map(PIC_FIRST_VECTOR, PIC_FIRST_VECTOR + 8);
    pic_unblock_irq(PIC_IRQ_KEYBOARD);
}
