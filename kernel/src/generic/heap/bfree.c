#include "fors/kheap.h"

void bfree(void *ptr, buddy_allocator *alloc) {

    if (!ptr) return;

    buddy_block *bl = HEADER(ptr);
    buddy_block *buddy = get_buddy(alloc, bl);

    if (!buddy) {
        return;
    } else {
    }

    if (buddy->order != bl->order || buddy->taken) {
        insert_block(alloc, bl);
    } else {
        // Iteratively merge as much as possible
        while (bl->order != alloc->max_order && !get_buddy(alloc, bl)->taken) {
            bl = merge(alloc, bl);
            if (!bl) { // Merge bl with buddy
                return;
            }
        }
    }     
}