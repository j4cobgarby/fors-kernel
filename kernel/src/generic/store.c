#include "fors/store.h"
#include "fors/kheap.h"
#include "fors/printk.h"
#include "forslib/string.h"

int bc_cached_count = 0;
store_type_t store_types[MAX_STORETYPES] = { 0 };
store_t stores[MAX_STORES] = { 0 };

storetype_id find_storetype(char name[8])
{
    for (storetype_id id = 0; id < MAX_STORETYPES; id++) {
        if (strncmp(name, store_types[id].name, 8) == 0) return id;
    }
    return -1;
}

storetype_id register_storetype(store_type_t type)
{
    for (storetype_id id = 0; id < MAX_STORETYPES; id++) {
        if (!store_types[id].name[0]) {
            store_types[id] = type;
            return id;
        }
    }
    return -1;
}

static store_t *get_store_by_id(store_id id)
{
    if (id < 0 || id >= MAX_STORES) return NULL;
    return &stores[id];
}

// Get a buffer cache's data buffer. It can then be edited elsewhere, and saved
// to disk with bc_put
int bc_get(store_id id, size_t addr, char **buf)
{
    store_t *st = get_store_by_id(id);
    if (!st) return 0;

    for (bc_entry_t *bc = st->bc_first; bc; bc = bc->next) {
        if (bc->addr == addr) {
            *buf = bc->data;
            return true;
        }
    }

    bc_entry_t *blk;

    // It's not cached yet
    if (bc_cached_count == MAX_CACHED_BLOCKS) {
        // We have to free up a currently cached block
        // In an ideal world we would pick an infrequently used one,
        // but for now let's just take the first one we can find!
        for (store_id i = 0; i < MAX_STORES; i++) {
            store_t *victim = &stores[i];
            if (victim->bc_first) {
                // This store has a cached block, let's uncache it and use
                // it for our new block.
                if (!victim->type->wr(&victim->dev, victim->bc_first->addr,
                        victim->bc_first->data)) {
                    printk("[store.c] Could not save victim to disk; looking "
                           "for another.\n");
                    continue;
                }

                blk = victim->bc_first; // Reuse block's metadata memory
                victim->bc_first = blk->next;
                kfree(blk->data);
                break;
            }
        }
        printk("[store.c] Bug? Tried to find a cached block to reuse, but "
               "failed to find any.\n");
        return 0;
    } else {
        // We can just make a new one
        blk = kalloc(sizeof(bc_entry_t));
        if (!blk) {
            printk("[store.c] Failed to allocate memory for new bc entry.\n");
            return 0;
        }

        blk->data = kalloc(st->type->block_sz);
        if (!blk->data) {
            printk("[store.c] Failed to allocate memory for new bc data "
                   "buffer.\n");
            kfree(blk);
            return 0;
        }

        blk->next = st->bc_first;
        st->bc_first = blk;

        bc_cached_count++;
    }

    *buf = blk->data;

    // Load data into new buffer
    return st->type->rd(&st->dev, addr, blk->data);
}

// Ensure that a checked-out buffer of addr in a given store is saved back to
// disk.
int bc_put(store_id id, size_t addr)
{
    store_t *st = get_store_by_id(id);
    if (!st) return -1;

    for (bc_entry_t *bc = st->bc_first; bc; bc = bc->next) {
        if (bc->addr == addr) {
            // It is checked out, so save it
            return st->type->wr(&st->dev, addr, bc->data);
        }
    }

    // Return success if block was not found, because it just means there was
    // nothing to write back
    return 1;
}

store_id register_store(char storetype[8], const char *cfg)
{
    store_id id = 0;
    for (; id < MAX_STORES && stores[id].type != NULL; id++);
    if (id == MAX_STORES) return -1;
    store_t *st = &stores[id];

    storetype_id stid = find_storetype("atapio");
    if (stid < 0) return -1;
    printk("Found storetype ID=%d\n", stid);

    store_type_t *type = &store_types[stid];

    st->type = type;
    st->bc_first = NULL;
    if (!type->init(&st->dev.ata_info, cfg)) {
        return -1;
    }

    return id;
}
