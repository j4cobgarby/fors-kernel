#ifndef __INCLUDE_ARCH_X64_PIC_H__
#define __INCLUDE_ARCH_X64_PIC_H__

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DAT 0x21
#define PIC_SLAVE_CMD  0xa0
#define PIC_SLAVE_DAT  0xa1

#define PIC_FIRST_VECTOR 0x20

#include <stdint.h>

void pic_map(uint8_t offset_master, uint8_t offset_slave);
void pic_block_irq(uint8_t irq);
void pic_unblock_irq(uint8_t irq);

void pic_eoi(uint8_t irq);

#endif /* __INCLUDE_ARCH_X64_PIC_H__ */