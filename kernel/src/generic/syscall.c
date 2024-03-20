#include "fors/syscall.h"

int syscall_dispatch(long vec, long long p_1, long long p_2, long long p_3)
{
    switch (vec) {
    case 0:
        return __sys_open((const char *)p_1, p_2);
    }
    return -1;
}
