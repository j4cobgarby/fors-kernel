#include "fors/syscall.h"

__attribute__((no_caller_saved_registers)) int syscall_dispatch(
    long vec, long long p_1, long long p_2, long long p_3)
{
    switch (vec) {
    case 0:
        return __sys_open((const char *)p_1, p_2);
    default:
        return -1;
    }
    return 0;
}
