/******************************************************************************
 *      Virtual Memory Manager
 *
 ******************************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"
#include "mmzone.h"
#include "cache_allocator.h"
#include "sys_observer.h"
#include "pmm.h"

void *vmm_vmalloc(mm_struct *pd,size_t bytes);
void *vmm_kmalloc(size_t bytes);
void *vmm_pmalloc(size_t bytes);
void *vmm_malloc(mm_struct *pd,size_t bytes);
void vmm_pfree(addr_t ptr);
void dealloc(mm_struct *mm,addr_t ptr);

extern vm_root * vm_allocator_init(addr_t start_addr,uint32_t size);
extern addr_t vm_allocator_alloc(uint32_t size,vm_root *vmroot);
extern int vm_allocator_free(addr_t addr,vm_root *vmroot);

//we should create cache for kmalloc
#define KMALLOC_CACHE_LENGTH 16
core_mem_cache *kmalloc_cache[KMALLOC_CACHE_LENGTH];

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


#define SIZE_TO_PGD(b) \
    ((b) / (1024*1024*4))

#define SIZE_TO_PTE(b)  \
    ((b/(1024*4)) % (1024) )

#define PDT_FIND 1
#define PDT_LOSS 2

memory_range memory_range_core = {
    .start_pgd = 0,
    .start_pte = 0,
};

memory_range memory_range_user = {
    .start_pgd = 0,
    .start_pte = 0,
};

int scan_start_pgd = 0;
int scan_start_pte = 0;

void dealloc(mm_struct *mm,addr_t ptr);

void load_pd(addr_t pde)
{
     __asm__ volatile ("mov %0, %%cr3": :"r"(pde));
}

mm_struct *get_root_mm() {
    return &core_mem;
}

static void *vmalloc_alloc_bytes(mm_struct *mm,int type,size_t size)
{
    int block = bytes_to_blocks(size);
    uint32_t page_size = block*PAGE_SIZE;
    int i = 0;

    vm_root *vmroot = NULL;
    switch(type)
    {
        case MEM_USR:
            vmroot = mm->userroot;
            break;

        case MEM_CORE:
            vmroot = mm->vmroot;
            break;

        default:
            return NULL;
    }

    addr_t va = vm_allocator_alloc(page_size,vmroot);

    addr_t start_pgd = va_to_pt_idx((addr_t)va);
    addr_t start_pte = va_to_pte_idx((addr_t)va);
    addr_t pgd = va_to_pt_idx(va + page_size);
    addr_t pte = va_to_pte_idx(va + page_size);

    //kprintf("vmalloc_alloc_bytes trace1 va is %x,start_pgd is %d,start_pte is %d,end_pgd is %d,end_pte is %d,\n",
    //       va,start_pgd,start_pte,pgd,pte);

    switch(type)
    {
        case MEM_USR:
            start_pgd = start_pgd - memory_range_user.start_pgd;
            start_pte = start_pte - memory_range_user.start_pte;
            pgd -= memory_range_user.start_pgd;
            pte -= memory_range_user.start_pte;
            //kprintf("vmalloc_alloc_bytes trace2 va is %x,start_pgd is %d,start_pte is %d,end_pgd is %d,end_pte is %d,\n",
            //     va,start_pgd,start_pte,pgd,pte);

            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = (addr_t)zone_get_page(ZONE_HIGH,PAGE_SIZE);
                //kprintf("mem is %x,i is %d \n",mem,i);
                mm->pte_user[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
                size -= PAGE_SIZE;
                if(size < 0) {
                    break;
                }
            }
            //kprintf("vmalloc_alloc_bytes trace2 \n");
            break;

        case MEM_CORE:
            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = (addr_t)zone_get_page(ZONE_HIGH,PAGE_SIZE);
                mm->pte_core[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
                size -= PAGE_SIZE;
                if(size < 0) {
                    break;
                }
            }
            break;

        default:
            return NULL;
    }

    //load_pd((addr_t)mm->pgd);

    return (void *)va;
}

int vmm_init(size_t mem_kb, addr_t krnl_bin_end,size_t reserve)
{
    addr_t i;
    addr_t user_index = 0;

    //we should also update kernel memory range
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
    }
    kprintf("vmm_init abc \n");
    //before cr3
    load_pd((addr_t)core_mem.pgd);
    enable_paging();

    cache_allocator_init();
    //we should create kmalloc cache
    int init_index = 0;
    //kprintf("vmm_init cache \n");
    for(init_index = 0;init_index < KMALLOC_CACHE_LENGTH;init_index++)
    {
        kmalloc_cache[init_index] = creat_core_mem_cache(kmalloc_cache_init_list[init_index]);
    }
    //kprintf("vmm_init trace0 \n");
    //reconfig struct mm
    mm_operation.vmalloc = vmm_vmalloc;
    mm_operation.kmalloc = vmm_kmalloc;
    mm_operation.malloc = vmm_malloc;
    mm_operation.pmalloc = vmm_pmalloc;
    mm_operation.free = dealloc;
    mm_operation.pfree = vmm_pfree;
    //kprintf("vmm_init trace1 \n");
    //init for high memory
    core_mem.vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);
    core_mem.userroot = vm_allocator_init(1024*1024*1024,(uint32_t)1024*1024*1024*3); //user space is 1~3G
    //kprintf("vmm_init trace2 \n");

    return 0;
}

/*
 * Frees previously allocated memory chunk.
 */
