#include "mmzone.h"
#include "mm.h"

extern void* coalition_malloc(uint32_t size);
extern void coalition_free(uint32_t address);
extern void coalition_allocator_init(uint32_t start_address,uint32_t size);

void mm_zone_init(uint32_t addr,size_t size)
{
    //we give 24M for high memory
    //the other used for normal memory
    //normal memory
    zone_list[ZONE_NORMAL].start_pa = addr;
    zone_list[ZONE_NORMAL].end_pa = addr + size - ZONE_HIGH_MEMROY;
    zone_list[ZONE_NORMAL].alloctor_init = coalition_allocator_init;
    zone_list[ZONE_NORMAL].alloctor_get_memory = coalition_malloc;
    zone_list[ZONE_NORMAL].alloctor_free = coalition_free;
    zone_list[ZONE_NORMAL].alloctor_init(addr,size - ZONE_HIGH_MEMROY);

 
    //high memory,high memory's memory alloctor need rewrite....
    //TODO,haha
    zone_list[ZONE_HIGH].start_pa = zone_list[ZONE_NORMAL].end_pa + 1;
    zone_list[ZONE_HIGH].end_pa = addr + size;

}

void *zone_get_page(int type,uint32_t size)
{
    return zone_list[type].alloctor_get_memory(size);
}



