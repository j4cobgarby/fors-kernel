#include "arch/x64/idt.h"
#include "fors/printk.h"


INTERRUPT_HANDLER isr_SYSCALL(struct isr_frame *frame) {
    printk("SYSCALL!\n");
}