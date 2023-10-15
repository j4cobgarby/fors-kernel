#ifndef __INCLUDE_FORSLIB_STRING_H__
#define __INCLUDE_FORSLIB_STRING_H__

#include <stddef.h>

// These functions all match the regular libc versions

void   *memccpy(void *, const void *, int, size_t);
void   *memchr(const void *, int, size_t);
void   *memrchr(const void *, int, size_t);
int     memcmp(const void *, const void *, size_t);
void   *memcpy(void *, const void *, size_t);
void   *memset(void *, int, size_t);

char   *stpcpy(char *, const char *);
char   *stpncpy(char *, const char *, size_t);

char   *strcat(char *, const char *);
char   *strncat(char *, const char *, size_t);

char   *strchr(const char *, int);
char   *strrchr(const char *, int);
char   *strnchr(const char *, int, size_t);
char   *strnrchr(const char *, int, size_t);

char   *strpbrk(const char *, const char *);

int     strcmp(const char *, const char *);
int     strncmp(const char *, const char *, size_t);

char   *strcpy(char *, const char *);
char   *strncpy(char *, const char *, size_t);

size_t  strspn(const char *, const char *);
size_t  strcspn(const char *, const char *);

size_t  strlen(const char *);
size_t  strnlen(const char *, size_t);

void x();

#endif /* __INCLUDE_FORSLIB_STRING_H__ */