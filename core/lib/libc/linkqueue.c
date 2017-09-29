#include "linkqueue.h"
#include "mm.h"
#include "log.h"

link_queue_t *link_queue_create()
{
    link_queue_t *linkqueue = (link_queue_t *)kmalloc(sizeof(link_queue_t));
    kmemset(linkqueue,0,sizeof(link_queue_t));
    INIT_LIST_HEAD(&linkqueue->container);
    linkqueue->tail = &linkqueue->container;
}

void link_queue_add_tail(link_queue_t *queue,void *data)
{
    link_data_t *link = (link_data_t *)kmalloc(sizeof(link_data_t));
    kmemset(link,0,sizeof(link_data_t));
    link->data = data;
    list_add(&link->ll,queue->tail);
    queue->tail = &link->ll;
    queue->size++;
}

void link_queue_add_head(link_queue_t *queue,void *data)
{
    link_data_t *link = (link_data_t *)kmalloc(sizeof(link_data_t));
    kmemset(link,0,sizeof(link_data_t));
    if(list_empty(&queue->container))
    {
        queue->tail = &link->ll;
    }

    link->data = data;
    list_add(&link->ll,&queue->container);
    queue->size++;
}

void *link_queue_remove_head(link_queue_t *queue)
{
    struct list_head *p = queue->container.next;
    if(p == NULL || p == &queue->container)
    {
        return NULL;
    }

    list_del(p);
    link_data_t *link = list_entry(p,link_data_t,ll);
    void *ret = link->data;
    free(link);
    queue->size--;
    return ret;
}

void *link_queue_remove_tail(link_queue_t *queue)
{
    struct list_head *p = queue->tail;
    if(p == NULL || p == &queue->container)
    {
        return NULL;
    }

    queue->tail = p->prev;
    list_del(p);
    link_data_t *link = list_entry(p,link_data_t,ll);
    void *ret = link->data;
    free(link);
    queue->size--;
    return ret;
}

uint32_t link_queue_size(link_queue_t *queue)
{
    return queue->size;
}

void link_queue_destroy(link_queue_t *queue)
{
    free(queue);
}
