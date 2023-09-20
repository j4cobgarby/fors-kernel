#ifndef __INCLUDE_FORS_KERRNO_H__
#define __INCLUDE_FORS_KERRNO_H__

// Error codes for use by the kernel. Separate ones used in userspace.

// Globally applicable error codes

#define EGENERIC -1 // Generic error
#define EINVARG  -2 // Invalid argument
#define EIMPL    -3 // Thing not implemented

// Memory related ones

#define ENOMAP  -4 // Virtual address has no physical mapping

#endif /* __INCLUDE_FORS_KERRNO_H__ */