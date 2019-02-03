/**************************************************************
 CopyRight     :No
 FileName      :mm_initiator.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :mm intiator
 History       
 20190201    * move vmm_init from vmm.c
***************************************************************/

#include "mm.h"
#include "error.h"
#include "task.h"
#include "mmzone.h"
#include "log.h"

extern void *common_vmalloc(mm_struct *pd,size_t bytes);
extern void *common_kmalloc(size_t bytes);
extern void *common_pmalloc(size_t bytes);
extern void *common_malloc(mm_struct *pd,size_t bytes);
extern void common_pfree(addr_t ptr);
extern void common_dealloc(mm_struct *mm,addr_t ptr);
extern void common_kmalloc_cache_init();

#define SIZE_TO_PGD(b) \
    ((b) / (1024*1024*4))

#define SIZE_TO_PTE(b)  \
    ((b/(1024*4)) % (1024) )

memory_range memory_range_core = {
    .start_pgd = 0,
    .start_pte = 0,
};

memory_range memory_range_user = {
    .start_pgd = 0,
    .start_pte = 0,
};

//reserve:mem_avail_begin 
int mm_initiator_start(size_t mem_kb, addr_t krnl_bin_end,size_t reserve)
{
    addr_t i;
    addr_t user_index = 0;

    //we should also update kernel memory range.
    memory_range_core.start_pgd = SIZE_TO_PGD(reserve);
    memory_range_core.start_pte = SIZE_TO_PTE(reserve);

    //we should init memory range
    addr_t user_start_memory = 1024*1024*1024; //may be align 4K
    memory_range_user.start_pgd = SIZE_TO_PGD(user_start_memory);
    memory_range_user.start_pte = SIZE_TO_PTE(user_start_memory); //4K for gully

    core_mem.pgd = process_core_pgd;
    core_mem.pte_core = &process_core_pte[0][0];

#ifdef CORE_PROCESS_USER_SPACE
   core_mem.pte_user = &process_user_pte[0][0];
#endif

    //LOGD("memory_range_core.start_pgd is %d,start pte is %d \n",memory_range_core.start_pgd,memory_range_core.start_pte);
    //LOGD("memory_range_user.start_pgd is %d,start pte is %d \n",memory_range_user.start_pgd,memory_range_user.start_pte);
    
    //map 4G memory, physcial address = virtual address
    for (i = 0; i < memory_range_user.start_pgd; i++) {
        core_mem.pgd[i] = (addr_t)&core_mem.pte_core[i*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    }

#ifdef CORE_PROCESS_USER_SPACE
    for(i = memory_range_user.start_pgd; i < PD_ENTRY_CNT; i++) {
        core_mem.pgd[i] = (addr_t)&core_mem.pte_user[user_index*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        user_index++;
    }
#endif
    //because there is no user memory in core process,
    //needn't init user core memory in core process~
    addr_t *pte = (addr_t *)core_mem.pte_core;
    //core_mem.core_mem_map = &core_mem_reserve_map;
    addr_t core_init_end = memory_range_user.start_pgd*PD_ENTRY_CNT + memory_range_user.start_pte;

    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT/4; i++) {
        pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
        //LOGD("pte[%d] is %d",i,i << 12);
    }

    //LOGD("vmm_init,core_mem.pgd is %x \n",core_mem.pgd);
    //before cr3
    load_pd((addr_t)core_mem.pgd);
    addr_t tmp = rcr3();
    //LOGD("cr3 = %d,core_mem.pgd is %d \n",tmp,core_mem.pgd);
    enable_paging();

    cache_allocator_init();
    //we should create kmalloc cache
    common_kmalloc_cache_init();

    //LOGD("vmm_init trace0 \n");
    cache_free_statistic();

    //reconfig struct mm
    mm_operation.vmalloc = common_vmalloc;
    mm_operation.kmalloc = common_kmalloc;
    mm_operation.malloc = common_malloc;
    mm_operation.pmalloc = common_pmalloc;
    mm_operation.free = common_dealloc;
    mm_operation.pfree = common_pfree;
    //LOGD("vmm_init trace1 \n");

    //init for high memory
    core_mem.vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);
    core_mem.userroot = vm_allocator_init(1024*1024*1024,(uint32_t)1024*1024*1024*3); //user space is 1~3G
    //LOGD("vmm_init trace2 \n");

    return 0;
}
