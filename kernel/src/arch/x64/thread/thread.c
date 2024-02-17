#include "fors/thread.h"
#include "fors/timer.h"
#include "arch/x64/pic.h"
#include "fors/memory.h"

#include "arch/x64/cpu.h"
#include "arch/x64/memory.h"
#include "arch/x64/thread_arch.h"

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
                map_page_4k(new_pml4, ent->base + off,
                    hhdm_request.response->offset + off, PSE_PRESENT | PSE_WRITABLE);
            }
        }
    }

    size_t fors_load = kernel_address_request.response->virtual_base;
    size_t fors_phys = kernel_address_request.response->physical_base;

    for (size_t off = FORS_CODE_OFFSET; off < FORS_CODE_END_OFFSET;
         off += ARCH_PAGE_SIZE) {
        map_page_4k(new_pml4, fors_phys + off, fors_load + off, PSE_PRESENT);
    }

    for (size_t off = FORS_RO_OFFSET; off < FORS_RO_END_OFFSET; off += ARCH_PAGE_SIZE) {
        map_page_4k(new_pml4, fors_phys + off, fors_load + off, PSE_PRESENT);
    }

    for (size_t off = FORS_RW_OFFSET; off < FORS_RW_END_OFFSET; off += ARCH_PAGE_SIZE) {
        map_page_4k(
            new_pml4, fors_phys + off, fors_load + off, PSE_PRESENT | PSE_WRITABLE);
    }

    return new_pml4;
}

long mkthread(char *name, void (*entry)(void *), void *arg, void *stack, bool user)
{
    thread *th;
    long tid = find_free_tid();

    if (tid < 0) {
        return -1;
    } else {
        th = &threads[tid];

        memset(th, 0, sizeof(thread));

        th->tid = tid;
        th->present = true;
        th->status = THR_READY;
        strncpy(th->name, name, THREAD_NAME_LENGTH);

        if (user) {
            th->ctx.cs = USER_CS;
            th->ctx.ss = USER_SS;
            th->ctx.cr3 = (uint64_t)new_blank_user_pml4();
            printk("Made new PML4 for thread %d at %p\n", tid, th->ctx.cr3);
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

void schedule_set_current_thread()
{
    current_thread = schedule();
}

void arch_start_running_threads()
{
    // Invoke the scheduler every 10 ticks
    add_timer_handle(&schedule_set_current_thread, 10);
    pic_unblock_irq(0); // Start timer.
}
