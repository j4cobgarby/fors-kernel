#include "fors/filesystem.h"
#include "fors/kheap.h"
#include "fors/syscall.h"
#include "fors/thread.h"
#include "forslib/string.h"

int __sys_open(const char *path, of_mode_t mode)
{
    char *curr_cwd = threads[current_thread].cwd;
    int full_path_len = strlen(curr_cwd) + strlen(path) + 1;
    char *full_path = kalloc(full_path_len);

    strcpy(full_path, curr_cwd);
    strcpy(full_path + strlen(curr_cwd), path);

    int ret = vfs_open(current_thread, full_path, mode);

    kfree(full_path);
    return ret;
}
