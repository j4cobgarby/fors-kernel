#include "arch/x64/memory.h"
#include "fors/printk.h"

struct gdt_descriptor_long gdt_table[] = {
    INIT_SEG_DESCRIPTOR(0l, 0l, 0, 0), // Mandatory null descriptor

    // Kernel code
    INIT_SEG_DESCRIPTOR(0l, 0l, 
          SEG_AB_PRESENT 
        | SEG_AB_DPL(0) 
        | SEG_AB_CODE_DATA 
        | SEG_AB_EXEC 
        | SEG_AB_RW,

          SEG_FLAG_LONG 
        | SEG_FLAG_4K_BLOCKS),

    // Kernel data
    INIT_SEG_DESCRIPTOR(0l, 0l, 
          SEG_AB_PRESENT 
        | SEG_AB_DPL(0) 
        | SEG_AB_CODE_DATA 
        | SEG_AB_RW,
        
          SEG_FLAG_4K_BLOCKS),

    INIT_SEG_DESCRIPTOR(0l, 0l, 
        SEG_AB_PRESENT | SEG_AB_DPL(3) | SEG_AB_CODE_DATA | SEG_AB_EXEC | SEG_AB_RW, 
        SEG_FLAG_LONG | SEG_FLAG_4K_BLOCKS),

    INIT_SEG_DESCRIPTOR(0l, 0l, 
        SEG_AB_PRESENT | SEG_AB_DPL(3) | SEG_AB_CODE_DATA | SEG_AB_RW,
        SEG_FLAG_4K_BLOCKS),
};

struct gdtr_image gdtr;

void x64_init_gdt() {
    gdtr.base = gdt_table;
    gdtr.limit = sizeof(gdt_table);

    load_gdt();
    reload_seg_registers();
}