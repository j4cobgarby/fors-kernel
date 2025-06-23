#ifndef __INCLUDE_FORS_PROCESS_H__
#define __INCLUDE_FORS_PROCESS_H__

#include <stdbool.h>

#ifdef __ARCH_x64__
#include "arch/x64/memory.h"
#include "arch/x64/cpu.h"
#endif

#include "fors/types.h"

#define PROC_NAME_LENGTH 32
#define MAX_PROCS        32
#define FDS_PER_PROC     32

typedef enum proc_status_e {
    PROC_RUNNING,
    PROC_READY,
    PROC_BLOCKED,
    PROC_KILLED
} proc_status;

typedef struct process_t {
    bool present; // Present in proc array?

    long tid; // Unique ID
    gid_t gid;
    uid_t uid;
    char name[PROC_NAME_LENGTH]; // Not necessarily unique name string
    char *cwd;                   // Current working directory

    proc_status status;

    fd_t fdmap[FDS_PER_PROC];

#ifdef __ARCH_x64__
    register_ctx_x64 ctx;
#endif
} process;

extern process procs[MAX_PROCS];
extern long current_proc;

// Finds a free TID, -1 if none available.
long find_free_tid();

long create_process(
    char *name, void (*entry)(void *), void *arg, void *stack, bool user);

int enqueue_proc(long tid);
long dequeue_proc();

long schedule();

void arch_start_running_procs();

#endif /* __INCLUDE_FORS_PROCESS_H__ */
