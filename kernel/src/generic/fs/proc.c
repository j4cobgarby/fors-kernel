#include "fors/filesystem.h"
#include "forslib/string.h"

int proc_lookup_node(inode *parent, const char *name, size_t name_len, inode **result) {
    if (parent == parent->mount->root_node && strncmp(name, "hello", name_len) == 0) {
        
    }
}
