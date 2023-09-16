#include "fors/kheap.h"
#include <stddef.h>

typedef struct buddy_block {
    bool taken;
    size_t size; // Total size of block including this header
    struct buddy_block *next;
    char data_start[];
} buddy_block;

#define BUDDY_BLOCK_HEADER_SIZE offsetof(buddy_block, data_start)
#define BUDDY_MIN_ALLOC BUDDY_MIN_BLOCK - BUDDY_BLOCK_HEADER_SIZE

static_assert(sizeof(buddy_block) <= BUDDY_MIN_BLOCK, 
    "Buddy block header must fit within smallest block size");

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
        .taken = 0,
    };
}

static void *buddy_split(buddy_block *bl) {
    buddy_block *l_block = bl;
    buddy_block *r_block = ((void*)l_block) + l_block->size/2;

    r_block->next = l_block->next;
    l_block->next = r_block;

    l_block->size /= 2;
    r_block->size = l_block->size;

    // Copy takenness of left block, although really this could just be set to
    // false because there's no case where an already taken block is split up.
    r_block->taken = l_block->taken; 

    return r_block;
}

void *kalloc(size_t size) {
    if (size <= 0) {
        return NULL;
    } else {
        size += sizeof(buddy_block); // The caller _really_ needs to allocate how much they want plus the size of the header
        size = size == 1 ? 1 : 1 << (64 - __builtin_clz(size-1)); // Round to next pow of 2
    }

    // By this point size is a power of 2

    buddy_block *closest_block = kheap_start;

    for (buddy_block *bl = kheap_start; bl; bl = bl->next) {
        if (!bl->taken && bl->size >= size) {
            if (bl->size == size) {
                bl->taken = true;
                return bl->data_start; // No need to split if found precise size
            } else {
                closest_block = bl->size < closest_block->size ? bl : closest_block;
            }
        }
    }

    // If we haven't found the precise size block then closest_block needs to be split at least once to be size bytes exactly.

    int divs = __builtin_clz(size) - __builtin_clz(closest_block->size);

    for (int i = 0; i < divs; i++) {
        buddy_split(closest_block);
    }

    return closest_block->data_start;
}

void kfree(void *ptr) {

}