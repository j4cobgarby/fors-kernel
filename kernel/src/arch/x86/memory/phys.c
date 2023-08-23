#include "limine.h"
#include "src/arch/x86/memory/memory.h"
#include "src/generic/memory/memory.h"

#include <stdlib.h>

volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

static struct frame_marker *frame_list;
static unsigned long int free_frames;

static void update_cons(struct frame_marker *from) {
    if (!from) return;

    struct frame_marker *curr = from->prev;
    
    while (curr && curr->next == curr + ARCH_PAGE_SIZE) {
        curr->cons = curr->next->cons + 1;
        curr = curr->prev;
    }
}

// Insert a page frame into the frame list, ordered by ascending address.
static void frame_insert(void *frame_ptr) {
    free_frames++;

    struct frame_marker *prev = NULL,
                        *to_ins = frame_ptr;

    for (struct frame_marker *curr = frame_list; curr; curr = curr->next) {
        if (to_ins < curr) {
            to_ins->next = curr;

            update_cons(to_ins->next);

            if (to_ins->next && to_ins->next == to_ins + ARCH_PAGE_SIZE) {
                to_ins->cons = to_ins->next->cons + 1;
            }

            if (prev) { // If we're not inserting at the start of the list...
                to_ins->prev = prev;
                prev->next = to_ins;
            } else { // Or if we are...
                to_ins->prev = NULL;
                frame_list = to_ins;
            }
            
            return;
        } else if (curr->next == NULL) {
            curr->next = to_ins;

            to_ins->next = NULL;
            to_ins->prev = curr;
            to_ins->cons = 0;

            return;
        } else {
            prev = curr;
        }
    }
}

int x86_init_physical_memory() {
    struct limine_memmap_response *mm_resp = memmap_req.response;

    free_frames = 0;
    frame_list = NULL;

    for (size_t i = 0; i < mm_resp->entry_count; i++) {
        struct limine_memmap_entry *ent = mm_resp->entries[i];

        if (ent->type == LIMINE_MEMMAP_USABLE) {
            for (unsigned int byte = 0; byte < ent->length; byte += ARCH_PAGE_SIZE) {
                frame_insert((void*)ent->base + byte);
            }
        }
    }

    return 0;
}

void *pfalloc_one() {
    void *ret = (void*)frame_list;
    frame_list = frame_list->next;
    frame_list->prev = NULL;

    free_frames--;

    return ret;
}

void *pfalloc_consecutive(int n) {
    
}

void pffree_one(void *pf) {
    frame_insert(pf);
}

void pffree_consecutive(void *pf_first, int n) {
    for (int b = 0; b < n; b++) {
        pffree_one(pf_first + (b * ARCH_PAGE_SIZE));
    }
}