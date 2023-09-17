#include "fors/kheap.h"
#include "fors/printk.h"
#include <stddef.h>

typedef struct buddy_block {
    unsigned int flags;
    size_t size; // Total size of block including this header
    struct buddy_block *next;
    struct buddy_block *prev;
    char data_start[];
} buddy_block;

#define BUDDY_TAKEN (1 << 0)
#define BUDDY_LEFT  (1 << 1)
#define BUDDY_RIGHT (1 << 2)

#define BUDDY_MIN_ALLOC BUDDY_MIN_BLOCK - sizeof(buddy_block)

/* Virtual address of start of kernel heap */
buddy_block *kheap_start;

/* Size (in bytes) of kernel heap */
size_t kheap_size;

void kheap_reinitialise(void *start, size_t size) {
    kheap_size = size;
    kheap_start = start;

    *kheap_start = (buddy_block){
        .size = size,
        .next = NULL,
        .prev = NULL,
        .flags = 0,
    };
}

static void *buddy_split(buddy_block *bl) {
    buddy_block *l_block = bl;
    buddy_block *r_block = ((void*)l_block) + l_block->size/2;

    // Insert new block to right of current
    r_block->prev = l_block;
    r_block->next = l_block->next;
    if (r_block->next) r_block->next->prev = r_block;
    l_block->next = r_block;

    l_block->size /= 2;
    r_block->size = l_block->size;

    // Copy takenness of left block, although really this could just be set to
    // false because there's no case where an already taken block is split up.
    r_block->flags |= l_block->flags & BUDDY_TAKEN;

    l_block->flags |= BUDDY_LEFT;
    l_block->flags &= ~BUDDY_RIGHT;

    r_block->flags |= BUDDY_RIGHT;
    r_block->flags &= ~BUDDY_LEFT;

    return r_block;
}

void *kalloc(size_t size) {
    if (size < BUDDY_MIN_ALLOC) size = BUDDY_MIN_ALLOC;

    if (size <= 0) {
        return NULL;
    } else {
        size += sizeof(buddy_block); // The caller _really_ needs to allocate how much they want plus the size of the header
        size = size == 1 ? 1 : 1 << (64 - __builtin_clz(size-1)); // Round to next pow of 2
    }

    // By this point size is a power of 2

    buddy_block *closest_block = NULL;

    for (buddy_block *bl = kheap_start; bl; bl = bl->next) {
        if (!(bl->flags & BUDDY_TAKEN) && bl->size >= size) {
            if (bl->size == size) {
                bl->flags |= BUDDY_TAKEN;
                return bl->data_start; // No need to split if found precise size
            } else {
                if (!closest_block) 
                    closest_block = bl;
                else 
                    closest_block = bl->size < closest_block->size ? bl : closest_block;
            }
        }
    }

    int divs = __builtin_clz(size) - __builtin_clz(closest_block->size);

    for (int i = 0; i < divs; i++) {
        buddy_split(closest_block);
    }

    closest_block->flags |= BUDDY_TAKEN;
    return closest_block->data_start;
}


// x -> lb -> rb -> y
// ...
// x -> lb+rb -> y
static void buddy_merge(buddy_block *lb, buddy_block *rb) {
    while (true) {
        if (!lb || !rb) return;

        if (lb->size == rb->size && !(lb->flags & BUDDY_TAKEN) && !(rb->flags & BUDDY_TAKEN) /*&&
            lb->flags & BUDDY_LEFT && rb->flags & BUDDY_RIGHT*/) {

            if (rb->next) rb->next->prev = lb;
            lb->next = rb->next;
            lb->size *= 2;

            rb = lb->next;
        } else {
            return;
        }
    }
}

void kfree(void *ptr) {
    printk("Freeing %p\n", ptr);

    buddy_block *bl = ptr - offsetof(buddy_block, data_start);

    bl->flags &= ~BUDDY_TAKEN;

    if (bl->flags & BUDDY_LEFT) {
        buddy_merge(bl, bl->next);
    } else if (bl->flags & BUDDY_RIGHT) {
        buddy_merge(bl->prev, bl);
    } 
}