#ifndef __INCLUDE_X64_MEMORY_H__
#define __INCLUDE_X64_MEMORY_H__

#define ARCH_PAGE_SIZE 4096

struct frame_marker {
    struct frame_marker *next;
    struct frame_marker *prev;
    unsigned int cons; // Number of consecutive free frames after this
};

#endif /* __INCLUDE_X64_MEMORY_H__ */