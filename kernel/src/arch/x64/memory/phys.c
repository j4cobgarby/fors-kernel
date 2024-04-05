#include "fors/panic.h"
#include "fors/printk.h"
#include "limine.h"
#include "arch/x64/memory.h"
#include "fors/memory.h"

#include <stdlib.h>

static struct frame_marker *frame_list = NULL;
static unsigned long int free_frames = 0;

static void update_cons(struct frame_marker *from)
{
    struct frame_marker *curr = from->prev;

    while (curr && curr->next == curr + ARCH_PAGE_SIZE) {
        curr = curr->prev;
    }
}

// Insert a page frame into the frame list, ordered by ascending address.
static void frame_insert(void *frame_ptr)
{
    struct frame_marker *prev = NULL, *to_ins = frame_ptr;

    if (!frame_list) {
        to_ins->prev = to_ins->next = NULL;
        frame_list = to_ins;

        free_frames++;

        return;
    }

    for (struct frame_marker *curr = frame_list; curr; curr = curr->next) {
        if (to_ins < curr) {
            to_ins->next = curr;
            curr->prev = to_ins;

            if (prev) { // If we're not inserting at the start of the list...
                to_ins->prev = prev;
                prev->next = to_ins;
            } else { // Or if we are...
                to_ins->prev = NULL;
                frame_list = to_ins;
            }

            free_frames++;
            return;
        } else if (curr->next == NULL) { // Insert at end of list
            curr->next = to_ins;

            to_ins->next = NULL;
            to_ins->prev = curr;

            free_frames++;
            return;
        } else {
            prev = curr;
        }
    }

    KPANIC("Somehow couldn't insert page frame.");
}

void x64_init_physical_memory()
{
#ifdef DEBUG_PRINT_LIMINE_MEMMAP
    static const char *const memtype_strs[] = {
        " * Usable           ",
        "   Reserved         ",
        "   ACPI Recl.       ",
        "   ACPI NVS         ",
        "   Bad              ",
        "   Bootloader Recl. ",
        "   Kernel + Modules ",
        "   Framebuffer      ",
    };
#endif

    struct limine_memmap_response *mm_resp = memmap_req.response;

    printk(") Memory map received from Limine\n");
    for (int i = mm_resp->entry_count - 1; i >= 0; i--) {
        struct limine_memmap_entry *ent = mm_resp->entries[i];

#ifdef DEBUG_PRINT_LIMINE_MEMMAP
        // printk(" >> [%s]: From %p ==> %p (%dK)\n", memtype_strs[ent->type],
        //     ent->base, ent->base + ent->length, ent->length / 1024);
#endif

        if (ent->type == LIMINE_MEMMAP_USABLE) {
            for (int byte = ent->length - ARCH_PAGE_SIZE; byte >= 0;
                 byte -= ARCH_PAGE_SIZE) {
                frame_insert((void *)ent->base + byte);
            }
        }
    }

    printk(") Page frame pool initialised with %d free %d-byte frames.\n",
        free_frames, ARCH_PAGE_SIZE);
}

void *pfalloc_one()
{
    if (!frame_list) return NULL;

    void *ret = (void *)frame_list;

    if (frame_list->next) frame_list->next->prev = NULL;
    frame_list = frame_list->next;

    free_frames--;

    return ret;
}

void *pfalloc_consecutive(unsigned int n)
{
    struct frame_marker *set_start = frame_list, *curr;
    unsigned int so_far;

    if (!frame_list) {
        return NULL;
    } else {
        curr = set_start->next;
        so_far = 1; // One known consecutive frame (the start of the list)
    }

    // Special case for allocating only one consecutive frame
    if (n == 1) {
        return pfalloc_one();
    }

    // Iterate until we find a suitable set or reach the end of the list
    while (curr) {
        // If the current frame is immediately following the one before it, then
        // increase the number of consecutive frames. If it's not, then reset
        // the consecutive frames counter and the start frame of the current
        // set.
        if ((void *)curr == (void *)curr->prev + ARCH_PAGE_SIZE) {
            so_far++;

            if (so_far == n) {
                break;
            }
        } else {
            set_start = curr;
            so_far = 1;
        }

        curr = curr->next;
    }

    if (so_far == n) {
        // Cut allocated region out of the linked list
        if (set_start->prev) {
            set_start->prev->next = curr->next;
        } else {
            frame_list = curr->next;
        }

        if (curr->next) curr->next->prev = set_start->prev;

        free_frames -= n;

        return set_start;
    } else {
        return NULL; // We couldn't find a long enough run
    }
}

void pffree_one(void *pf)
{
    frame_insert(pf);
}

void pffree_consecutive(void *pf_first, unsigned int n)
{
    for (unsigned int b = 0; b < n; b++) {
        pffree_one((void *)pf_first + (b * ARCH_PAGE_SIZE));
    }
}
