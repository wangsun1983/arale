#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "ctype.h"

struct AvahiHashmap {
    AvahiHashFunc hash_func;
    AvahiEqualFunc equal_func;
    AvahiFreeFunc key_free_func, value_free_func;

    Entry *entries[HASH_MAP_SIZE];
    AVAHI_LLIST_HEAD(Entry, entries_list);
};

typedef unsigned (*hashmap_hash_func)(const void *data);
typedef int (*hashmap_equal_func)(const void *a, const void *b);
typedef void (*hashmap_free_func)(void *p);
typedef void (*hashmap_foreach_func)(void *key, void *value, void *userdata);

typedef struct AvahiHashmap AvahiHashmap;
typedef struct AvahiHashmap hashmap_t;


public hashmap_t *hashmap_create(hashmap_hash_func hash_func, hashmap_equal_func equal_func,
  hashmap_free_func key_free_func,
  hashmap_free_func value_free_func);

public void *hashmap_put(hashmap_t *hashmap,char *key,int keylength,char *data);
public char *hashmap_get(hashmap_t *hashmap,char *key,int keylength);
public void *hashmap_for_each(hashmap_t *hashmap,hashmap_foreach_func func);
public void *hashmap_remove(hashmap_t *hashmap,char *key);
public void *hashmap_destroy(hashmap_t *hashmap);
public void *hashmap_replace(hashmap_t *hashmap,char *key,char *value);


#endif
