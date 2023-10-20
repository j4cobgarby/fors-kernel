#include "fors/eventqueue.h"

#include <stddef.h>

#define EVENTQUEUE_SIZE 512

size_t evq_head = 0;
size_t evq_tail = 0;

size_t evq_remaining = EVENTQUEUE_SIZE; // Remaining space in circular queue

system_event event_queue[EVENTQUEUE_SIZE];

int eventqueue_init() {
    return 0;
}

int evqfull() {
    return evq_remaining == 0;
}

int evqempty() {
    return evq_head == evq_tail;
}

int evqpush(system_event ev) {
    if (!evqfull()) {
        evq_remaining--;
        event_queue[evq_tail++] = ev;
        if (evq_tail >= EVENTQUEUE_SIZE) evq_tail = 0;
    }
    return evq_remaining; // Returns 0 on full queue
}

int evqpop(system_event *ev) {
    int ret = -1;
    if (!evqempty()) {
        *ev = event_queue[evq_head];
        ret = 0;

        evq_remaining++;
        evq_head++;
        if (evq_head >= EVENTQUEUE_SIZE) evq_head = 0;
    }
    return ret;
}