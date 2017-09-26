#include "test_mm.h"
#include "cache_allocator.h"
#include "mm.h"
#include "test_utils.h"

struct mm_cache_test_data
{
    struct mm_cache_test_data *next;
    int index;
    char pad[233];
};

struct mm_cache_test_data *head = NULL;
struct mm_cache_test_data *current = NULL;

int test_mm_cache_reclaim()
{
    core_mem_cache *cache = creat_core_mem_cache(sizeof(struct mm_cache_test_data));
    //we start do cache alloc && free
    int domalloc = 1024;
    while(domalloc > 0)
    {
        struct mm_cache_test_data *data = cache_alloc(cache);
        if(head == NULL)
        {
            head = data;
            current = head;
        }
        else
        {
            current->next = data;
            current = current->next;
        }
        domalloc--;
    }

    //start free
    struct mm_cache_test_data *p = head;
    while(p != current)
    {
        cache_free(cache,(addr_t)p);
        p = p->next;
    }

    //cout cache's free list
    struct list_head *hh;
    int free_count = 0;
    list_for_each(hh,&cache->free_list) {
        free_count++;
    }

    if(free_count == 0)
    {
         return -1;
    }

    //do more alloc for reclaim
    domalloc = 20;
    head = NULL;
    current = NULL;

    while(domalloc > 0)
    {
        if(head == NULL)
        {
            head = (struct mm_cache_test_data *)kmalloc(1024*1024);
            current = head;
        }
        else
        {
            current->next = (struct mm_cache_test_data *)kmalloc(1024*1024);
            if(current->next == NULL)
            {
                break;
            }
            current = current->next;
        }
        domalloc--;
    }

    free_count = 0;
    list_for_each(hh,&cache->free_list) {
        free_count++;
    }

    //start free all the memory
    struct mm_cache_test_data *freep = head;
    while(freep != NULL)
    {
        struct mm_cache_test_data *next = freep->next;
        free(freep);
        freep = next;
    }

    if(free_count > 0)
    {
        return -1;
    }

    return  1;
}


int test_mm_reclaim()
{
    TEST_ASSERT(test_mm_cache_reclaim);
}
