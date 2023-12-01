#ifndef __INCLUDE_FORS_SYSCALL_H__
#define __INCLUDE_FORS_SYSCALL_H__

int syscall_dispatch(unsigned long long vector, unsigned long long p1, unsigned long long p2, unsigned long long p3);

int __syscall_open(const char *path, int flags);

#endif /* __INCLUDE_FORS_SYSCALL_H__ */