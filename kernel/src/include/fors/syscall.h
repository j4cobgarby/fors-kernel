#ifndef __INCLUDE_FORS_SYSCALL_H__
#define __INCLUDE_FORS_SYSCALL_H__

#include "fors/types.h"
#include "fors/filesystem.h"
#include <stddef.h>

#define SYSCALL_VECTOR 0xf0

// __attribute__((no_caller_saved_registers))
long long syscall_dispatch(
    long vec, long long p_1, long long p_2, long long p_3);

long long __sys_open(const char *path, of_mode_t mode);
long long __sys_close(pfd_t pfd);
long long __sys_read(pfd_t pfd, size_t num, char *buff);
long long __sys_write(pfd_t pfd, size_t num, const char *buff);
long long __sys_seek(pfd_t pfd, ssize_t offset, seek_anchor_t type);
long long __sys_getpos(pfd_t pfd);
long long __sys_readdir(pfd_t pfd, int count, dir_entry_t *arr);
long long __sys_makenode(fsn_type_t type, fsn_perm_t perms, const char *path);
long long __sys_makelink(const char *link_path, const char *dest_path);
long long __sys_mount(long device, const char *mount_dir);
long long __sys_unmount(const char *mount_dir);
long long __sys_delete(const char *node_dir);
long long __sys_fork();
long long __sys_quit(int exit_code);
long long __sys_exec(const char *exe);

#endif /* __INCLUDE_FORS_SYSCALL_H__ */
