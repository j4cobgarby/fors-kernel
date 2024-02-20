#include "fors/filesystem.h"
#include "fors/types.h"
#include "forslib/string.h"

int can_read(fsnode_t *dir, pid_t p)
{
    return p == -1 || dir->perms & FP_ROTH || (dir->perms & FP_RUSR && dir->user == p);
}

int can_write(fsnode_t *dir, pid_t p)
{
    return p == -1 || dir->perms & FP_WOTH || (dir->perms & FP_WUSR && dir->user == p);
}

int can_exec(fsnode_t *dir, pid_t p)
{
    return p == -1 || dir->perms & FP_XOTH || (dir->perms & FP_XUSR && dir->user == p);
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
        if (fsnodes[i].ref_count == 0) return i;
    return -1;
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

fsnode_t *get_node_n(fsnode_t *parent, const char *name, size_t len)
{
    /* First do a lookup on existing fslinks */
    for (fslink_t *link = parent->child; link; link = link->next) {
        if (strncmp(link->name, name, len) == 0) {
            return link->node;
        }
    }

    /* If that didn't work, ask the fs to get a child (maybe from the disk, or whatever it
     * wants) */
    return parent->mountpoint->fs->retrieve_child(parent, name, len);
}

fsnode_t *get_node(fsnode_t *parent, const char *name)
{
    return get_node_n(parent, name, FILENAME_SIZE);
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
        if (fn->internal_id == internal_id && fn->mountpoint == mount) return fn;
    }

    return mount->fs->node_from_id(internal_id);
}

const char *first_of_trailing(const char *s, char c)
{
    const char *end;
    for (end = s + strlen(s) - 1; end >= s && *end == '/'; end--) { }
    return end + 1;
}

fsnode_t *find_parent_checkperm(fsnode_t *root, const char *path, pid_t p)
{
    fsnode_t *parent = root;
    const char *next_delim, *end, *last_delim;

    if (path[0] != '/') return NULL;
    path++; /* skip initial delim */

    end = first_of_trailing(path, '/');
    last_delim = memrchr(path, '/', end - path);

    while ((next_delim = strnchr(path, '/', end - path)) != NULL) {
        if (next_delim - path > 0) { /* Skip empty fields; a//b == a/b */
            parent = get_node_n(parent, path, next_delim - path);
            if (!parent) return NULL; /* Couldn't find intermediate directory */
        }
        if (next_delim == last_delim) break;
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
