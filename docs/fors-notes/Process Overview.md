Being a microkernel, fors takes a simplistic view towards managing processes.

Fors knows about processes, but doesn't care about threads. Processes manage their own threads. This provides some advantages and disadvantages, discussed more in [[Threads]].

The `newproc` syscall is used for creating a new process ([[Syscalls]]). It is defined as:

```c
int newproc(const char *exe_path, unsigned int flags);
```

`exe_path` should specify the executable's path in the filesystem (for example "/exe/gcc"). If the process fails, `newproc` will return <0, or otherwise it returns the ID of the new process.

Fors automatically schedules all created processes, unless they are currently waiting on some resource, waiting for some signal, asleep, etc.