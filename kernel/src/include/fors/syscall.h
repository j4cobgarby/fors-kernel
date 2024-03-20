#ifndef __INCLUDE_FORS_SYSCALL_H__
#define __INCLUDE_FORS_SYSCALL_H__

#include "fors/types.h"
#include "fors/filesystem.h"
#include <stddef.h>

__attribute__((no_caller_saved_registers)) int syscall_dispatch(
    long vec, long long p_1, long long p_2, long long p_3);

int __sys_open(const char *path, of_mode_t mode);
int __sys_close(fd_t fd);
ssize_t __sys_read(fd_t fd, size_t num, char *buff);
ssize_t __sys_write(fd_t fd, size_t num, const char *buff);
int __sys_seek(fd_t fd, ssize_t offset, seek_anchor_t type);
ssize_t __sys_getpos(fd_t fd);
int __sys_readdir(fd_t fd, int count, dir_entry_t *arr);
int __sys_makenode(fsn_type_t type, fsn_perm_t perms, const char *path);
int __sys_makelink(const char *link_path, const char *dest_path);
int __sys_mount(long device, const char *mount_dir);
int __sys_unmount(const char *mount_dir);
int __sys_delete(const char *node_dir);
int __sys_fork();
int __sys_quit(int exit_code);
int __sys_exec(const char *exe);

#endif /* __INCLUDE_FORS_SYSCALL_H__ */
