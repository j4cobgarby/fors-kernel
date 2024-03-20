#include "fors/kheap.h"

void *balloc(size_t size, buddy_allocator *alloc)
{
    if (size > alloc->size) {
        return NULL;
        // TODO: Could allocate more memory for the buddy allocator here.
    }

    if (size <= 0) {
        return NULL;
    }

    size += sizeof(buddy_block);

    // Round up to next power of 2
    size = size == 1 ? 1 : 1 << (64 - __builtin_clz(size - 1));

    if (size < 1 << alloc->min_order) size = 1 << alloc->min_order;

    // Log2 of a power of 2 is the amount of trailing 0's
    unsigned short order = __builtin_ctz(size);

    buddy_block *ret_block;

    if (alloc->order_lists[order - alloc->min_order]) {
        ret_block
            = remove_block(alloc, alloc->order_lists[order - alloc->min_order]);
        goto ret;
    } else {
        for (int n = order + 1; n <= alloc->max_order; n++) {
            ret_block = alloc->order_lists[n - alloc->min_order];

            if (ret_block) {
                int splits = n - order;

                for (int s = 0; s < splits; s++) {
                    split(alloc, ret_block);
                }

                goto ret;
            }
        }

        ret_block = NULL;
    }

ret:
    remove_block(alloc, ret_block);
    return &ret_block[1];
}
