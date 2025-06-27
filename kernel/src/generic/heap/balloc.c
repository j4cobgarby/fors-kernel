#include "fors/kheap.h"
#include "fors/printk.h"

void *balloc(size_t size, buddy_allocator *alloc)
{
    if (size > alloc->size) {
        printk("[balloc] Want more memory than total size.\n");
        return NULL;
        // TODO: Could allocate more memory for the buddy allocator here.
    }

    if (size <= 0) {
        printk("[balloc] Want negative size.\n");
        return NULL;
    }

    /* All allocation regions have to, in addition to their desired contents,
     * store a buddy block header structure */
    size += sizeof(buddy_block);

    // Round up to next power of 2
    size = size == 1 ? 1 : 1 << (64 - __builtin_clz(size - 1));

    if (size < 1U << alloc->min_order) size = 1U << alloc->min_order;

    // Log2 of a power of 2 is the amount of trailing 0's
    unsigned short order = __builtin_ctz(size);

    buddy_block *ret_block;

    if (alloc->order_lists[order - alloc->min_order]) {
        /* If we can find an available block of the exact size desired (so, the
         * minimum block size that fits the required bytes), then we can
         * immediately return this. */
        ret_block
            = remove_block(alloc, alloc->order_lists[order - alloc->min_order]);
        goto ret;
    } else {
        for (int n = order + 1; n <= alloc->max_order; n++) {
            ret_block = alloc->order_lists[n - alloc->min_order];

            /* If a block of this order (large than the target) exists, then we
             * need to split it iteratively until it's the minimum size that
             * still fits the required bytes */
            if (ret_block) {
                int splits = n - order;

                for (int split_n = 0; split_n < splits; split_n++) {
                    split(alloc, ret_block);
                }

                goto ret;
            }
        }

        /* At this point, no available blocks have been found of any suitable
         * order, so NULL will be returned. */
        printk("[balloc] No blocks found of any suitable order.\n");
        ret_block = NULL;
    }

ret:
    remove_block(alloc, ret_block);
    return &ret_block[1];
}
