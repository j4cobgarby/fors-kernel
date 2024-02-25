#include "fors/filesystem.h"
#include "fors/types.h"
#include "forslib/string.h"

fsnode_t *vfs_root = NULL;
fsnode_t fsnodes[NUM_FSNODES] = { 0 };
fslink_t fslinks[NUM_FSLINKS] = { 0 };
openfile_t open_files[NUM_OPEN_FILES] = { 0 };
mount_t mounts[NUM_MOUNTS] = { 0 };

int vfs_init()
{
    return 0;
}

int can_read(fsnode_t *dir, pid_t p)
{
    return p == -1 || dir->perms & FP_ROTH
        || (dir->perms & FP_RUSR && dir->user == p);
}

int can_write(fsnode_t *dir, pid_t p)
{
    return p == -1 || dir->perms & FP_WOTH
        || (dir->perms & FP_WUSR && dir->user == p);
}

int can_exec(fsnode_t *dir, pid_t p)
{
    return p == -1 || dir->perms & FP_XOTH
        || (dir->perms & FP_XUSR && dir->user == p);
}

int find_free_link()
{
    for (int i = 0; i < NUM_FSLINKS; i++)
        /* If .node is NULL, the fslink is not in used */
        if (fslinks[i].node == NULL) return i;
    return -1;
}

int find_free_node()
{
    for (int i = 0; i < NUM_FSNODES; i++)
        if (fsnodes[i].type == EMPTY) return i;
    return -1;
}

fd_t find_free_fd_in(openfile_t *arr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (arr[i].node == NULL) return i;
    }
    return -1;
}

fd_t find_free_fd()
{
    return find_free_fd_in(open_files, NUM_OPEN_FILES);
}

/* add_child
 * Appends a given fsnode to a parent node's children list
 */
int add_child(fsnode_t *parent, fsnode_t *new, const char *name)
{
    int l_i = find_free_link();
    if (l_i == -1) return -1;
    if (strlen(name) >= FILENAME_SIZE) return -1;

    fslink_t *link = &fslinks[l_i];
    link->node = new;
    link->next = NULL;
    link->prev = NULL;
    strncpy(link->name, name, FILENAME_SIZE);

    if (parent->child == NULL) {
        parent->child = link;
        return 0;
    }

    /* Insert at start of list */
    parent->child->prev = link;
    link->next = parent->child;
    parent->child = link;
    return 0;
}

int del_child(fsnode_t *parent, fslink_t *to_del)
{
    to_del->node->ref_count--;
    to_del->node = NULL; /* Now link is "free" */

    if (to_del->prev) to_del->prev->next = to_del->next;
    if (to_del->next) to_del->next->prev = to_del->prev;

    if (to_del->prev)
        parent->child = to_del->prev;
    else if (to_del->next)
        parent->child = to_del->next;
    else
        parent->child = NULL;

    parent->child = NULL;

    return 0;
}

fsnode_t *get_node_with_len(fsnode_t *parent, const char *name, size_t len)
{
    /* First do a lookup on existing fslinks */
    for (fslink_t *link = parent->child; link; link = link->next) {
        if (strncmp(link->name, name, len) == 0) {
            return link->node;
        }
    }

    /* If that didn't work, ask the fs to get a child (maybe from the disk, or
     * whatever it wants) */

    long new_node_index = find_free_node();
    if (new_node_index < 0) return NULL;
    fsnode_t *new_node = &fsnodes[new_node_index];
    new_node->mountpoint = parent->mountpoint;

    long id = parent->mountpoint->fs->retrieve_child(parent, name, len);
    if (parent->mountpoint->fs->node_from_id(id, new_node) < 0) return NULL;
    return new_node;
}

fsnode_t *get_node(fsnode_t *parent, const char *name)
{
    return get_node_with_len(parent, name, strnlen(name, FILENAME_SIZE));
}

void put_node(fsnode_t *node)
{
    // TODO: Assert that node's ref count == 0
    node->mountpoint->fs->put_node(node);
    node->type = EMPTY;
}

fsnode_t *find_node(fsnode_t *root, const char *path)
{
    fsnode_t *parent = find_parent(root, path);
    if (!parent) return NULL;

    return get_node(parent, basename(path));
}

fsnode_t *get_node_byid(mount_t *mount, long internal_id)
{
    fsnode_t *fn;

    for (int i = 0; i < NUM_FSNODES; i++) {
        fn = &fsnodes[i];
        if (fn->type == EMPTY) continue;
        if (fn->internal_id == internal_id && fn->mountpoint == mount)
            return fn;
    }

    long new_node_index = find_free_node();
    if (new_node_index < 0) return NULL;
    fsnode_t *new_node = &fsnodes[new_node_index];
    new_node->mountpoint = mount;

    if (mount->fs->node_from_id(internal_id, new_node) < 0) return NULL;
    return new_node;
}

const char *first_of_trailing(const char *s, char c)
{
    const char *end;
    for (end = s + strlen(s) - 1; end >= s && *end == '/'; end--) { }
    return end + 1;
}

/* Find parent node, returning the parent node if given process has access to
 * read all dirs up to given path's parent dir, or otherwise return NULL. */
fsnode_t *find_parent_checkperm(fsnode_t *root, const char *path, pid_t p)
{
    fsnode_t *parent = root;
    const char *next_delim, *end, *last_delim;

    if (path[0] != '/') return NULL;
    path++; /* skip initial delim */

    end = first_of_trailing(path, '/');
    last_delim = memrchr(path, '/', end - path);

    /* exec permission on a directory means you can enter it */
    if (!can_exec(parent, p)) return NULL;

    while ((next_delim = strnchr(path, '/', end - path)) != NULL) {
        if (next_delim - path > 0) { /* Skip empty fields; a//b == a/b */
            parent = get_node_with_len(parent, path, next_delim - path);
            if (!parent) return NULL; /* Couldn't find intermediate directory */
        }
        if (next_delim == last_delim) break;
        if (!can_exec(parent, p)) return NULL;

        path = next_delim + 1;
    }

    return parent;
}

fsnode_t *find_parent(fsnode_t *root, const char *path)
{
    return find_parent_checkperm(root, path, -1);
}

const char *basename(const char *path)
{
    const char *end, *last_delim;

    end = first_of_trailing(path, '/');
    last_delim = memrchr(path, '/', end - path);

    if (last_delim)
        return last_delim + 1;
    else
        return NULL;
}
