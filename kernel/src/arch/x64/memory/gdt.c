#include "arch/x64/memory.h"
#include "arch/x64/cpu.h"

gdt_table_entry gdt_table[] = {
    INIT_SEG_DESCRIPTOR(0l, 0l, 0, 0), // Mandatory null descriptor

    // Kernel code
    INIT_SEG_DESCRIPTOR(0l, 0l,
        SEG_AB_PRESENT | SEG_AB_DPL(0) | SEG_AB_CODE_DATA |
        SEG_AB_EXEC | SEG_AB_RW,
        SEG_FLAG_LONG | SEG_FLAG_4K_BLOCKS),

    // Kernel data
    INIT_SEG_DESCRIPTOR(
        0l, 0l, SEG_AB_PRESENT | SEG_AB_DPL(0) | SEG_AB_CODE_DATA | SEG_AB_RW,

        SEG_FLAG_4K_BLOCKS),

    INIT_SEG_DESCRIPTOR(0l, 0l,
        SEG_AB_PRESENT | SEG_AB_DPL(3) | SEG_AB_CODE_DATA |
        SEG_AB_EXEC | SEG_AB_RW,
        SEG_FLAG_LONG | SEG_FLAG_4K_BLOCKS),

    INIT_SEG_DESCRIPTOR(
        0l, 0l, SEG_AB_PRESENT | SEG_AB_DPL(3) | SEG_AB_CODE_DATA | SEG_AB_RW,
        SEG_FLAG_4K_BLOCKS),

    INIT_TSS_DESCRIPTOR_FIRST_HALF(0l, 0l),
    INIT_TSS_DESCRIPTOR_SECOND_HALF(0l),
};

struct gdtr_image gdtr;

void x64_init_gdt() {
    gdt_table[5] = INIT_TSS_DESCRIPTOR_FIRST_HALF((uint64_t)&tss, 0l);
    gdt_table[6] = INIT_TSS_DESCRIPTOR_SECOND_HALF((uint64_t)&tss);

    gdtr.base = gdt_table;
    gdtr.limit = sizeof(gdt_table);

    load_gdt();
    reload_seg_registers();
}