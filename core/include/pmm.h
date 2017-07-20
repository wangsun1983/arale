#include "ctype.h"
#include "mm.h"
#include "page.h"
#include "cache_allocator.h"

#ifndef _PMM_H_
#define _PMM_H_

enum PMM_TYPE {
    PMM_TYPE_NORMAL = 0,
    PMM_TYPE_PMEM,
    PMM_TYPE_CACHE
};


typedef struct pmm_stamp
{
    int type;
    union 
    {
        mm_page page;
        core_mem_cache_content cache_content;
    };
}pmm_stamp;


void *pmm_kmalloc(size_t bytes);

#endif
