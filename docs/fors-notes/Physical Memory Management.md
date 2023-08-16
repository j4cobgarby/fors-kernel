## Theory

At boot, the [[Bootloader]] will tell the kernel about different regions of physical memory that are available. From all of the [[Page Frames]] available for us to use, Fors will create a linked list. This will be constructed from a `page_frame_list_entry` structure at the beginning of each of these frames, which will contain a field for a (physical memory) pointer to the next frame in the list.

In this way, allocating a page frame is just removing the first frame from the linked list, and freeing one is just returning it to the back. This is quite efficient in terms of memory usage (O(0) basically, because the only memory used to represent the linked list resides in the unused frames, and so it's not taking up any memory that could be used otherwise). Allocating single frames is very efficient too (O(1)), and allocating multiple non-consecutive blocks is as simple as multiple single allocations. If consecutive allocations are required, this allocation algorithm becomes a lot less efficient, because in the worst case the entire linked list (of length k) has to be searched through n times, where n is the amount of consecutive frames requested. This results in a worst case complexity of O(k * n).

In the future, this could be improved in a few ways. One option is, at initialisation, taking notice of some sequences of consecutive blocks and storing them in a number of separate linked lists (say 10 linked lists of runs of 16 blocks). The number of blocks in each run could be a function of the total memory available on the system.

Another option would be to change the block freeing algorithm to insert them back into the linked list maintaining ascending order of physical memory address. In this case the worst case would still be O(k * n) (since it may be that there are no consecutive runs of frames until the very end of the list). This could be improved further by storing an extra piece of metadata in each entry in the page frame linked list counting the amount of free consecutive frames after and including it, called *cons*, for instance. This could be iteratively updated each time a new frame is added to the list by simply counting back until a gap between frames is found, and incrementing each frame's *cons* value. Removing a frame from the list may also modify this value for frames coming before it, so those should be recalculated. I'm yet to work out if this algorithm would be faster than not doing this.

## Functions

The kernel will provide some functions for dealing with physical memory (page _frame_) allocation. Most of these functions return a status integer, negative if an error occurred or 0 on success. Possible errors for these functions are (in addition to those mentioned in [[Error Codes]]):

 - (-5) Out of free page frames.
 - (-6) Cannot find suitable consecutive page frames.

### Allocating Page Frames
 - `void* pfalloc_one()` - Finds and reserves one free page frame (of size `PAGE_FRAME_SIZE` bytes), and returns its **physical** address. If a valid frame cannot be found (e.g. the system is out of free memory) then `NULL (0)` is returned.
 - `int pfalloc(size_t n, void **frames)` - Finds `n` free page frames (each of size PAGE_FRAME_SIZE bytes), and copies their **physical** addresses into the array of void pointers at `frames`. Reserves these page frames so they cannot be allocated by a further call to this function. The page frames are not guaranteed to be consecutive.
 - `int pfalloc_consecutive(size_t n, void **frames)` - Exactly like `pfalloc`, except that the page frames are guaranteed consecutive. `frames` will contain their physical addresses in ascending order.

### Freeing Page Frames
 - `int pffree_one(void *frame)` - Frees the page frame at the **physical** address `frame`.
 - `int pffree(size_t n, void **frames)` - Frees all of the page frames in the list `frames` of **physical** addresses.