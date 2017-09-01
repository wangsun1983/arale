/*
 *  cache allocator
 *
 *  Author:sunli.wang
 *
 */

#include "cache_allocator.h"
#include "pmm.h"
#include "klibc.h"

/*
 *
 *
 *
 * Cache Node |CacheNode|pmm_stamp|memory|.....|pmm_stamp|mempory|
 *                |                 |                      |
 *                |                 \                      \
 *                |insert list      start_pa               start_pa
 *                |
 *                |
 * Cache      |free_list|full_list|....
 *
 *
 *
 */
core_mem_cache *creat_core_mem_cache(size_t size)
{
    if(size > CONTENT_SIZE)
    {
        kprintf("too large");
        return NULL;
    }

    core_mem_cache *cache = pmm_kmalloc(sizeof(core_mem_cache));
    cache->objsize = size;

    INIT_LIST_HEAD(&cache->full_list);
    INIT_LIST_HEAD(&cache->partial_list);
    INIT_LIST_HEAD(&cache->free_list);
    INIT_LIST_HEAD(&cache->lru_free_list);

    return cache;
}

static void *get_content(core_mem_cache *cache,core_mem_cache_node *cache_node,int size)
{
    int need_size = size + sizeof(pmm_stamp);
    addr_t start_pa = cache_node->start_pa;
    addr_t end_pa = cache_node->end_pa;

    for(;(start_pa + need_size) < end_pa;start_pa += need_size)
    {
        pmm_stamp *stamp = (pmm_stamp *)start_pa;
        stamp->type = PMM_TYPE_CACHE;

        core_mem_cache_content *content = &stamp->cache_content;
        if(content->is_using == CACHE_FREE)
        {
            cache_node->nr_free--;
            if(cache_node->nr_free == 0)
            {
                list_del(&cache_node->list);
                list_add(&cache_node->list,&cache->full_list);//move this node to full list
            }
            content->head_node = cache_node;
            content->is_using = CACHE_USING;
            content->start_pa = start_pa + sizeof(pmm_stamp);
            return (void *)content->start_pa;
        }
    }

    return NULL;
}

void *cache_alloc(core_mem_cache *cache)
{
    if(!list_empty(&cache->lru_free_list))
    {
        core_mem_cache_content *free_content = list_entry(cache->lru_free_list.next,core_mem_cache_content,ll);

        list_del(&free_content->ll);
        core_mem_cache_node *node = free_content->head_node;
        node->nr_free--;

        if(node->nr_free == 0)
        {
            list_del(&node->list);
            list_add(&node->list,&cache->full_list);//move this node to full list
        }

        free_content->is_using = CACHE_USING;
        return (void *)free_content->start_pa;
    }

    //we should find whether there are partial cache
    if(!list_empty(&cache->partial_list))
    {
        core_mem_cache_node *new_content = list_entry(cache->partial_list.next,core_mem_cache_node,list);
        return get_content(cache,new_content,cache->objsize);
    }

    core_mem_cache_node *cache_node = NULL;
    if(list_empty(&cache->free_list))
    {
        cache_node = pmm_kmalloc(CONTENT_SIZE);
        kmemset(cache_node,0,CONTENT_SIZE);
        cache_node->cache = cache;
        cache_node->start_pa = (addr_t)cache_node + sizeof(core_mem_cache_node);
        cache_node->end_pa = (addr_t)cache_node + CONTENT_SIZE;
    } else {
        cache_node = list_entry(cache->free_list.next,core_mem_cache_node,list);
        list_del(&cache_node->list);
    }

    //INIT_LIST_HEAD(&cache_node->head);
    list_add(&cache_node->list,&cache->partial_list);
    //we should compute free contents
    cache_node->nr_free = (CONTENT_SIZE - sizeof(core_mem_cache_node))/(sizeof(pmm_stamp) + cache->objsize);

    //start get free memory
    return get_content(cache,cache_node,cache->objsize);
}

void cache_free(core_mem_cache *cache,addr_t addr)
{
    pmm_stamp *stamp = (pmm_stamp *)(addr - sizeof(pmm_stamp));
    core_mem_cache_content *content = &stamp->cache_content;

    if(content->is_using == CACHE_FREE)
    {
        //kprintf("free agein \n");
        return;
    }
    content->is_using = CACHE_FREE;
    core_mem_cache_node *node = content->head_node;
    node->nr_free++;
    if(node->nr_free == (CONTENT_SIZE - sizeof(core_mem_cache_node))/(sizeof(pmm_stamp) + cache->objsize))
    {
        list_del(&node->list);
        list_add(&node->list,&cache->free_list);
        return;
    }
    //we add this content to lru list
    list_add(&content->ll,&cache->lru_free_list);
}


void cache_destroy(core_mem_cache *cache)
{
    //start free all node
    struct list_head *p = cache->full_list.next;

    while(p != NULL && p!= &cache->full_list) {
        core_mem_cache_node *node = list_entry(p,core_mem_cache_node,list);
        p = node->list.next;
        free(node);
    }

    p = cache->partial_list.next;
    while(p != NULL && p!= &cache->partial_list) {
        core_mem_cache_node *node = list_entry(p,core_mem_cache_node,list);
        p = node->list.next;
        free(node);
    }

    p = cache->free_list.next;
    while(p != NULL && p!= &cache->free_list) {
        core_mem_cache_node *node = list_entry(p,core_mem_cache_node,list);
        p = node->list.next;
        free(node);
    }

    free(cache);
}
