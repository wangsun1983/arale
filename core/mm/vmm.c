/******************************************************************************
 *      Virtual Memory Manager
 *
 ******************************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"
#include "mmzone.h"

void *vmm_vmalloc(mm_struct *pd,size_t bytes);
void *vmm_kmalloc(mm_struct *pd,size_t bytes);
void *vmm_malloc(mm_struct *pd,size_t bytes);

extern vm_root * vm_allocator_init(addr_t start_addr,uint32_t size);
extern addr_t vm_allocator_alloc(uint32_t size,vm_root *vmroot);
extern void vm_allocator_free(addr_t addr,vm_root *vmroot);


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

mm_struct *get_root_pd() {
    return &core_mem.pgd;
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

    printf("vmalloc_alloc_bytes trace1 va is %x,start_pgd is %d,start_pte is %d,end_pgd is %d,end_pte is %d,\n",
           va,start_pgd,start_pte,pgd,pte);
    
    switch(type) 
    {
        case MEM_USR:
            start_pgd = start_pgd - memory_range_user.start_pgd;
            start_pte = start_pte - memory_range_user.start_pte;
            pgd -= memory_range_user.start_pgd;
            pte -= memory_range_user.start_pte;
            printf("vmalloc_alloc_bytes trace2 va is %x,start_pgd is %d,start_pte is %d,end_pgd is %d,end_pte is %d,\n",
                 va,start_pgd,start_pte,pgd,pte);

            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = zone_get_page(ZONE_HIGH,PAGE_SIZE);
                //printf("mem is %x,i is %d \n",mem,i);
                mm->pte_user[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
                size -= PAGE_SIZE;
                if(size < 0) {
                    break;
                }
            }
            printf("vmalloc_alloc_bytes trace2 \n");
            break;

        case MEM_CORE:
            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = zone_get_page(ZONE_HIGH,PAGE_SIZE);
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

    return va;
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

    core_mem.pgd = &process_core_pgd;
    core_mem.pte_core = &process_core_pte;

#ifdef CORE_PROCESS_USER_SPACE
   core_mem.pte_user = &process_user_pte;
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

    //before cr3
    load_pd((addr_t)core_mem.pgd);
    enable_paging();

    //reconfig struct mm
    mm_operation.vmalloc = vmm_vmalloc;
    mm_operation.kmalloc = vmm_kmalloc;
    mm_operation.malloc = vmm_malloc;
    mm_operation.free = dealloc;

    //init for high memory
    core_mem.vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);
    core_mem.userroot = vm_allocator_init(1024*1024*1024,1024*1024*1024*3); //user space is 1~3G
    
    return 0;
}

/*
 * Every freshly allocated memory chunk will have the first DW
 * reserved for keeping track the allocation size in bytes,
 * which will be used when calling free().
 */
inline static void *mark_size(void *mem, size_t bytes)
{
    *((int *) mem) = bytes;

    return mem + MEM_MARK_SIZE;
}

inline static void *unmark_size(void *mem)
{
    return mem - MEM_MARK_SIZE;
}

//first 4K for size
inline static size_t get_mark_size(void *mem)
{
    return *((int *) (mem - MEM_MARK_SIZE));
}


/*
 * Frees previously allocated memory chunk.
 */
void dealloc(mm_struct *mm,addr_t ptr)
{
    size_t b = get_mark_size(ptr);
    if (!b)
        return;

    ptr = unmark_size(ptr);
    //we should distribute the vm address
    if(ptr < zone_list[ZONE_NORMAL].end_pa) 
    {
        zone_list[ZONE_NORMAL].alloctor_free(ptr);
    } 
    else
    {
        vm_allocator_free(ptr,mm->vmroot);
        addr_t lptr = ptr;

        for(;lptr < ptr + b;lptr+=PAGE_SIZE)
        {
            //get physical address
            int pt = va_to_pt_idx(lptr);
            int pte = va_to_pte_idx(lptr);
            addr_t pa = mm->pte_core[pt*PD_ENTRY_CNT + pte];

            zone_list[ZONE_HIGH].alloctor_free(pa);
        }   
    }
}

//this is used from userspace
void *vmm_malloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    printf("vmm_malloc start \n");
    va = vmalloc_alloc_bytes(mm, MEM_USR,bytes);
    printf("vmm_malloc trace \n");
    if (!va)
        return 0;
    printf("vmm_malloc va is %x \n",va);
    va = mark_size(va, bytes);
    printf("vmm_malloc2 va is %x \n",va);

    return va;
}


/*
 * Allocates `bytes` sized memory chunk in kernel space.
 */
void *vmm_kmalloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    va = zone_get_page(ZONE_NORMAL,bytes);
    va = mark_size(va, bytes);

    if (!va)
        return 0;

    return va;
}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *vmm_vmalloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;

    va = vmalloc_alloc_bytes(mm,MEM_CORE,bytes);
    if (!va)
        return 0;

    va = mark_size(va, bytes);

    return va;

}


//PGD needs a continus memory,and the start address 
//must be the start of 4K.
inline static void *mark_pdg_size(void *mem, size_t bytes) {

    char * p = mem + PAGE_SIZE - MEM_MARK_SIZE;
    *p = bytes;

    return mem + PAGE_SIZE;
}

void *create_task_pgd(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += PAGE_SIZE;
    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    va = zone_get_pmem(bytes);

    if (!va)
        return 0;

    va = mark_pdg_size(va, bytes);

    return va;
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
    printf("refresh va is %x \n",va);

    addr_t cr3 = rcr3();

    //__asm __volatile("movl %0,%%cr3" : : "r" (cr3));
    printf("cr3 is %x \n",cr3);
    printf("pgd is %x \n",pgd);
    load_pd(cr3);
}