void dealloc(mm_struct *mm,addr_t ptr)
{
    int pageNum = 0;
    int free_zone = pmm_get_dealloc_zone(ptr);
    //kprintf("dealloc 1 \n");
    switch(free_zone)
    {
        case ZONE_NORMAL:
            //kprintf("dealloc 2 \n");
            pmm_normal_free(ptr);
            break;

        case ZONE_HIGH:
            pageNum = vm_allocator_free(ptr,mm->vmroot);
            pmm_high_free(mm,ptr,pageNum);
            break;
    }
}

//this is used from userspace
void *vmm_malloc(mm_struct *mm,size_t bytes)
{
    return vmalloc_alloc_bytes(mm, MEM_USR,bytes);
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
 * Allocates `bytes` sized memory chunk in kernel space.
 */
void *vmm_kmalloc(size_t bytes)
{
    //do free memory check first
    uint32_t freemem = pmm_free_mem_statistic();
    if(freemem < RECLAIM_MM_NORMAL_THRESHOLD)
    {
        //kprintf("reclaim 1");
        sys_observer_notify(SYSTEM_EVENT_SHRINK_MEM_NORMAL,NULL);
    }
    else if(freemem < RECLAIM_MM_CRITICAL_THRESHOLD)
    {
        //kprintf("reclaim 2");
        sys_observer_notify(SYSTEM_EVENT_SHRINK_MEM_CRITICAL,NULL);
    }

    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    //we use coalition to record all the memory used,
    //so byte needn't save in the memory.
    if(bytes < kmalloc_cache_init_list[KMALLOC_CACHE_LENGTH - 1])
    {
        //use cache
        //kprintf("cache bytes is %d \n",bytes);
        int index = get_cache_index(bytes);
        void *result = cache_alloc(kmalloc_cache[index]);
        if(result != NULL)
        {
            return result;
        }
    }

    return pmm_kmalloc(bytes);
}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *vmm_vmalloc(mm_struct *mm,size_t bytes)
{
    return vmalloc_alloc_bytes(mm,MEM_CORE,bytes);
}

void *vmm_pmalloc(size_t bytes)
{
    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    return (void *)pmm_alloc_pmem(bytes);
}

void vmm_pfree(addr_t ptr)
{
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
    kprintf("refresh va is %x \n",va);

    addr_t cr3 = rcr3();

    //__asm __volatile("movl %0,%%cr3" : : "r" (cr3));
    kprintf("cr3 is %x \n",cr3);
    kprintf("pgd is %x \n",pgd);
    load_pd(cr3);
}
