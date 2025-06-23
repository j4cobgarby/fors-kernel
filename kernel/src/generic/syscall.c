#include "fors/syscall.h"
#include "fors/printk.h"

long long syscall_dispatch(
    long vec, long long p_1, long long p_2, long long p_3)
{
    /* On x86:
     * vec = rax
     * p_1 = rsi
     * p_2 = rbx
     * p_3 = rcx
     */
    // TODO: This switch could be sped up as a lookup table,
    // but then the __sys_X funcs need a generic signature.
    switch (vec) {
    case 0:
        return __sys_open((const char *)p_1, p_2);
    case 1:
        return __sys_close((fd_t)p_2);
    case 2:
        return __sys_read(p_2, p_3, (char *)p_1);
    case 3:
        return __sys_write(p_2, p_3, (const char *)p_1);
    default:
        return -1;
    }
}
