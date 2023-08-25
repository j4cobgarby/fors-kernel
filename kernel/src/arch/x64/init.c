#include "fors/init.h"
#include "arch/x64/uart.h"

void arch_early_setup() {
    if (x64_uart_init() < 0) {
        for (;;) __asm__ volatile("hlt");
    }
}