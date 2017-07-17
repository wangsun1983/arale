#include <stdio.h>
#include "cache_allocator.h"

#define CONTENT_SIZE 1024*4*8

core_mem_cache *creat_core_mem_cache(int size)
{
    core_mem_cache *cache = malloc(sizeof(core_mem_cache));
    INIT_LIST_HEAD(&cache->full_list);
    INIT_LIST_HEAD(&cache->partial_list);
    INIT_LIST_HEAD(&cache->free_list);
    INIT_LIST_HEAD(&cache->lru_list);
    cache->objsize = size;

    return cache;
}

/*
void cache_list_add(list_head *new,list_head *head)
{
    core_mem_cache_content *new_content = list_entry(new,core_mem_cache_content,ll);
    list_head *p;
    list_head *add_head = head;
    //printf("list_add 1:add_head is %x \n",add_head);

    list_for_each(p,head) {
         core_mem_cache_content *node = list_entry(new,core_mem_cache_content,ll);
         add_head = p;
         //printf("list_add 2: new_page.ll is %x, page.ll is %x head is %x\n",&new_page->ll,&page->ll,head);
         if(new_content < node) {
             //head_page = &page->ll;   
             //list_insert(&new_page->ll,&page->ll.prev,&page->ll.next);
             //printf("list_add 3: p->prev is %x,head is %x \n",p->prev,head);
             add_head = p->prev;
             //return;
         }
    }

    list_add(&new_content->ll,add_head);
}
*/

void *cache_alloc(core_mem_cache *cache)
{
    //we should find whether there are partial cache
    if(!list_empty(cache->partial_list))
    {
        //TODO
        return;
    }

    core_mem_cache_node *cache_node = NULL;
    if(list_empty(cache->free_list)) 
    {
        cache_node = malloc(CONTENT_SIZE);
        cache_node->start_pa = cache_node + sizeof(core_mem_cache_node);
        cache_node->end_pa = cache_node + CONTENT_SIZE;
    } else {
        cache_node = list_entry(cache->free_list.next,core_mem_cache_node,list);
        list_del(&cache_node->list);
    }

    INIT_LIST_HEAD(&cache_node->head);
    list_add(&cache_node->list,&cache->patiral_list);
    //we should compute free contents
    cache_node->nr_free = (CONTENT_SIZE - sizeof(core_mem_cache_node))/(sizeof(core_mem_cache_content) + cache->objsize);
    cache_node->last_addr = cache_node->start_pa + sizeof(core_mem_cache_node);
    //start get free memory
}


static void *get_content(struct list_head *partial_list,int size) 
{
    core_mem_cache_node *cache_node = list_entry(partial_list,core_mem_cache_node,list);
    int need_size = size + sizeof(core_mem_cache_content);

    if((cache_node->last_addr + need_size) < cache_node->end_pa)
    {
        core_mem_cache_content *content = cache_node->last_addr;
        cache_node->last_addr += need_size;
        //list_add(partial_list,&content->ll);
        //we should change list
        //TODO
        
        return content + sizeof(core_mem_cache_content);
    } 

    //we should find a free content from partial list
    struct list_head *p;
    list_for_each(p,partial_list) {
        core_mem_cache_content *content = list_entry(p,core_mem_cache_content,ll);
        list_head *next = &content->ll.next;
        if(next != partial_list) {
            core_mem_cache_content *next_content = list_entry(next,core_mem_cache_content,ll);
            if((next_content - content) > (size + sizeof(core_mem_cache_content)))
            {
                core_mem_cache_content *select = content + size + sizeof(core_mem_cache_content);
                //TODO
                //list_add(&select->ll,partial_list);
                content->ll.next = &select->ll;
                select->ll.prev = &content->ll;
                select->next = &next_content->ll;
                next_content->ll.prev = &select->ll;
            }
        }
    }
}


int main()
{
    //TODO


}
