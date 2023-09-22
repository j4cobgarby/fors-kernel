#ifndef __INCLUDE_MEMORY_H__
#define __INCLUDE_MEMORY_H__

#include <stddef.h>
#include <stdint.h>

void arch_init_memory();

/* Physical memory functions */

void *pfalloc_one();
void *pfalloc_consecutive(unsigned int n);

void pffree_one(void *pf);
void pffree_consecutive(void *pf_first, unsigned int n);

/* Virtual memory functions */

int vmap(int pid, void *pa, void *va, int size, int flags);
int vunmap(int pid, void *va, int size);

int vis_mapped_to(int pid, void *va, void *pa);
void *vget_mapping(int pid, void *va, void *attr_out);

/* Finds a free region of virtual memory large enough to fit `size` bytes.
    `pid`: Determines the memory manager to use. -1 for kernel, >=0 for some process.
    `size`: Amount of bytes wanted. This will be rounded up to page boundaries.
    `flags`: Allocation flags. Refer to VAF_*
    `align`: Alignment of start of virtual region, or 0 if alignment not required
    return value: The start of the virtual address region, or NULL if failed. */
void *valloc(int pid, int size, unsigned int flags, size_t align);

// Virtual address flags
#define VMAP_NONE 0
#define VMAP_EXEC 1 << 0
#define VMAP_USER 1 << 1
#define VMAP_WRIT 1 << 2
#define VMAP_4K   1 << 3
#define VMAP_2M   1 << 4
#define VMAP_1G   1 << 5

typedef struct memory_region {
    uintptr_t base;
    size_t size;

    int flags;
} memory_region;

typedef struct region_manager {
    memory_region *first;
    size_t regions_count;

    int pid;
} region_manager;

#endif /* __INCLUDE_MEMORY_H__ */