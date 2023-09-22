#include "arch/x64/idt.h"

#include "fors/printk.h"

INTERRUPT_HANDLER isr_DE(struct isr_frame *frame){
    ERROR("DE");
}

INTERRUPT_HANDLER isr_NMI(struct isr_frame *frame){
    ERROR("NMI");
}

INTERRUPT_HANDLER isr_BP(struct isr_frame *frame){
    ERROR("BP");
}

INTERRUPT_HANDLER isr_OF(struct isr_frame *frame){
    ERROR("OF");
}

INTERRUPT_HANDLER isr_BR(struct isr_frame *frame){
    ERROR("BR");
}

INTERRUPT_HANDLER isr_UD(struct isr_frame *frame){
    ERROR("UD");
}

INTERRUPT_HANDLER isr_DF(struct isr_frame *frame, uint64_t error_code){
    ERROR("DF");
}

INTERRUPT_HANDLER isr_TS(struct isr_frame *frame, uint64_t error_code){
    ERROR("TS");
}

INTERRUPT_HANDLER isr_NP(struct isr_frame *frame, uint64_t error_code){
    ERROR("NP");
}

INTERRUPT_HANDLER isr_SS(struct isr_frame *frame, uint64_t error_code){
    ERROR("SS");
}

INTERRUPT_HANDLER isr_GP(struct isr_frame *frame, uint64_t error_code){
    printk("#GP w/ error code %d\n", error_code);
    printk("\tRIP = %x\n", frame->ip);
    __asm__ ("hlt");
    
}

INTERRUPT_HANDLER isr_PF(struct isr_frame *frame, uint64_t error_code){
    printk("#PF w/ error code %d\n", error_code);
    __asm__ ("hlt");
}
