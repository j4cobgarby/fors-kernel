#ifndef __INCLUDE_FORS_KHEAP_H__
#define __INCLUDE_FORS_KHEAP_H__

#include <stdint.h>
#include <stddef.h>

// Total amount of memory supported is 2^(min_order + BUDDY_NUM_LISTS)
// If an allocator is initialised with min_order=5 (32 bytes min allocation block),
// then the total memory possible to be managed is 128 Gbytes.
#define BUDDY_NUM_LISTS 32 

typedef struct buddy_block {
    unsigned short int order;
    unsigned short int taken;

    struct buddy_block *next;
    struct buddy_block *prev;
} buddy_block;

typedef struct buddy_allocator {
    unsigned short int min_order;
    unsigned short int max_order;

    size_t size;
    void *memory_start;

    buddy_block *order_lists[BUDDY_NUM_LISTS];
} buddy_allocator;

extern buddy_allocator kheap_alloc;

#define DATA(blkptr) ((void*)(blkptr[1]))
#define HEADER(dataptr) ((buddy_block *)((void*)dataptr - sizeof(buddy_block)))

int buddy_init(size_t size, void *ptr, size_t min_order, buddy_allocator *alloc);

void *balloc(size_t size, buddy_allocator *alloc);
void bfree(void *ptr, buddy_allocator *alloc);

buddy_block *merge(buddy_allocator *alloc, buddy_block *bl);
buddy_block *split(buddy_allocator *alloc, buddy_block *bl);
buddy_block *get_buddy(buddy_allocator *alloc, buddy_block *bl);
void insert_block(buddy_allocator *alloc, buddy_block *bl);
void *remove_block(buddy_allocator *alloc, buddy_block *bl);

#endif /* __INCLUDE_FORS_KHEAP_H__ */