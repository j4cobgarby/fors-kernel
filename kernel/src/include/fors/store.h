#ifndef __INCLUDE_STORE_H__
#define __INCLUDE_STORE_H__

/* A store is a device (real or virtual) which stores blocks of data.
 * Disks are the main type.
 * The interface to access blocks from a store simplifies things:
 *  - A buffer cache is transparently in use
 *  */

#include <stdint.h>
#include <stdlib.h>

#include "fors/ata.h"
#include "fors/types.h"

#define STORE_OK           0
#define STORE_ERR_TIMEOUT  -1
#define STORE_ERR_NOBLK    -2
#define STORE_ERR_SETTINGS -3

struct store_type_t;
struct bc_entry_t;

typedef struct store_t {
    struct store_type_t *type;
    union {
        ata_device_t ata_info;
    } dev;

    struct bc_entry_t *bc_first;
} store_t;

#define MAX_CACHED_BLOCKS 2
#define MAX_STORES        8
#define MAX_STORETYPES    4

extern int bc_cached_count;
extern store_type_t store_types[MAX_STORETYPES];
extern store_t stores[MAX_STORES];

typedef struct bc_entry_t {
    struct bc_entry_t *next;
    store_id id;
    size_t addr;
    char *data;
} bc_entry_t;

storetype_id find_storetype(char name[8]);
storetype_id register_storetype(store_type_t type);
store_id register_store(char storetype[8], const char *cfg);
store_t *get_store_by_id(store_id id);

int bc_get(store_id id, size_t addr, char **buf);
int bc_put(store_id id, size_t addr);

#endif // __INCLUDE_STORE_H__
