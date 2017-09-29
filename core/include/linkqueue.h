#ifndef __LINK_QUEUE_H__
#define __LINK_QUEUE_H__

#include "ctype.h"
#include "list.h"

typedef struct link_queue {
    struct list_head container;
    struct list_head *tail;
    uint32_t size;
}link_queue_t;

typedef struct link_data {
    struct list_head ll;
    void *data;
}link_data_t;

public link_queue_t *link_queue_create();
public void link_queue_add_tail(link_queue_t *queue,void *data);
public void link_queue_add_head(link_queue_t *queue,void *data);
public void *link_queue_remove_head(link_queue_t *queue);
public void *link_queue_remove_tail(link_queue_t *queue);
public void link_queue_destroy(link_queue_t *queue);
public uint32_t link_queue_size(link_queue_t *queue);

#endif
