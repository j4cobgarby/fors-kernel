#ifndef __INCLUDE_MEMORY_H__
#define __INCLUDE_MEMORY_H__

/* Physical memory functions */

void *pfalloc_one();
void *pfalloc_consecutive(int n);

void pffree_one(void *pf);
void pffree_consecutive(void *pf_first, int n);

/* Virtual memory functions */

int vmap(int pid, void *pa, void *va, int size);
int vunmap(int pid, void *va, int size);

int vis_mapped_to(int pid, void *va, void *pa);
void *vget_mapping(int pid, void *va, void *attr_out);

#endif /* __INCLUDE_MEMORY_H__ */