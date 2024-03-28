#include "fors/process.h"
#include <stddef.h>
#include <stdbool.h>

process procs[MAX_PROCS];
long sched_queue[MAX_PROCS];
long current_proc = -1;

static size_t sq_head = 0;
static size_t sq_tail = 0;
static size_t sq_space_left = MAX_PROCS; // Space remaining in queue

static bool sq_full()
{
    return sq_space_left == 0;
}

static bool sq_empty()
{
    return sq_space_left == MAX_PROCS;
}

long find_free_tid()
{
    for (size_t i = 0; i < MAX_PROCS; i++) {
        if (!procs[i].present) return i;
    }
    return -1;
}

long schedule()
{
    long to_run = dequeue_proc();
    enqueue_proc(to_run);
    return to_run;
}

int enqueue_proc(long tid)
{
    if (sq_full()) {
        return -1;
    }

    sched_queue[sq_tail] = tid;
    sq_tail = (sq_tail + 1) % MAX_PROCS;

    sq_space_left--;

    return 0;
}

long dequeue_proc()
{
    if (sq_empty()) {
        return -1;
    }

    long ret = sched_queue[sq_head];
    sq_head = (sq_head + 1) % MAX_PROCS;

    sq_space_left++;

    return ret;
}
