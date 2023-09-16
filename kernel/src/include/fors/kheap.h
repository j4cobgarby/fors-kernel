#ifndef __INCLUDE_FORS_KHEAP_H__
#define __INCLUDE_FORS_KHEAP_H__

#include <stddef.h>
#include <stdint.h>

#define BUDDY_MIN_BLOCK 256

void kheap_reinitialise(void *start, size_t size);

void *kalloc(size_t size);
void kfree(void *ptr);

#endif /* __INCLUDE_FORS_KHEAP_H__ */