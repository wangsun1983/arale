#include "mmzone.h"
#include "mm.h"
#include "coalition_allocator.h"
#include "log.h"
#include "mm_common.h"

extern void* get_fragment_page();
extern int free_fragment_page(uint32_t page_addr);
extern void fragment_allocator_init(uint32_t start_addr,uint32_t size);

void mm_zone_init(uint32_t addr,size_t size)
{
    //ZONE_NORMAL memory is used for kernel
    //ZONE_HIGH memory is used for userspace
    zone_list[ZONE_NORMAL].start_pa = PAGE_SIZE_RUND_UP(addr);
    if((addr + ZONE_HIGH_MEMORY) > size) {
        zone_list[ZONE_NORMAL].end_pa = addr + size/2;
    } else {
        zone_list[ZONE_NORMAL].end_pa = addr + ZONE_HIGH_MEMORY;
    }
    
    zone_list[ZONE_NORMAL].alloctor_init = coalition_allocator_init;
    zone_list[ZONE_NORMAL].alloctor_get_memory = coalition_malloc;
    zone_list[ZONE_NORMAL].alloctor_free = coalition_free;

    zone_list[ZONE_NORMAL].alloctor_pmem = pmem_malloc;
    zone_list[ZONE_NORMAL].alloctor_pmem_free = pmem_free;
    zone_list[ZONE_NORMAL].free_mem = zone_list[ZONE_NORMAL].end_pa - zone_list[ZONE_NORMAL].start_pa;
    zone_list[ZONE_NORMAL].alloctor_init(zone_list[ZONE_NORMAL].start_pa,size - ZONE_HIGH_MEMORY);
    //LOGD("alloctor init addr is %d,size is %d \n",zone_list[ZONE_NORMAL].start_pa,size-ZONE_HIGH_MEMORY);

    //high memory,high memory's memory alloctor need rewrite....
    zone_list[ZONE_HIGH].start_pa = zone_list[ZONE_NORMAL].end_pa + PAGE_SIZE;
    zone_list[ZONE_HIGH].end_pa = addr + size;
    zone_list[ZONE_HIGH].alloctor_init = fragment_allocator_init;
    zone_list[ZONE_HIGH].alloctor_get_memory = get_fragment_page;
    zone_list[ZONE_HIGH].alloctor_free = free_fragment_page;
    zone_list[ZONE_HIGH].free_mem = zone_list[ZONE_HIGH].end_pa - zone_list[ZONE_HIGH].start_pa;
    zone_list[ZONE_HIGH].alloctor_init(zone_list[ZONE_HIGH].start_pa,size - zone_list[ZONE_HIGH].start_pa);

}

void *zone_get_page(int type,uint32_t size)
{
    int *alloc_size = 0;
    void *ret = zone_list[type].alloctor_get_memory(size,alloc_size);
    zone_list[type].free_mem -= *alloc_size;

    return ret;
}

void zone_free_page(int type,addr_t ptr)
{
    LOGD("zone_free_page ptr is %x,type is %d\n",ptr,type);
    zone_list[type].alloctor_free(ptr);
}

void *zone_get_pmem(size_t addr)
{
    int *alloc_size = 0;
    void *ret = zone_list[ZONE_NORMAL].alloctor_pmem(addr,alloc_size);
    zone_list[ZONE_NORMAL].free_mem -= *alloc_size;
    return ret;
}

void zone_free_pmem(addr_t addr)
{
    zone_list[ZONE_NORMAL].alloctor_pmem_free(addr);
}

uint32_t zone_free_mem_statistic()
{
    //LOGD("zone_free_mem_statistic \n");
    return zone_list[ZONE_NORMAL].free_mem + zone_list[ZONE_HIGH].free_mem;
}
