#include "pmm.h"
#include "mmzone.h"
#include "cache_allocator.h"
#include "vmm.h"
#include "log.h"

void *pmm_kmalloc(size_t bytes)
{
    return zone_get_page(ZONE_NORMAL,bytes);
}

int pmm_get_dealloc_zone(addr_t ptr)
{
    if(ptr < zone_list[ZONE_NORMAL].end_pa)
    {
        return ZONE_NORMAL;
    }

    return ZONE_HIGH;
}

void pmm_normal_free(addr_t ptr)
{
    pmm_stamp *stamp = (pmm_stamp *)(ptr - sizeof(pmm_stamp));
    core_mem_cache_content *content;
    core_mem_cache *cache;

    switch(stamp->type)
    {
        case PMM_TYPE_NORMAL:
            zone_list[ZONE_NORMAL].alloctor_free(ptr);
            break;

        case PMM_TYPE_PMEM:
            //PMEM need use special free
            break;

        case PMM_TYPE_CACHE:
            content = &stamp->cache_content;
            cache = content->head_node->cache;
            cache_free(cache,ptr);
            break;
    }
}


void pmm_high_free(mm_struct *mm,addr_t ptr,int pageNum)
{
    int start_page = 0;

    for(;start_page < pageNum;start_page++)
    {
        //get physical address
        addr_t lptr = ptr + start_page*PAGE_SIZE;
        int pt = va_to_pt_idx(lptr);
        int pte = va_to_pte_idx(lptr);
        addr_t pa = mm->pte_core[pt*PD_ENTRY_CNT + pte];
        zone_list[ZONE_HIGH].alloctor_free(pa);
    }
}

addr_t pmm_alloc_pmem(size_t bytes)
{
    return (addr_t)zone_get_pmem(bytes);
}

void pmm_free_pmem(addr_t addr)
{
    zone_free_pmem(addr);
}

uint32_t pmm_free_mem_statistic()
{
    //LOGD("pmm_free_mem_statistic \n");
    return zone_free_mem_statistic() + cache_free_statistic();
}
