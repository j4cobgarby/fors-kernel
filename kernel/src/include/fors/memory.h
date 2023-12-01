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

void *tmpmap(void *pa); // Map a frame to the kernel's scratch page, for temporary access.

int vmap(int pid, void *pa, void *va, int size, int flags);
int vunmap(int pid, void *va, int size);

int vis_mapped_to(int pid, void *va, void *pa);
void *vget_mapping(int pid, void *va, void *attr_out);

void *allocate_stack();

// Virtual address flags
#define VMAP_NONE 0
#define VMAP_EXEC 1 << 0
#define VMAP_USER 1 << 1
#define VMAP_WRIT 1 << 2
#define VMAP_4K   1 << 3
#define VMAP_2M   1 << 4
#define VMAP_1G   1 << 5

#endif /* __INCLUDE_MEMORY_H__ */
