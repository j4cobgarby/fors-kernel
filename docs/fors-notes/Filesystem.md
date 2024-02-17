# Filesystem

## Overview

### Filesystem Nodes (`fsnode_t`)

In memory, files/directories from any filesystem type (e.g. ext2, fat, virtual ones) are represented by the same structure -- the `fsnode_t`. This provides a seamless interface to various types. fsnodes are conceptually very similar to inodes found in UNIX. They are an **in-core representation of a file or directory on a real disk, or of that in a virtual filesystem**.

fsnodes are organised in a tree structure, where each fsnode has a pointer to the first of its children. All fsnodes also have a `sibling` pointer, which organises siblings into a linked list. Importantly, fsnodes don't contain pointers to their parents, because due to the nature of most filesystems, two different files with two different parents can refer to the same fsnode (due to hard-links).

Memory for fsnodes is not actually allocated dynamically. There is a fixed maximum number of fsnodes, `NUM_FSNODES`. All fsnodes are stored in `fsnodes`.

There isn't necessarily (or usually) an fsnode in the system for every single file on the filesystem. Getting a child fsnode from a parent, given the child's name, roughly looks like this:

 1) Look through the parent's children linked list, starting at `child`.
    - If an fsnode by the correct name is found, then return it.
    - Otherwise, continue to 2.
 2) Call the `retrieve_child` method of the filesystem type associated with the parent, which requests the FS to return a new fsnode if the target file exists on disk (or wherever it likes).
    - If an fsnode is returned, this is the target child. Return it.
    - If NULL is returned, then either the `fsnodes` is full, or the child just doesn't exist.

So, what happens when the `fsnodes` array does fill up? Basically, when no more open files refer to a specific fsnode, it is seen as "free", so when a filesystem implementation is trying to read in a disk-node to an fsnode, they will see a free entry in the array and reallocate it.

### Open Files (`openfile_t`)

As hinted at, when a process opens a file, an `openfile_t` structure is used to represent this. I think of open files as "cursors" on an fsnode, where there can be many all referring to the same actual file (or directory), but reading and writing at different locations.

When a file is opened by a process, the fsnode that it refers to has its `ref_count` increased by one. This is true of every "file" type, including directories. It's important to note that a directory's fsnode doesn't need to exist (in memory) for its children to have fsnodes.

## How it's used

Below are some rough pseudocode implementations of some high level filesystem functions.
They assume the existence of the following procedures:

 - `get_node(parent fsnode, name) => fsnode at parent/name` queries the fs implementation to make a new fsnode for an existing fsnode named name, directly inside the parent node.
 - `put_node(node)` frees up the entry in `fsnodes` of a given node.

 - `find_node(root fsnode, path) => fsnode at path` recurses the tree structure from root until path's parent's node is got (using `get_node`), and then uses `get_node` again to attempt to find the required node. Basically, an extended version of `get_node`.

 - `add_child(parent fsnode, new fsnode)` appends a new child node to a children list.

 - `add_child_byname(parent fsnode, new node's name)` = `add_child(parent, get_node(parent, new node's name))`

### Opening a file

Parameters:
 - path to open
 - access mode
 - process id

 1) node = find_node(system root, path to open)
 2) file = {node = node, cursor = 0, mode = mode, proc = process id}
 3) find available slot in `open_files`, and insert file
 3) increment node's reference count
 4) return insertion index

### Closing a file

Parameters:
 - file to close

 1) decrement file->node's reference counter
 2) if it is now at 0, call put_node on it
 3) mark the file's entry in `open_files` as free

### Reading a file
### Writing to a file
### Seeking in a file

### Creating a new file
### Creating a hard-link

### Creating a directory
### Deleting a directory
### Reading entries from a directory

### Mounting a filesystem
