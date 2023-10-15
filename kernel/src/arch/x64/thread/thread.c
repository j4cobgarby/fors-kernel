#include "fors/thread.h"
#include "fors/memory.h"
#include "arch/x64/cpu.h"
#include "arch/x64/memory.h"
#include "forslib/string.h" // IWYU pragma: keep: clangd keeps thinking this header is not used without this pragma

int mkthread(char *name, void (*entry)(void *), void *arg, void *stack, bool user) {
    thread *th;
    long tid = find_free_tid();

    if (tid < 0) {
        return -1;
    } else {
        th = &threads[tid];
        th->tid = tid;
        th->present = true;
        th->status = THR_READY;
        strncpy(th->name, name, THREAD_NAME_LENGTH);

        if (user) {
            th->ctx.cs = USER_CS;
            th->ctx.ss = USER_SS;
        } else {
            th->ctx.cs = KERNEL_CS;
            th->ctx.ss = KERNEL_SS;
        }
        th->ctx.rflags = RFLAGS_BASE | RFLAGS_IF;

        if (stack) {
            th->ctx.rsp = (uint64_t)stack;
        } else {
            th->ctx.rsp = (uint64_t)allocate_stack();
        }

        th->ctx.rip = (uint64_t)entry;
        th->ctx.rdi = (uint64_t)arg;

        th->ctx.rbp = 0;

        return tid;
    }
}