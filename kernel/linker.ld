OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

PHDRS
{
    text    PT_LOAD     FLAGS((1 << 0) | (1 << 2));
    rodata  PT_LOAD     FLAGS((1 << 2));
    data    PT_LOAD     FLAGS((1 << 1) | (1 << 2));
}

SECTIONS
{
    . = 0xffffffff80000000;

    _FORS_KERNEL_START = .;
    _FORS_KERNEL_CODE_START = .;
    .text : ALIGN(4096) {
        *(.text .text.*)
    } :text
    _FORS_KERNEL_CODE_END = .;
    _FORS_KERNEL_RO_START = .;

    .rodata : ALIGN(4096) {
        *(.rodata .rodata.*)
    } :rodata
    _FORS_KERNEL_RO_END = .;
    _FORS_KERNEL_RW_START = .;
    .data : ALIGN(4096) {
        *(.data .data.*)
    } :data

    .bss : ALIGN(4096) {
        *(COMMON)
        *(.bss .bss.*)
    } :data
    _FORS_KERNEL_RW_END = .;
    _FORS_KERNEL_END = .;

    _FORS_KERNEL_TEMP_MAP_PAGE = ALIGN(4096);
    . += 4096; /* Set aside an aligned page for temporary kernel mappings. */

    _FORS_HEAP_START = ALIGN(65536);
}