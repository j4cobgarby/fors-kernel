#ifndef __INCLUDE_X64_CPU_H__
#define __INCLUDE_X64_CPU_H__

#include <stdint.h>

typedef struct tss_t {
    uint32_t _0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t _1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t _2;
    uint16_t _3;
    uint16_t io_bitmap_offset;
} __attribute__((packed)) tss_t;

extern tss_t tss;

typedef struct register_ctx_x64 {
    uint64_t cr3;

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

#define RFLAGS_BASE      (1 << 1) // Necessary bit set
#define RFLAGS_IF        (1 << 9) // Interrupt enable
#define RFLAGS_IOPL(lvl) ((lvl & 0x3) << 12)

#define REGDUMP(ctx)                                                                     \
    printk("\
cr3\t%p\n\
r15\t%p\n\
r14\t%p\n\
r13\t%p\n\
r12\t%p\n\
r11\t%p\n\
r10\t%p\n\
r9\t%p\n\
r8\t%p\n\
rbp\t%p\n\
rdi\t%p\n\
rsi\t%p\n\
rdx\t%p\n\
rcx\t%p\n\
rbx\t%p\n\
rax\t%p\n\
vector\t%p\n\
error_code\t%p\n\
rip\t%p\n\
cs\t%p\n\
rflags\t%p\n\
rsp\t%p\n\
ss\t%p\n",                                                                               \
        ctx->cr3, ctx->r15, ctx->r14, ctx->r13, ctx->r12, ctx->r11, ctx->r10, ctx->r9,   \
        ctx->r8, ctx->rbp, ctx->rdi, ctx->rsi, ctx->rdx, ctx->rcx, ctx->rbx, ctx->rax,   \
        ctx->vector, ctx->error_code, ctx->rip, ctx->cs, ctx->rflags, ctx->rsp, ctx->ss)

#endif /* __INCLUDE_X64_CPU_H__ */