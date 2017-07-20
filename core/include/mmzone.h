#include "list.h"
#include "ctype.h"

#ifndef __MM_ZONE_H__
#define __MM_ZONE_H__

#define ZONE_FREE_MAX_ORDER 32

#define ZONE_HIGH_MEMORY 24*1024*1024l

#define GET_FREE_ORDER(x) GET_ORDER(x) - 12 


enum zone_type {
    ZONE_NORMAL,
    ZONE_HIGH, //just like kernel's ZONE_HIGH
    ZONE_MAX
};


typedef struct zone_area 
{
    struct list_head free_page_list;
    struct list_head used_page_list;
    //uint32_t nr_free_pages; no use
}zone_area;


typedef struct mm_zone 
{
    addr_t start_pa;
    addr_t end_pa;
    //use the follow three param to do kswap.
    uint32_t pages_low;
    uint32_t pages_high;
    uint32_t pages_mid;

    //free pages
    uint32_t total_free_pages;
    zone_area nr_area[ZONE_FREE_MAX_ORDER]; //4K,8K,16K
    void (*alloctor_init)(addr_t start_address,uint32_t size);
    void* (*alloctor_get_memory)(uint32_t size);
    void (*alloctor_free)(addr_t address);
    void* (*alloctor_pmem)(uint32_t size);
    void (*alloctor_pmem_free)(addr_t address);
}mm_zone;

mm_zone zone_list[ZONE_MAX];

void *zone_get_page(int type,uint32_t size);
void *zone_get_pmem(size_t size);

#endif
