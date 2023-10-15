#include "fors/printk.h"
#include "fors/thread.h"
#include "fors/init.h"
#include "fors/memory.h"

#include "forslib/string.h"

#include "limine.h"

#include <stdint.h>
#include <stddef.h>


volatile struct limine_framebuffer_request framebuf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

void funky_func(void*) {
    __asm__ ("xchg %bx, %bx");

    for (;;);

    // struct limine_framebuffer *fb = framebuf_req.response->framebuffers[0];
    // uint32_t *fb_arr = fb->address;

    // printk("I am a thread!\n");

    // for (;;) {
    //     printk("Thread woken.\n");
    //     for (int i = 0; i < 256; i++) {
    //         for (int j = 0; j < 256; j++) {
    //             fb_arr[i * fb->pitch/4 + j] = (i % 256 << 0) + (j % 256 << 8);
    //         }
    //     }
    //     __asm__("hlt");
    // }
}

void _start(void) {
    arch_early_setup();
    arch_init_memory();
    arch_late_setup();

    struct limine_framebuffer *fb = framebuf_req.response->framebuffers[0];
    uint32_t *fb_arr = fb->address;

    void *user_start = (void*)0x200000000;

    if (vmap(-1, pfalloc_one(), (void*)0x200000000, 4096, 
    VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT) < 0) {
        printk("Couldn't map userspace page\n"); for (;;);
    }

    if (vmap(-1, pfalloc_one(), (void*)0x200000000 + 4096, 4096,
    VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT) < 0) {
        printk("Couldn't map user stack.\n"); for (;;);
    }

    memcpy(user_start, &funky_func, 4096);

    int tid = mkthread("Funky Function", user_start, NULL, user_start + 4096*2, 1);
    printk("Created thread with TID %d\n", tid);

    for (;;) {
        __asm__("hlt");
        printk("Back to main thread!\n");
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                fb_arr[i * fb->pitch/4 + j] = (i % 256 << 8) + (j % 256 << 16);
            }
        }
    }
}