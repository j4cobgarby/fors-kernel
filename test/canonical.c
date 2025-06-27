#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define TAG_SIZE 8

bool is_canonical(const char *path)
{
    const char *tag_end;
    size_t tag_len;

    if (!path) return false;

    // Check tag
    if (path[0] != ':') return false; // No starting : for tag
    tag_end = strchr(path + 1, ':');
    if (!tag_end) return false; // No closing : for tag
    tag_len = tag_end - path - 1;
    if (tag_len == 0 || tag_len > TAG_SIZE) return false; // No/too long tag

    path = tag_end + 1;
    if (path[0] == '/') return false; // Rule 3)
    for (; *path; path++) {
        if (path[0] == '/' && (path[1] == '\0' || path[1] == '/'))
            return false; // Rules 4,5
    }

    return true;
}

int try_make_canonical(char *path, size_t *len_res)
{
    char *save_path = path;
    if (is_canonical(path)) {
        *len_res = strlen(path);
        return 0;
    }

    char *tag_end = strchr(path + 1, ':');
    if (tag_end == NULL) return -1;
    int tag_len = tag_end - path + 1;

    char *buff = malloc(strlen(path) + 1);
    int buff_i = 0;

    // First, we assume that the tag is correct. If it's not, there's nothing
    // we can do about it anyway.
    strncpy(buff, path, tag_len);
    buff_i += tag_len;

    for (path = tag_end + 1; *path; path++) {
        if (path[0] != '/') {
            buff[buff_i++] = path[0];
        } else {
            // Don't add trailing or double(or more) slashes
            // or leading slashes
            if (path[1] == '\0')
                break;
            else if (path[1] == '/') {
                bool found_nonslash = false;
                for (char *c = path + 1; *c; c++) {
                    if (*c != '/') {
                        if (path != tag_end + 1) buff[buff_i++] = '/';
                        path = c - 1; // Skip adjacent slashes
                        found_nonslash = true;
                        break;
                    }
                }
                // printf("\n");
                if (!found_nonslash) break; // Got to end of path, just break
            } else {
                if (path != tag_end + 1) buff[buff_i++] = '/';
            }
        }
    }

    buff[buff_i++] = '\0';

    strcpy(save_path, buff);
    free(buff);

    return is_canonical(path);
}

#define TEST_CANONICAL(path, expected)                                         \
    {                                                                          \
        bool c = is_canonical(path);                                           \
        if (c == expected)                                                     \
            printf("[PASS]\t");                                                \
        else {                                                                 \
            failures++;                                                        \
            printf("[FAIL]\t");                                                \
        }                                                                      \
        printf("(%d) \"%s\"\n", c, path);                                      \
    }

int main()
{
    int failures = 0;

    char *goods[] = {
        ":drive1:my_docs/school/homework1.txt",
        ":usb:arch_linux_live_disk.iso",
        ":backup:backups/2023-12-07",
        ":ssd1:",
        ":tag::a_nice_file.txt",
        ":tag: a file.txt",
    };

    char *bads[] = {
        ":drive1:/my_docs/my_paper.txt",
        ":drive1:my_docs//my_paper.txt",
        ":drive1://///hello////world////",
        "/usr/bin/helix",
        ":usr:bin/helix/",
    };

    for (size_t i = 0; i < sizeof(goods) / sizeof(char *); i++) {
        TEST_CANONICAL(goods[i], true);
    }

    for (size_t i = 0; i < sizeof(bads) / sizeof(char *); i++) {
        TEST_CANONICAL(bads[i], false);
    }

    for (size_t i = 0; i < sizeof(bads) / sizeof(char *); i++) {
        char *mut = malloc(64);
        strcpy(mut, bads[i]);
        printf("%s --> ", mut);
        try_make_canonical(mut, NULL);
        // strcpy(bads[i], "Hello");
        // bads[i][0] = '#';
        printf("%s\n", mut);
        free(mut);
    }

    // TEST_CANONICAL(":drive1:my_docs/school/homework1.txt", true);
    // TEST_CANONICAL(":usb:arch_linux_live_disk.iso", true);
    // TEST_CANONICAL(":backup:backups/2023-12-07", true);
    // TEST_CANONICAL(":ssd1:", true);

    // TEST_CANONICAL(":drive1:/my_docs/my_paper.txt", false);
    // TEST_CANONICAL(":drive1:my_docs//my_paper.txt", false);
    // TEST_CANONICAL("/usr/bin/helix", false);
    // TEST_CANONICAL(":usr:bin/helix/", false);

    // // Weirder cases
    // TEST_CANONICAL(":tag::a_nice_file.txt", true); // Here the file does
    // indeed have a : at the start TEST_CANONICAL(":tag: a file.txt", true); //
    // Files may have spaces in them

    printf("");

    printf("Failed test cases: %d\n", failures);

    return 0;
}
