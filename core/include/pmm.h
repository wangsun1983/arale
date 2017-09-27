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
int pmm_get_dealloc_zone(addr_t address);
void pmm_normal_free(addr_t address);
void pmm_high_free(mm_struct *mm,addr_t ptr,int pageNum);
addr_t pmm_alloc_pmem(size_t bytes);
void pmm_free_pmem(addr_t addr);
uint32_t pmm_free_mem_statistic();

#endif
