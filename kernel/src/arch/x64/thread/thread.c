#include "fors/thread.h"
#include "fors/memory.h"

int mkthread(char *name, void (*entry)(void *), void *arg) {
    thread *th;
    unsigned long tid = find_free_tid();

    if (tid < 0) {
        return -1;
    } else {
        th = &threads[tid];
        th->tid = tid;
        th->present = true;
        th->status = THR_READY;

        th->ctx.cs = 0x08;
        th->ctx.ss = 0x10;
        th->ctx.rflags = 0x202;

        th->ctx.rsp = (uint64_t)allocate_stack();
        th->ctx.rip = (uint64_t)entry;
        th->ctx.rdi = (uint64_t)arg;

        th->ctx.rbp = 0;

        return tid;
    }
}