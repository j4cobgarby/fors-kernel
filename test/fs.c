/* Tests the logic of find parent in vfs */

#include "forslib/string.h"
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

void find_parent(const char *path)
{
    printf("find_parent(%s)\n", path);
    const char *next_delim, *last_delim, *end;
    char *parent = malloc(128 * sizeof(char));

    strcpy(parent, "/");

    if (path[0] != '/') {
        printf("path[0] != '/' -> NULL\n");
        return;
    }

    path++;

    for (end = path + strlen(path) - 1; end >= path && *end == '/'; end--) { };

    end++;

    last_delim = strnrchr(path, '/', end - path);

    while ((next_delim = strnchr(path, '/', end - path)) != NULL) {
        if (next_delim - path > 0) {
            strncpy(parent, path, next_delim - path);
            parent[next_delim - path] = '\0';
            printf("Parent = '%s' (len=%d)\n", parent, next_delim - path);
        }

        if (next_delim == last_delim) break;
        path = next_delim + 1;
    }

    printf("Final result = '%s'\n", parent);
}

int main()
{
    char *buf = NULL;
    size_t len = 0;
    ssize_t nread;

    printf("> ");
    while ((nread = getline(&buf, &len, stdin)) != -1) {
        buf[strlen(buf) - 1] = '\0';
        // printf("path: '%s'\n", buf);
        find_parent(buf);
        printf("> ");
    }

    return 0;
}
