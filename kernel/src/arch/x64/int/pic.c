#include "arch/x64/pic.h"
#include "arch/x64/io.h"
#include "fors/printk.h"

#define SMALL_WAIT outb(0x80, 0)

// Flags for ICW1
#define USE_ICW4   (1 << 0)
#define IS_SINGLE  (1 << 1)
#define INTERVAL_4 (1 << 2)
#define LEVELTRIG  (1 << 3) // If not set, then edge trigger mode is used
#define INIT       (1 << 4) // This is required, always set

// Flags for ICW4
#define MODE_8086   (1 << 0)
#define AUTO_EOI    (1 << 1)
#define BUFF_SLAVE  (0b1000) // ha ha funny names
#define BUFF_MASTER (0b1100)
#define SFNM        (1 << 4)

void pic_map(uint8_t offset_master, uint8_t offset_slave)
{
    outb(PIC_MASTER_CMD, INIT | USE_ICW4);
    SMALL_WAIT;
    outb(PIC_SLAVE_CMD, INIT | USE_ICW4);
    SMALL_WAIT;
    outb(PIC_MASTER_DAT, offset_master);
    SMALL_WAIT;
    outb(PIC_SLAVE_DAT, offset_slave);
    SMALL_WAIT;
    outb(PIC_MASTER_DAT, 1 << 2);
    SMALL_WAIT;
    outb(PIC_SLAVE_DAT, 2);
    SMALL_WAIT;
    outb(PIC_MASTER_DAT, MODE_8086);
    SMALL_WAIT;
    outb(PIC_MASTER_DAT, MODE_8086);
    SMALL_WAIT;

    // Right now disable all IRQs, except IRQ2 (so the slave can raise IRQs if they're
    // enabled)
    outb(PIC_MASTER_DAT, 0xfb);
    outb(PIC_SLAVE_DAT, 0xff);

    printk("Enabling interrupts.\n");

    __asm__("sti");
}

void pic_block_irq(uint8_t irq)
{
    uint16_t port;

    if (irq < 8) {
        port = PIC_MASTER_DAT;
    } else {
        port = PIC_SLAVE_DAT;
        irq -= 8;
    }

    outb(port, inb(port) | (1 << irq));
}

void pic_unblock_irq /*<3*/ (uint8_t irq)
{
    uint16_t port;

    if (irq < 8) {
        port = PIC_MASTER_DAT;
    } else {
        port = PIC_SLAVE_DAT;
        irq -= 8;
    }

    outb(port, inb(port) & ~(1 << irq));
}

void pic_eoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC_SLAVE_CMD, 0x20);
    }

    outb(PIC_MASTER_CMD, 0x20);
}