#include "fors/timer.h"
#include "fors/printk.h"
#include "fors/thread.h"

#define MAX_TIMER_HANDLES 8

unsigned long long ticks = 0;

static timer_handle handles[MAX_TIMER_HANDLES];
static int next_handle_ind = 0;

int add_timer_handle(void (*cbk)(), unsigned long long n)
{
    if (next_handle_ind >= MAX_TIMER_HANDLES) return -1;

    handles[next_handle_ind] = (timer_handle) {
        .cbk = cbk,
        .n = n,
    };

    return next_handle_ind++;
}

void timer_tick()
{
    ticks++;
    printk("Timer tick %d\n", ticks);

    for (int i = 0; i < next_handle_ind; i++) {
        if (ticks % handles[i].n == 0) handles[i].cbk();
    }
}
