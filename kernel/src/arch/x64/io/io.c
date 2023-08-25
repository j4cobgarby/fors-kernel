#include "arch/x64/io.h"

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0"
                    : "=a"(ret)
                    : "Nd"(port));
    return ret;
}

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1"
                    :
                    : "a"(val), "Nd"(port));
}