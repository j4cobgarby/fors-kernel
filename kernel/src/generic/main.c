#include "fors/printk.h"
#include "fors/thread.h"
#include "limine.h"

#include <stdint.h>
#include <stddef.h>

#include "fors/init.h"
#include "fors/memory.h"

volatile struct limine_framebuffer_request framebuf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

void funky_func(void*) {
    printk("I am a thread!\n");

    for (;;) {
        printk("Thread woken.\n");
        __asm__("hlt");
    }
}

void _start(void) {
    arch_early_setup();
    arch_init_memory();
    arch_late_setup();

    struct limine_framebuffer *fb = framebuf_req.response->framebuffers[0];
    uint32_t *fb_arr = fb->address;

    printk("Done.\n");

    int tid = mkthread("JACOB", funky_func, NULL);
    printk("Created thread with TID %d\n", tid);

    int k = 0;
    for (;;) {
        __asm__("hlt");
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                fb_arr[i * fb->pitch/4 + j] = (i % 256 << (k ? 0 : 8)) + (j % 256 << (k ? 8 : 16));
            }
        }
        k = !k;
    }
}