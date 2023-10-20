#ifndef __INCLUDE_FORS_EVENTQUEUE_H__
#define __INCLUDE_FORS_EVENTQUEUE_H__

enum event_subsystem {
    SUB_KBD, // Generic type keyboard handler 
    SUB_THREAD, // Thread scheduler 
};

typedef struct system_event {
    enum event_subsystem subsys; // The subsystem that created the event
    int code;
    unsigned int data;
} system_event;

int eventqueue_init();

int evqfull();
int evqempty();

int evqpush(system_event ev);
int evqpop(system_event *ev);

#endif /* __INCLUDE_FORS_EVENTQUEUE_H__ */