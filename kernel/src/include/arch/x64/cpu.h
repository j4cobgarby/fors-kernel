#ifndef __INCLUDE_X64_CPU_H__
#define __INCLUDE_X64_CPU_H__

#include <stdint.h>

typedef struct register_ctx_x64 {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t vector;
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) register_ctx_x64;

#define RFLAGS_BASE (1 << 1) // Necessary bit set
#define RFLAGS_IF   (1 << 9) // Interrupt enable
#define RFLAGS_IOPL(lvl) ((lvl & 0x3) << 12)

#endif /* __INCLUDE_X64_CPU_H__ */