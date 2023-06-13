## Physical Memory
At boot, the bootloader (Limine probably) will tell the kernel about different regions of physical memory that are available.

The kernel will provide some functions for dealing with physical memory (page _frame_) allocation. All of these functions return a status integer, negative if an error occurred or 0 on success. Possible errors for these functions are:

 - -1: Generic error.
 - -2: Out of free page frames.
 - -3: Cannot find suitable consecutive page frames.

`int pfalloc(size_t n, void **frames)` - Finds `n` free page frames (each of size PAGE_FRAME_SIZE bytes), and copies their **physical** addresses into the array of void pointers at `frames`. The page frames are not guaranteed to be consecutive.

`int pfalloc_consecutive(size_t n, void **frames)` - Exactly like `pfalloc`, except that the page frames are guaranteed consecutive. `frames` will contain their physical addresses in ascending order.

`int pffree_one(void *frame)` - Frees the page frame at the **physical** address `frame`.

`int pffree(size_t n, void **frames)` - Frees all of the page frames in the list `frames` of **physical** addresses.