#include "fors/memory.h"
#include "fors/process.h"
#include "arch/x64/memory.h"

int vunmap(int pid, void *va, int size)
{
    return -1;
}

int vis_mapped_to(int pid, void *va, void *pa)
{
    void *actual_pa = vget_mapping(pid, va, NULL);
    if (!actual_pa) return 0;
    return actual_pa == pa;
}

void *vget_mapping(int pid, void *va, void *attr_out)
{
    struct process_t *p = &procs[pid];
    if (!p->present) return NULL;

    pml4_entry_t *pml4_table = (pml4_entry_t *)p->ctx.cr3;
    uintptr_t phys_addr;

    if (map_lookup(pml4_table, (uintptr_t)va, &phys_addr) != 0) return NULL;
    return (void *)phys_addr;
}
