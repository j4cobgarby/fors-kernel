#ifndef __INCLUDE_FORS_THREAD_STRUCT_H__
#define __INCLUDE_FORS_THREAD_STRUCT_H__

#include <stdbool.h>

#ifdef __ARCH_x64__
#include "arch/x64/memory.h"
#include "arch/x64/cpu.h"
#endif

#define THREAD_NAME_LENGTH 32
#define MAX_THREADS 128

typedef enum thread_status {
    THR_RUNNING, THR_READY, THR_BLOCKED, THR_KILLED
} thread_status;

/* This struct should be the first element of the arch-specific thread_arch
 * struct.
 */
typedef struct thread {
    bool present; // Present in thread array?

    long tid; // Unique ID
    char name[THREAD_NAME_LENGTH]; // Not necessarily unique name string

    thread_status status;

#ifdef __ARCH_x64__
    register_ctx_x64 ctx;
#endif
} thread;

extern thread threads[MAX_THREADS];

// Finds a free TID, -1 if none available.
long find_free_tid();

int mkthread(char *name, void(*entry)(void*), void *arg, void *stack, bool user);

#endif /* __INCLUDE_FORS_THREAD_STRUCT_H__ */