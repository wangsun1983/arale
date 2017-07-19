#include "cache_allocator.h"
#include "pmm.h"

core_mem_cache *creat_core_mem_cache(int size)
{
    if(size > CONTENT_SIZE) 
    {
        printf("too large");
        return NULL;
    }

    core_mem_cache *cache = pmm_kmalloc(sizeof(core_mem_cache));
    INIT_LIST_HEAD(&cache->full_list);
    INIT_LIST_HEAD(&cache->partial_list);
    INIT_LIST_HEAD(&cache->free_list);
    INIT_LIST_HEAD(&cache->lru_free_list);
    cache->objsize = size;

    return cache;
}

static void *get_content(core_mem_cache *cache,core_mem_cache_node *cache_node,int size) 
{
    int need_size = size + sizeof(core_mem_cache_content);
    addr_t start_pa = cache_node->start_pa;
    addr_t end_pa = cache_node->end_pa;

    for(;start_pa<end_pa;start_pa += need_size)
    {
        core_mem_cache_content *content = start_pa;
        if(content->is_using == CACHE_FREE) 
        {
            cache_node->nr_free--;
            if(cache_node->nr_free == 0)
            {
                //printf("get_content trace1 \n");
                list_del(&cache_node->list);
                list_add(&cache_node->list,&cache->full_list);//move this node to full list
            }
            content->head_node = cache_node;
            content->is_using = CACHE_USING;
            //printf("get_content start_pa is %x,core_mem_cache_content size is %x,cache_node->nr_free is %d \n",
            //        start_pa,sizeof(core_mem_cache_content),cache_node->nr_free);
            return start_pa + sizeof(core_mem_cache_content);         
        }
    }
    
    return NULL;
}

void *cache_alloc(core_mem_cache *cache)
{
    //first we can get from lru_free_list
    //printf("cache_alloc start \n");
    if(!list_empty(&cache->lru_free_list))
    {
        //printf("cache_alloc trace1 \n");
        core_mem_cache_content *free_content = list_entry(cache->lru_free_list.next,core_mem_cache_content,ll);
        //printf("cache_alloc trace2 free_content = %x,core_mem_cache_content size is %x \n",free_content,sizeof(core_mem_cache_content));

        list_del(&free_content->ll);
        core_mem_cache_node *node = free_content->head_node;
        node->nr_free--;

        if(node->nr_free == 0)
        {
            list_del(&node->list);
            list_add(&node->list,&cache->full_list);//move this node to full list
        }

        free_content->is_using = CACHE_USING;
        return (addr_t)free_content + sizeof(core_mem_cache_content);
    }

    //we should find whether there are partial cache
    if(!list_empty(&cache->partial_list))
    {
        //printf("cache_alloc trace3 \n");
        core_mem_cache_node *new_content = list_entry(cache->partial_list.next,core_mem_cache_node,list);
        return get_content(cache,new_content,cache->objsize);
    }

    core_mem_cache_node *cache_node = NULL;
    if(list_empty(&cache->free_list)) 
    {
        //printf("cache_alloc trace4 \n");
        //cache_node = kmalloc(CONTENT_SIZE);
        cache_node = pmm_kmalloc(CONTENT_SIZE);
        memset(cache_node,0,CONTENT_SIZE);

        cache_node->start_pa = (addr_t)cache_node + sizeof(core_mem_cache_node);
        cache_node->end_pa = cache_node + CONTENT_SIZE;
    } else {
        //printf("cache_alloc trace5 \n");
        cache_node = list_entry(cache->free_list.next,core_mem_cache_node,list);
        list_del(&cache_node->list);
    }

    //INIT_LIST_HEAD(&cache_node->head);
    list_add(&cache_node->list,&cache->partial_list);
    //we should compute free contents
    cache_node->nr_free = (CONTENT_SIZE - sizeof(core_mem_cache_node))/(sizeof(core_mem_cache_content) + cache->objsize);
    //printf("cache_node->nr_free is %d \n",cache_node->nr_free);
    //start get free memory
    return get_content(cache,cache_node,cache->objsize);
}

void cache_free(core_mem_cache *cache,size_t addr)
{
    core_mem_cache_content *content = addr - sizeof(core_mem_cache_content);
    if(content->is_using == CACHE_FREE) 
    {
        //printf("free agein \n");
        return;
    }
    content->is_using = CACHE_FREE;
    core_mem_cache_node *node = content->head_node;
    node->nr_free++;
    if(node->nr_free == (CONTENT_SIZE - sizeof(core_mem_cache_node))/(sizeof(core_mem_cache_content) + cache->objsize))
    {
        list_del(&node->list);
        list_add(&node->list,&cache->free_list);
        //printf("cache_free trace1 \n");
        return;
    }
    //we add this content to lru list 
    //printf("cache_free trace1,free conent is %x \n",content);
    list_add(&content->ll,&cache->lru_free_list);
}


void cache_destroy(core_mem_cache *cache)
{
    //start free all node
    struct list_head *p;
    list_for_each(p,&cache->full_list) {
         core_mem_cache_node *node = list_entry(p,core_mem_cache_node,list);
         free(node);
    }

    list_for_each(p,&cache->partial_list) {
         core_mem_cache_node *node = list_entry(p,core_mem_cache_node,list);
         free(node);
    }

    list_for_each(p,&cache->free_list) {
         core_mem_cache_node *node = list_entry(p,core_mem_cache_node,list);
         free(node);
    }

    free(cache);
}


#if 0
struct abc 
{
    int i;
};


int main()
{
    core_mem_cache *cache = creat_core_mem_cache(512);
    struct abc *t1 = cache_alloc(cache);
    struct abc *t2 = cache_alloc(cache);
    struct abc *t3 = cache_alloc(cache);
    printf("t1 is %x \n",t1);
    printf("t2 is %x \n",t2);
    printf("t3 is %x \n",t3);

    cache_free(cache,t1);
    cache_free(cache,t2);
    cache_free(cache,t3);

    struct abc *t4 = cache_alloc(cache);
    struct abc *t5 = cache_alloc(cache);
    struct abc *t6 = cache_alloc(cache);
    printf("t4 is %x \n",t4);
    printf("t5 is %x \n",t5);
    printf("t6 is %x \n",t6);
    
}
#endif
