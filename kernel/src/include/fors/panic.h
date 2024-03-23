#ifndef INCLUDE_FORS_PANIC_H_
#define INCLUDE_FORS_PANIC_H_

#define __AS_STRING_LITERAL(x)      #x
#define __EVAL_AS_STRING_LITERAL(x) __AS_STRING_LITERAL(x)

#define KPANIC(reason)                                                         \
    {                                                                          \
        __asm__ volatile("cli");                                               \
        printk("[" __FILE__ ":" __EVAL_AS_STRING_LITERAL(                      \
            __LINE__) "] Kernel Panic: '" reason "'");                         \
        for (;;) __asm__ volatile("hlt");                                      \
    }

#define KPANIC_VA(fmt, ...)                                                    \
    {                                                                          \
        __asm__ volatile("cli");                                               \
        printk("[" __FILE__ ":" __EVAL_AS_STRING_LITERAL(                      \
                   __LINE__) "] Kernel Panic: '" fmt "'\n",                    \
            __LINE__, __VA_ARGS__);                                            \
        for (;;) __asm__ volatile("hlt");                                      \
    }

#endif // INCLUDE_FORS_PANIC_H_
