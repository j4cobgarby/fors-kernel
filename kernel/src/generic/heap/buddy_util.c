#include "fors/kheap.h"

// Remove a block from whichever list it's in
// Before:  ... A <-> bl <-> B ...
// After:   ... A <--------> B ...
void *remove_block(buddy_allocator *alloc, buddy_block *bl) {
    buddy_block *left = bl->prev, *right = bl->next;

    if (left) left->next = right;
    if (right) right->prev = left;

    if (!left) alloc->order_lists[bl->order - alloc->min_order] = right;

    bl->taken = 1; // Blocks are removed from the list when they are taken

    return bl;
}

// Insert a block at the head of its correct list
// Before:  NULL <--------> A (head) ...
// After:   NULL <-> bl (head) <-> A ...
void insert_block(buddy_allocator *alloc, buddy_block *bl) {
    buddy_block *head = alloc->order_lists[bl->order - alloc->min_order];

    bl->prev = NULL;
    bl->next = head;
    bl->taken = 0; // Blocks in the list are free ones

    if (head) head->prev = bl;
    
    alloc->order_lists[bl->order - alloc->min_order] = bl;
}

int buddy_init(size_t size, void *ptr, size_t min_order, buddy_allocator *alloc) {
    if ((size & (size-1)) != 0) {
        return -1;
    }

    buddy_block *root_block = ptr;

    if (!root_block) {
        return -1;
    }

    for (int i = 0; i < BUDDY_NUM_LISTS; i++) {
        alloc->order_lists[i] = NULL;
    }

    alloc->min_order = min_order;
    alloc->max_order = __builtin_ctz(size);
    alloc->size = size;
    alloc->memory_start = root_block;

    root_block->order = alloc->max_order;
    insert_block(alloc, root_block);

    return 0;
}

buddy_block *get_buddy(buddy_allocator *alloc, buddy_block *bl) {
    if (bl->order == alloc->max_order) return NULL;

    size_t mask = 1 << bl->order;
    buddy_block *buddy = (buddy_block *)((size_t)bl ^ mask);

    return buddy;
}

buddy_block *split(buddy_allocator *alloc, buddy_block *bl) {
    buddy_block *br;

    if (bl->order - 1 < alloc->min_order) {
        return NULL; // Splitting block too small
    }

    remove_block(alloc, bl);

    bl->order--;

    br = get_buddy(alloc, bl);
    br->order = bl->order;

    insert_block(alloc, bl);
    insert_block(alloc, br);

    return br;
}

buddy_block *merge(buddy_allocator *alloc, buddy_block *bl) {
    buddy_block *left_buddy;

    if (bl->order == alloc->max_order) return NULL;

    left_buddy = (buddy_block *)((size_t)bl & ~(1 << bl->order)); // Clear bit that separates the two buddies from each other

    if (left_buddy->order != get_buddy(alloc, left_buddy)->order) return NULL;

    remove_block(alloc, left_buddy);
    remove_block(alloc, get_buddy(alloc, left_buddy));

    left_buddy->order++;

    insert_block(alloc, left_buddy);

    return left_buddy;
}