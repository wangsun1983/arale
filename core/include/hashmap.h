#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "ctype.h"

#define HASH_MAP_SIZE 123

#define AVAHI_LLIST_HEAD(t,name) t *name

/** The pointers in the linked list's items. Use this in the item structure */
#define AVAHI_LLIST_FIELDS(t,name) t *name##_next, *name##_prev

/** Initialize the list's head */
#define AVAHI_LLIST_HEAD_INIT(t,head) do { (head) = NULL; } while(0)

/** Initialize a list item */
#define AVAHI_LLIST_INIT(t,name,item) do { \
                               t *_item = (item); \
                               _item->name##_prev = _item->name##_next = NULL; \
                               } while(0)

/** Prepend an item to the list */
#define AVAHI_LLIST_PREPEND(t,name,head,item) do { \
                                        t **_head = &(head), *_item = (item); \
                                        if ((_item->name##_next = *_head)) \
                                           _item->name##_next->name##_prev = _item; \
                                        _item->name##_prev = NULL; \
                                        *_head = _item; \
                                        } while (0)

/** Remove an item from the list */
#define AVAHI_LLIST_REMOVE(t,name,head,item) do { \
                                    t **_head = &(head), *_item = (item); \
                                    if (_item->name##_next) \
                                       _item->name##_next->name##_prev = _item->name##_prev; \
                                    if (_item->name##_prev) \
                                       _item->name##_prev->name##_next = _item->name##_next; \
                                    else {\
                                       *_head = _item->name##_next; \
                                    } \
                                    _item->name##_next = _item->name##_prev = NULL; \
                                    } while(0)

typedef struct AvahiHashmap AvahiHashmap;
typedef unsigned (*AvahiHashFunc)(const void *data);
typedef int (*AvahiEqualFunc)(const void *a, const void *b);
typedef void (*AvahiFreeFunc)(void *p);
typedef void (*AvahiHashmapForeachCallback)(void *key, void *value, void *userdata);
typedef struct Entry Entry;

struct Entry {
    AvahiHashmap *hashmap;
    void *key;
    void *value;

    AVAHI_LLIST_FIELDS(Entry, bucket);
    AVAHI_LLIST_FIELDS(Entry, entries);
};

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

public int hashmap_put(hashmap_t *hashmap,char *key,char *data);
public char *hashmap_get(hashmap_t *hashmap,char *key);
public void hashmap_for_each(hashmap_t *hashmap,hashmap_foreach_func func,void *userdata);
public void hashmap_remove(hashmap_t *hashmap,char *key);
public void hashmap_destroy(hashmap_t *hashmap);
public int hashmap_replace(hashmap_t *hashmap,void *key,void *value);

#endif
