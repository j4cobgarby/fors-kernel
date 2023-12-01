#ifndef __INCLUDE_FORS_TIMER_H__
#define __INCLUDE_FORS_TIMER_H__

extern unsigned long long ticks;

// A timer_handle is created by some code which
// wants a function to be called every 'n' ticks.
typedef struct timer_handle {
    void (*cbk)();
    unsigned long long n;
} timer_handle;

// Advance the system timer by one tick.
void timer_tick();

// Request that cbk is called every n ticks.
int add_timer_handle(void (*cbk)(), unsigned long long n);

#endif /* __INCLUDE_FORS_TIMER_H__ */
