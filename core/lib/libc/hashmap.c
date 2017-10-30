#include "hashmap.h"
#include "klibc.h"
#include "ctype.h"

//Hashmap is porting from avahi,but this is too
//complex to use.we add a interface to make it
//a little simple.

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
AvahiHashmap* avahi_hashmap_new(AvahiHashFunc hash_func, AvahiEqualFunc equal_func, AvahiFreeFunc key_free_func, AvahiFreeFunc value_free_func);
void avahi_hashmap_free(AvahiHashmap *m);
void* avahi_hashmap_lookup(AvahiHashmap *m, const void *key);
int avahi_hashmap_insert(AvahiHashmap *m, void *key, void *value);
int avahi_hashmap_replace(AvahiHashmap *m, void *key, void *value);
void avahi_hashmap_remove(AvahiHashmap *m, const void *key);

typedef void (*AvahiHashmapForeachCallback)(void *key, void *value, void *userdata);

void avahi_hashmap_foreach(AvahiHashmap *m, AvahiHashmapForeachCallback callback, void *userdata);
unsigned avahi_string_hash(const void *data);
int avahi_string_equal(const void *a, const void *b);

unsigned avahi_int_hash(const void *data);
int avahi_int_equal(const void *a, const void *b);


typedef struct Entry Entry;

struct Entry {
    AvahiHashmap *hashmap;
    void *key;
    void *value;

    AVAHI_LLIST_FIELDS(Entry, bucket);
    AVAHI_LLIST_FIELDS(Entry, entries);
};

static Entry* entry_get(AvahiHashmap *m, const void *key) {
    unsigned idx;
    Entry *e;

    idx = m->hash_func(key) % HASH_MAP_SIZE;

    for (e = m->entries[idx]; e; e = e->bucket_next)
        if (m->equal_func(key, e->key))
            return e;

    return NULL;
}

static void entry_free(AvahiHashmap *m, Entry *e, int stolen) {
    unsigned idx;

    idx = m->hash_func(e->key) % HASH_MAP_SIZE;

    AVAHI_LLIST_REMOVE(Entry, bucket, m->entries[idx], e);
    AVAHI_LLIST_REMOVE(Entry, entries, m->entries_list, e);

    if (m->key_free_func)
        m->key_free_func(e->key);
    if (m->value_free_func && !stolen)
        m->value_free_func(e->value);

    avahi_free(e);
}

AvahiHashmap* avahi_hashmap_new(AvahiHashFunc hash_func, AvahiEqualFunc equal_func,
  AvahiFreeFunc key_free_func,
  AvahiFreeFunc value_free_func) {

    AvahiHashmap *m;

    //if (!(m = avahi_new0(AvahiHashmap, 1)))
    //    return NULL;
    m = kmalloc(sizeof(AvahiHashmap));
    kmemset(m,0,sizeof(AvahiHashmap));

    m->hash_func = hash_func;
    m->equal_func = equal_func;
    m->key_free_func = key_free_func;
    m->value_free_func = value_free_func;

    AVAHI_LLIST_HEAD_INIT(Entry, m->entries_list);

    return m;
}

void avahi_hashmap_free(AvahiHashmap *m) {

    while (m->entries_list)
        entry_free(m, m->entries_list, 0);

    avahi_free(m);
}

void* avahi_hashmap_lookup(AvahiHashmap *m, const void *key) {
    Entry *e;

    if (!(e = entry_get(m, key)))
        return NULL;

    return e->value;
}

int avahi_hashmap_insert(AvahiHashmap *m, void *key, void *value) {
    unsigned idx;
    Entry *e;

    if ((e = entry_get(m, key))) {
        if (m->key_free_func)
            m->key_free_func(key);
        if (m->value_free_func)
            m->value_free_func(value);

        return 1;
    }

    //if (!(e = avahi_new(Entry, 1)))
    //    return -1;
    e = kmalloc(sizeof(Entry));
    kmemset(e,0,sizeof(Entry));

    e->hashmap = m;
    e->key = key;
    e->value = value;

    AVAHI_LLIST_PREPEND(Entry, entries, m->entries_list, e);

    idx = m->hash_func(key) % HASH_MAP_SIZE;
    AVAHI_LLIST_PREPEND(Entry, bucket, m->entries[idx], e);

    return 0;
}


int avahi_hashmap_replace(AvahiHashmap *m, void *key, void *value) {
    unsigned idx;
    Entry *e;

    if ((e = entry_get(m, key))) {
       if (m->key_free_func)
            m->key_free_func(e->key);
        if (m->value_free_func)
            m->value_free_func(e->value);

        e->key = key;
        e->value = value;

        return 1;
    }

    //if (!(e = avahi_new(Entry, 1)))
    //    return -1;
    e = kmalloc(sizeof(Entry));
    kmemset(e,0,sizeof(Entry));

    e->hashmap = m;
    e->key = key;
    e->value = value;

    AVAHI_LLIST_PREPEND(Entry, entries, m->entries_list, e);

    idx = m->hash_func(key) % HASH_MAP_SIZE;
    AVAHI_LLIST_PREPEND(Entry, bucket, m->entries[idx], e);

    return 0;
}

void avahi_hashmap_remove(AvahiHashmap *m, const void *key) {
    Entry *e;

    if (!(e = entry_get(m, key)))
        return;

    entry_free(m, e, 0);
}

void avahi_hashmap_foreach(AvahiHashmap *m, AvahiHashmapForeachCallback callback, void *userdata) {
    Entry *e, *next;

    for (e = m->entries_list; e; e = next) {
        next = e->entries_next;

        callback(e->key, e->value, userdata);
    }
}

unsigned avahi_string_hash(const void *data) {
    const char *p = data;
    unsigned hash = 0;

    for (; *p; p++)
        hash = 31 * hash + *p;

    return hash;
}

int avahi_string_equal(const void *a, const void *b) {
    const char *p = a, *q = b;

    return kstrcmp(p, q) == 0;
}

unsigned avahi_int_hash(const void *data) {
    const int *i = data;

    return (unsigned) *i;
}

int avahi_int_equal(const void *a, const void *b) {
   const int *_a = a, *_b = b;
   return *_a == *_b;
}

void avahi_free(char *map)
{
   free(map);
}


public void *hashmap_put(hashmap_t *hashmap,char *key,int keylength,char *data)
{


}

public void *hashmap_get(hashmap_t *hashmap,char *key,int keylength)
{


}

public void *hashmap_for_each(hashmap_t *hashmap,hashmap_foreach_func func)
{


}

public void *hashmap_remove(hashmap_t *hashmap,char *key)
{


}

public void *hashmap_destroy(hashmap_t *hashmap)
{


}

public hashmap_t *hashmap_create(hashmap_hash_func hash_func, hashmap_equal_func equal_func,
  hashmap_free_func key_free_func,
  hashmap_free_func value_free_func)
{


  
}
