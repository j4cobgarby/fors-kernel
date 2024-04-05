#include "fors/process.h"
#include "fors/panic.h"
#include "fors/timer.h"
#include "arch/x64/pic.h"
#include "fors/memory.h"

#include "arch/x64/cpu.h"
#include "arch/x64/memory.h"
#include "arch/x64/process_arch.h"

#include "fors/printk.h"
#include "forslib/string.h" // IWYU pragma: keep: clangd keeps thinking this header is not used without this pragma
#include "limine.h"

pml4_entry_t *new_blank_user_pml4()
{
    pml4_entry_t *new_pml4 = pfalloc_one();

    for (int i = memmap_req.response->entry_count - 1; i >= 0; i--) {
        struct limine_memmap_entry *ent = memmap_req.response->entries[i];

        if (ent->type != LIMINE_MEMMAP_RESERVED) {
            for (size_t off = 0; off < ent->length; off += ARCH_PAGE_SIZE) {
                if (map_page_4k(new_pml4, ent->base + off,
                        hhdm_request.response->offset + off,
                        PSE_PRESENT | PSE_WRITABLE)
                    < 0) {
                    KPANIC("Failed mapping.\n");
                }
            }
        }
    }

    size_t fors_load = kernel_address_request.response->virtual_base;
    size_t fors_phys = kernel_address_request.response->physical_base;

    /* TODO: Verify correctness of following for-loops.
     * At one point, to fix an error where sometimes certain pieces of data
     * wouldn't be mapped (most importantly, the TSS wasn't getting mapped in
     * user tables leading to crashes when doing syscalls), I now add
     * ARCH_PAGE_SIZE to upper bound of these memory ranges. In theory this
     * shouldn't be strictly necessary if instead we "properly" set the starts
     * and ends to actual page boundaries. */

    for (size_t off = FORS_CODE_OFFSET;
         off < FORS_CODE_END_OFFSET + ARCH_PAGE_SIZE; off += ARCH_PAGE_SIZE) {
        if (map_page_4k(new_pml4, fors_phys + off, fors_load + off, PSE_PRESENT)
            < 0) {
            KPANIC("Failed mapping.\n");
        }
    }

    for (size_t off = FORS_RO_OFFSET; off < FORS_RO_END_OFFSET + ARCH_PAGE_SIZE;
         off += ARCH_PAGE_SIZE) {
        if (map_page_4k(
                new_pml4, fors_phys + off, fors_load + off, PSE_PRESENT)) {
            KPANIC("Failed mapping.\n");
        }
    }

    for (size_t off = FORS_RW_OFFSET; off < FORS_RW_END_OFFSET + ARCH_PAGE_SIZE;
         off += ARCH_PAGE_SIZE) {
        if (map_page_4k(new_pml4, fors_phys + off, fors_load + off,
                PSE_PRESENT | PSE_WRITABLE)) {
            KPANIC("Failed mapping.\n");
        }
    }

    uint64_t kheap_start = (uint64_t)&_FORS_HEAP_START;
    uint64_t kheap_phys;
    int suc = map_lookup(kernel_pml4_table, kheap_start, &kheap_phys);

    if (suc < 0) {
        KPANIC("Failed to lookup kernel mapping of kernel heap.\n");
    }

    /* TODO: It's pretty messy that the heap has to be explicitly mapped in
     * here. Wouldn't it just be better to copy over all page mappings that
     * the kernel has, when creating a process? Or even better, do on demand
     * mapping where kernel pages are mapped in only when accessed */
    map_page_4k(new_pml4, kheap_phys, kheap_start, PSE_PRESENT | PSE_WRITABLE);

    return new_pml4;
}

long create_process(
    char *name, void (*entry)(void *), void *arg, void *stack, bool user)
{
    process *th;
    long tid = find_free_tid();

    if (tid < 0) {
        return -1;
    } else {
        th = &procs[tid];

        th->tid = tid;
        th->present = true;
        th->status = PROC_READY;
        strncpy(th->name, name, PROC_NAME_LENGTH);

        if (user) {
            th->ctx.cs = USER_CS;
            th->ctx.ss = USER_SS;
            th->ctx.cr3 = (uint64_t)new_blank_user_pml4();
        } else {
            th->ctx.cs = KERNEL_CS;
            th->ctx.ss = KERNEL_SS;
            th->ctx.cr3 = (uint64_t)kernel_pml4_table;
        }
        th->ctx.rflags = RFLAGS_BASE | RFLAGS_IF;

        if (stack) {
            th->ctx.rsp = (uint64_t)stack;
        } else {
            th->ctx.rsp = (uint64_t)allocate_stack();
        }

        th->ctx.rip = (uint64_t)entry;
        th->ctx.rdi = (uint64_t)arg;

        th->ctx.rbp = 0;

        return tid;
    }
}

void schedule_set_current_proc()
{
    current_proc = schedule();
}

void arch_start_running_procs()
{
    // Invoke the scheduler every 10 ticks
    add_timer_handle(&schedule_set_current_proc, 1);
    pic_unblock_irq(0); // Start timer.
}
