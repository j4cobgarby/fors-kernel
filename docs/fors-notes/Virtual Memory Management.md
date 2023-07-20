## Kernel Heap

## Process Heaps
The virtual memory system handles:

 - Keeping track of address spaces for the kernel and for each process.
 - Calling the physical memory manager (PMM) to supply virtual address regions with memory.
 - Sharing memory between processes.
 - Kernel temporary page mappings (e.g. so that the kernel can easily modify process mapped memory.)

## Architecture Common Functionality

Each supported architecture may deal with virtual (and physical) memory in different ways. Here is the set of constants, functions, and types which all architectures must define:

 - The **constant** `__ARCH__PAGE_SIZE` should be defined as the size (in bytes) of pages of memory, for example `#define __ARCH_PAGE_SIZE 4096`.
 - The **type** `physaddr_t` should be defined as an integer type at least large enough to represent any physical address, and also negative numbers (for error codes), for example `typedef intptr_t physaddr_t`.
 - The **type** `virtaddr_t` should be defined as an integer type at least large enough to represent any vitual address, and also negative numbers (for error codes), for example `typedef intptr_t virtaddr_t`.
   
 -  `int vmap(int pid, physaddr_t pa, virtaddr_t va, size_t size);` 
   `pa`: The physical address of the beginning of the region to map. This should be page-aligned.
   `va`: The virtual address of the beginning of the region to map to. This should be page-aligned.
   `pid`: The ID of the process for which this mapping should be performed. If `pid` is -1 (an invalid process ID, since they begin at 0), then the mapping is performed on the kernel VAS.
   `size`: The size (in bytes) of the region to map. This should be page-aligned to avoid confusion, but if it is not then it is automatically rounded up to the next page boundary.
   
   Maps a given physical address to a given physical address in a particular process's virtual address space (VAS). The `pid` argument allows the architecture specific code to provide individual page mappings for each process in any way it sees fit. On x86-64, this will involve maintaining a separate PML4 table per each process.
   
 - `int vunmap(int pid, virtaddr_t va, size_t size);`
   `va`: The virtual address of the beginning of the VAS region to unmap. Page-aligned.
   `pid`: The ID of the process for which this region should be unmapped. If `pid` is -1, then the it is unmapped from the kernel VAS.
   `size`: The size (in bytes) of the region to unmap. This should be page-aligned to avoid confusion, but if it is not then it is automatically rounded up to the next page boundary.
   
   Unmaps a particular virtual address from the VAS of a specified process, or from the kernel.
   
 - `int vismappedto(int pid, virtaddr_t va, physaddr_t pa);`
   `va`: The virtual address to check.
   `pa`: The page frame in question.
   `pid`: The ID of the process to check the mapping in. 
   
   Check if a particular virtual memory block is mapped to a particular page frame, in a particular process's (or the kernel's) VAS. Returns 1 if it is, or 0 otherwise.

 - `physaddr_t vgetmapping(int pid, virtaddr_t va, unsigned int *attr);`
   `va`: The virtual address to check.
   `pid`: The ID of the process to check the mapping in.
   `attr`: Pointer to variable which will be set to store the attributes of the mapping. This is architecture dependent, and each architecture can use it how it likes.
   
   Get the page frame address which virtual address maps to within a given process's VAS (or the kernel's, if `pid` is -1). Returns that physical address, or -1 if the page is not mapped.
