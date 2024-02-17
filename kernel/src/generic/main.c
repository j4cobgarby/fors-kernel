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

void task1(void *)
{
    for (;;) {
        __asm__ volatile("int $0xf0" : : "a"(0));
    }
}

void task2(void *)
{
    for (;;) {
        __asm__ volatile("int $0xf0" : : "a"(1));
    }
}

void _start(void)
{
    arch_early_setup();
    arch_init_memory();
    arch_late_setup();

    struct limine_framebuffer *fb = framebuf_req.response->framebuffers[0];
    uint32_t *fb_arr = fb->address;

    void *user_start = (void *)0x200000000;

    void *user_code_phys = pfalloc_one();
    void *user_stack = pfalloc_one();

    void *tmp = tmpmap(user_code_phys);
    if (tmp) memcpy(tmp, &task1, 4096);

    int tid = mkthread("Funky Function", user_start, NULL, user_start + 4096 * 2, 1);

    if (vmap(tid, user_code_phys, (void *)0x200000000, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        printk("Couldn't map userspace page\n");
        for (;;)
            ;
    }

    if (vmap(tid, user_stack, (void *)0x200000000 + 4096, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        printk("Couldn't map user stack.\n");
        for (;;)
            ;
    }

    user_code_phys = pfalloc_one();
    user_stack = pfalloc_one();

    tmp = tmpmap(user_code_phys);
    if (tmp) memcpy(tmp, &task2, 4096);

    int tid2 = mkthread("Another Thread", user_start, NULL, user_start + 4096 * 2, 1);

    if (vmap(tid2, user_code_phys, (void *)0x200000000, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        printk("Couldn't map userspace page\n");
        for (;;)
            ;
    }

    if (vmap(tid2, user_stack, (void *)0x200000000 + 4096, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        printk("Couldn't map user stack.\n");
        for (;;)
            ;
    }

    enqueue_thread(tid);
    enqueue_thread(tid2);
    printk("Starting threads %d and %d\n", tid, tid2);
    arch_start_running_threads();

    // pic_unblock_irq(0);

    // uint32_t col = 0xff0000;
    // uint32_t col2 = 0x00ff00;

    for (;;) {
        __asm__("hlt");

        // for (int i = 0; i < 200; i++) {
        //     for (int j = 0; j < 200; j++) {
        //         if ((i < 100 && j < 100) || (i >= 100 && j >= 100)) {
        //             fb_arr[i * fb->pitch/4 + j] = col;
        //         } else {
        //             fb_arr[i * fb->pitch/4 + j] = col2;
        //         }
        //     }
        // }
        // col >>= 8;
        // if (col == 0x0) col = 0xff0000;
        // col2 >>= 8;
        // if (col2 == 0x0) col2 = 0xff0000;

        // // for (int i = 0; i < 100; i++) {
        // //     for (int j = 0; j < 100; j++) {
        // //         fb_arr[i * fb->pitch/4 + j] = (i % 256 << (0+sh)) + (j % 256 <<
        // (8+sh));
        // //     }
        // // }
    }
}
