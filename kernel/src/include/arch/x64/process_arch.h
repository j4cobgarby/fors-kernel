#ifndef __INCLUDE_ARCH_X64_PROCESS_ARCH_H__
#define __INCLUDE_ARCH_X64_PROCESS_ARCH_H__

#include "arch/x64/memory.h"

const void *user_base = (void *)0x200000000;

const void *user_stack_top_page
    = (void *)((uint64_t)&_FORS_KERNEL_START - ARCH_PAGE_SIZE);

pml4_entry_t *new_blank_user_pml4();

#endif /* __INCLUDE_ARCH_X64_PROCESS_ARCH_H__ */
