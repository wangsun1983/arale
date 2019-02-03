/**************************************************************
 CopyRight     :No
 FileName      :mm_common.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :mm common function
 History       
 20190201    * more function from vmm.c
***************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"
#include "mmzone.h"
#include "cache_allocator.h"
#include "sys_observer.h"
#include "pmm.h"
#include "log.h"

void *common_vmalloc(mm_struct *pd,size_t bytes);
void *common_kmalloc(size_t bytes);
void *common_pmalloc(size_t bytes);
void *common_malloc(mm_struct *pd,size_t bytes);
void common_pfree(addr_t ptr);
void common_dealloc(mm_struct *mm,addr_t ptr);

extern void *vmalloc_alloc_bytes(mm_struct *mm,int type,size_t size);

//we should create cache for kmalloc
#define KMALLOC_CACHE_LENGTH 16
core_mem_cache *kmalloc_cache[KMALLOC_CACHE_LENGTH];

bool is_debug_kmalloc = false;

#define KMALLOC_LOG(fmt,...) \
if(is_debug_kmalloc) \
({\
    LOGD(fmt,##__VA_ARGS__);\
})

int kmalloc_cache_init_list[] = {
    16,
    32,
    64,
    128,
    160,
    192,
    224,
    256,
    288,
    320,
    352,
    384,
    416,
    448,
    480,
    512
};

void common_kmalloc_cache_init() 
{
    for(int init_index = 0;init_index < KMALLOC_CACHE_LENGTH;init_index++)
    {
        kmalloc_cache[init_index] = creat_core_mem_cache(kmalloc_cache_init_list[init_index]);
    }
}

/*
 * Frees previously allocated memory chunk.
 */
void common_dealloc(mm_struct *mm,addr_t ptr)
{
    int pageNum = 0;
    int free_zone = pmm_get_dealloc_zone(ptr);
    //LOGD("dealloc 1 ptr is %x \n",ptr);
    switch(free_zone)
    {
        case ZONE_NORMAL:
            //LOGD("dealloc 2 \n");
            pmm_normal_free(ptr);
            break;

        case ZONE_HIGH:
            //LOGD("dealloc 3 \n");
            pageNum = vm_allocator_free(ptr,mm->vmroot);
            pmm_high_free(mm,ptr,pageNum);
            break;
    }
}

//this is used from userspace
void *common_malloc(mm_struct *mm,size_t bytes)
{
    return vmalloc_alloc_bytes(mm, MEM_USR,bytes);
}

/*
 * Allocates `bytes` sized memory chunk in kernel space.
 */
void *common_kmalloc(size_t bytes)
{
    KMALLOC_LOG("vmm_kmalloc start");
    
    //do free memory check first
    uint32_t freemem = pmm_free_mem_statistic();

    KMALLOC_LOG("vmm_kmalloc trace1 \n");
    if(freemem < RECLAIM_MM_NORMAL_THRESHOLD)
    {
        //KMALLOC_LOG("reclaim 1");
        //sys_observer_notify(SYSTEM_EVENT_SHRINK_MEM_NORMAL,NULL);
    }
    else if(freemem < RECLAIM_MM_CRITICAL_THRESHOLD)
    {
        //KMALLOC_LOG("reclaim 2");
        //sys_observer_notify(SYSTEM_EVENT_SHRINK_MEM_CRITICAL,NULL);
    }

    KMALLOC_LOG("vmm_kmalloc trace2 \n");
    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    //we use coalition to record all the memory used,
    //so byte needn't save in the memory.
    if(bytes < kmalloc_cache_init_list[KMALLOC_CACHE_LENGTH - 1])
    {
        //use cache
        KMALLOC_LOG("cache bytes is %d \n",bytes);
        int index = get_cache_index(bytes);
        void *result = cache_alloc(kmalloc_cache[index]);
        if(result != NULL)
        {
            return result;
        }
    }
    KMALLOC_LOG("vmm_kmalloc trace3");
    return pmm_kmalloc(bytes);
}


int get_cache_index(size_t bytes)
{
    int max = bytes/32 + 1;
    for(;max >= 0;max--)
    {
        if(kmalloc_cache_init_list[max] < bytes)
        {
            max++;
            break;
        }
    }

    if(max < 0) {
        max = 0;
    }

    return max;

}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *common_vmalloc(mm_struct *mm,size_t bytes)
{
    return vmalloc_alloc_bytes(mm,MEM_CORE,bytes);
}

void *common_pmalloc(size_t bytes)
{
    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    return (void *)pmm_alloc_pmem(bytes);
}

void common_pfree(addr_t ptr)
{
    LOGD("common_pfree \n");
    pmm_free_pmem(ptr);
}

void enable_paging()
{
    addr_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r" (cr0));
}

void disable_paging()
{
    __asm__ __volatile__("mov %%cr0, %%eax \n"
                         "and %0, %%eax \n"
                         "mov %%eax, %%cr0 \n"
                    :
                    : "n" (~CR0_ENABLE_PAGING)
                    : "eax");
}

void invlpg(void *addr)
{
    //__asm __volatile("invlpg (%0)" : : "r" (addr) : "memory");
        __asm__ volatile ("invlpg (%0)"::"a"(addr));
}

addr_t rcr3(void)
{
    addr_t val;
    __asm __volatile("movl %%cr3,%0" : "=r" (val));
    return val;
}

void refresh_tlb(addr_t *pgd, addr_t va)
{
    // Flush the entry only if we're modifying the current address space.
    invlpg((void*)va);
    LOGD("refresh va is %x \n",va);

    addr_t cr3 = rcr3();

    //__asm __volatile("movl %0,%%cr3" : : "r" (cr3));
    LOGD("cr3 is %x \n",cr3);
    LOGD("pgd is %x \n",pgd);
    load_pd(cr3);
}

void load_pd(addr_t pde)
{
     __asm__ volatile ("mov %0, %%cr3": :"r"(pde));
}

mm_struct *get_root_mm() {
    return &core_mem;
}

addr_t common_lg2(addr_t x)
{
    unsigned int ret;
    __asm__ __volatile__ ("bsrl %1, %%eax":"=a"(ret):"m"(x));
    return ret;
}
