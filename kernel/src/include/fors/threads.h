#ifndef __INCLUDE_FORS_THREADS_H__
#define __INCLUDE_FORS_THREADS_H__

typedef enum thread_status_t {
    RUNNING, READY, BLOCKED, KILLED
} thread_status_t;

typedef struct thread_t {
    thread_status_t status;
    char *name;

    void *platform; // Platform specific stuff
} thread_t;

#define MAX_THREADS 128

extern thread_t thread_table[MAX_THREADS];

#endif /* __INCLUDE_FORS_THREADS_H__ */