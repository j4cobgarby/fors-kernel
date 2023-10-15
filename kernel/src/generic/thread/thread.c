#include "fors/thread.h"
#include <stddef.h>

thread threads[MAX_THREADS];

long find_free_tid() {
    for (size_t i = 0; i < MAX_THREADS; i++) {
        if (!threads[i].present) return i;
    }
    return -1;
}