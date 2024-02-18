#include "forslib/string.h"
#include "forslib/maths.h"

void *memccpy(void *dest, const void *src, int c, size_t n)
{
    size_t i;
    char *d = (char *)dest, *s = (char *)src;
    for (i = 0; i < n && s[i] != c; i++) d[i] = s[i];
    return i == n ? NULL : &d[i];
}

void *memchr(const void *mem, int c, size_t n)
{
    size_t i;
    char *m = (char *)mem;
    for (i = 0; i < n; i++) {
        if (m[i] == c) return &m[i];
    }
    return NULL;
}

void *memrchr(const void *mem, int c, size_t n)
{
    size_t i;
    char *m = (char *)mem;
    void *f = NULL;
    for (i = 0; i < n; i++) {
        if (m[i] == c) f = &m[i];
    }
    return f;
    // TODO: Why did I implement it like this? this is stupid, could just iterate from the
    // back.
}

int memcmp(const void *mema, const void *memb, size_t n)
{
    size_t i;
    char *sa = (char *)mema, *sb = (char *)memb;
    for (i = 0; i < n; i++) {
        if (sa[i] != sb[i]) return sa[i] - sb[i];
    }
    return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    size_t i;
    char *d = (char *)dest, *s = (char *)src;
    for (i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

void *memset(void *mem, int c, size_t n)
{
    size_t i;
    char *m = (char *)mem;
    for (i = 0; i < n; i++) m[i] = c;
    return mem;
}

char *stpcpy(char *dest, const char *src)
{
    for (; *src; src++, dest++) *dest = *src;
    *dest = '\0';
    return (char *)src;
}

char *stpncpy(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; *src && i < n; src++, dest++) *dest = *src;
    *dest = '\0';
    return dest;
}

char *strcat(char *dest, const char *src)
{
    stpcpy(dest + strlen(dest), src);
    return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
    stpncpy(dest + strlen(dest), src, n);
    return dest;
}

char *strchr(const char *s, int c)
{
    return memchr(s, c, strlen(s));
}

char *strchrnul(const char *s, int c)
{
    char *ret = memchr(s, c, strlen(s));
    if (ret == NULL) ret = (char *)s + strlen(s);
    return ret;
}

char *strrchr(const char *s, int c)
{
    return memrchr(s, c, strlen(s));
}

char *strnchr(const char *s, int c, size_t n)
{
    return memchr(s, c, strnlen(s, n));
}

char *strnrchr(const char *s, int c, size_t n)
{
    return memrchr(s, c, strnlen(s, n));
}

char *strpbrk(const char *s, const char *accept)
{
    size_t l = strlen(accept);
    size_t i;
    for (; *s; s++) {
        for (i = 0; i < l; i++) {
            if (*s == accept[i]) return (char *)s;
        }
    }
    return NULL;
}

int strcmp(const char *s1, const char *s2)
{
    // Note: This compares memory up to and INCLUDING the \0 char of the shorter
    // of the two strings. This correctly handles the behaviour of a shorter
    // string counting as "less" than a longer one, all other things equal.
    return memcmp(s1, s2, MIN(strlen(s1), strlen(s2)) + 1);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    return memcmp(s1, s2, MIN(strnlen(s1, n), strnlen(s2, n)) + 1);
}

char *strcpy(char *dest, const char *src)
{
    stpcpy(dest, src);
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    stpncpy(dest, src, n);
    return dest;
}

size_t strspn(const char *s, const char *accept)
{
    size_t n, i;
    size_t l = strlen(accept);
    for (n = 0; *s; s++) {
        for (i = 0; i < l; i++) {
            if (*s == accept[i]) {
                n++;
                break;
            } else
                return n;
        }
    }
    return n;
}

size_t strcspn(const char *s, const char *reject)
{
    size_t n, i;
    size_t l = strlen(reject);
    for (n = 0; *s; s++) {
        for (i = 0; i < l; i++) {
            if (*s == reject[i])
                return n;
            else {
                n++;
                break;
            }
        }
    }
    return n;
}

size_t strlen(const char *s)
{
    size_t i = 0;
    for (; *s; s++) i++;
    return i;
}

size_t strnlen(const char *s, size_t n)
{
    size_t i = 0;
    for (; *s && n > 0; s++, n--) i++;
    return i;
}
