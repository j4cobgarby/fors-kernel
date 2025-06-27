#include "arch/x64/memory.h"
#include "fors/printk.h"
#include "fors/process.h"
#include "fors/init.h"
#include "fors/memory.h"
#include "fors/filesystem.h"
#include "fors/fs/test.h"
#include "fors/panic.h"
#include "fors/ata.h"
#include "fors/store.h"

#include "fors/types.h"
#include "forslib/string.h"

#include "limine.h"

#include <stddef.h>

volatile struct limine_framebuffer_request framebuf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

void task1(void *)
{
    char buff[128];
    int ret = -1;
    __asm__ volatile("int $0xf0" : "=a"(ret) : "a"(0), "S"("test.txt"), "b"(2));

    int nread = -1;
    __asm__ volatile("int $0xf0" : "=a"(nread) : "a"(2), "S"(buff), "b"(ret), "c"(128));

    __asm__ volatile("int $0xf0" : : "a"(100), "S"("File contents:\n"));
    __asm__ volatile("int $0xf0" : : "a"(100), "S"(buff));

    for (;;) { }
}

void _start(void)
{
    arch_initialise();

    register_storetype(ata_store_type);
    
    store_id si = register_store("atapio", "pm");

    char *buf;
    int ret = bc_get(si, 0, &buf);
    if (ret)
	printk("Read from ATA: '%s'\n", buf);

    strcpy(buf, "Overwritten!");
    printk("Written: ret=%d\n", bc_put(si, 0));

    /* Initialise filesystem root */
    mount_t *testmnt = &mounts[0];
    testmnt->dev = -1;
    testfs_type.initmnt(testmnt);
    vfs_root = get_node_byid(testmnt, testmnt->root_fsnode);

    void *user_start = (void *)0x200000000;
    void *user_code_phys = pfalloc_one();
    void *user_stack = pfalloc_one();
    void *user_stack_top_page = (void *)((uint64_t)&_FORS_KERNEL_START - 4096);

    void *tmp = tmpmap(user_code_phys);
    if (tmp) memcpy(tmp, &task1, 4096);

    int tid = create_process(
        "initial", user_start, NULL, user_stack_top_page + 4095, 1);

    if (vmap(tid, user_code_phys, (void *)0x200000000, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        KPANIC("Couldn't map userspace page\n");
    }

    if (vmap(tid, user_stack, user_stack_top_page, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        KPANIC("Couldn't map user stack.\n");
    }

    enqueue_proc(tid);
    arch_start_running_procs();

__attribute_maybe_unused__ hlt:
    for (;;) {
        __asm__("hlt");
    }
}
