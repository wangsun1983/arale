/******************************************************************************
 *      Virtual Memory Manager
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"

void *vmm_malloc(mm_struct *pd,size_t bytes);
void *vmm_kmalloc(mm_struct *pd,size_t bytes);

#define PDT_FIND 1
#define PDT_LOSS 2

//static struct vmm_t vmm = {
//    .cur_pd = NULL,
//    .pd_count = 0,
//    .krnl_pt_pa = 0,
//    .krnl_pt_va = 0,
//    .krnl_pt_offset = 0,
//    .krnl_pt_idx = 0,
//    .mem_kb = 0
//};

static mm_struct core_mem;

static void load_pd(addr_t pde)
{
     __asm__ volatile ("mov %0, %%cr3": :"r"(pde));
}

mm_struct *get_root_pd() {
    return &core_mem;
}

static void *alloc_bytes(mm_struct *mm, size_t b, enum mem_area area)
{
    //TODO:area is useless now
    int block = bytes_to_blocks(b);

    //find block
    int pgd = 0;
    int pte = 0;

    for(;pgd < PD_ENTRY_CNT;pgd++)
    {
        pte = 1;
        int find_block = 0;
        //char *pte_t = mm->mem_map[pgd];
        int hit = PDT_LOSS;
        for(;pte < PD_ENTRY_CNT;pte++) {
            //printf("loop block is %d,pgd is %d,pte is %d find_block = %d pte_t[%d] is %d \n",block,pgd,pte,find_block,pte,pte_t[pte]);
            //printf("core_mem.mem_map[%d][%d] is %d \n",pgd,pte,core_mem.mem_map[pgd][pte]);
            //printf("ENTRY_PRESENT is %d \n",ENTRY_PRESENT);
            //printf("result is %d \n",mm->mem_map[pgd][pte] & ENTRY_PRESENT);
            if((mm->mem_map[pgd][pte] & ENTRY_PRESENT) != ENTRY_PRESENT) {
               find_block++;
            }
            //printf("after core_mem.mem_map[%d][%d] is %d \n",pgd,pte,core_mem.mem_map[pgd][pte]);
            //printf("after result is %d \n",mm->mem_map[pgd][pte] & ENTRY_PRESENT);
            //printf("find block is %d,block is %d\n",find_block,block);
            if(find_block == block) {
                hit = PDT_FIND;
                break;
            }
        }
        
        if(hit == PDT_FIND) 
        {
            break;
        }
    }
    printf("find pte is %d \n",pte);
    
    //va is pgd<<22 |pte<<12|ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    pte -= (block - 1);
    //addr_t *pte_t = mm->pte_kern[pgd];
    int start = 0;
    //char *mem_map = mm->mem_map[pgd];

    for(;start<block;start++) {
        addr_t mem = 0;
        if(b/PAGE_SIZE == 0) {
            mem = pmm_alloc(b);
        } else {
            mem = pmm_alloc(PAGE_SIZE);
        }
        //printf("start + pte is %d \n",start + pte);
        
        mm->pte_kern[pgd][start + pte] = mem|ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;//(mem<<12)|ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        mm->mem_map[pgd][start + pte] = ENTRY_PRESENT;
        //printf("mem is %x mm->pte_kern[pgd][start + pte] is %x \n",mem,mm->pte_kern[pgd][start + pte]);
        b -= PAGE_SIZE;
    }
    //printf("pgd is %x \n",pgd<<22);
    //printf("pte is %d \n",pte<<12);
    //printf("pgd ||pte is %d \n",(pgd<<22) |(pte<<12));
    //printf("pgd ||pte is %x \n",pgd<<22 |pte<<12);
    return (pgd<<22) | (pte<<12) ;
}

/*
 * Initializes VMM.
 */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end)
{
    addr_t i;

    // map 4G memory, physcial address = virtual address
    for (i = 0; i < PD_ENTRY_CNT; i++) {
        core_mem.pgd_kern[i] = (addr_t)core_mem.pte_kern[i] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    }
    
    addr_t *pte = (addr_t *)core_mem.pte_kern;
    char *mem_ptr = core_mem.mem_map;

    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT; i++) {
        pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
        mem_ptr[i]= 0;
    }
    //printf("core_mem.pgd_kern is %x,mem_ptr[0] is %d \n",core_mem.pgd_kern,mem_ptr[0]);
    load_pd((addr_t)core_mem.pgd_kern);
    enable_paging();

    mm_operation.malloc = vmm_malloc;
    mm_operation.kmalloc = vmm_kmalloc;
    mm_operation.free = free;

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

inline static size_t get_mark_size(void *mem)
{
    return *((int *) (mem - MEM_MARK_SIZE));
}


/*
 * Frees previously allocated memory chunk.
 */
void free(void *ptr)
{
    size_t b = get_mark_size(ptr);

    if (!b)
        return;

    ptr = unmark_size(ptr);
    //dealloc_bytes(ptr, b);
}

/*
 * Allocates `bytes` sized memory chunk in kernel space.
 */
void *vmm_kmalloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    va = alloc_bytes(mm, bytes + MEM_MARK_SIZE, MEM_KRNL);
    if (!va)
        return 0;
    va = mark_size(va, bytes);
    return va;
}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *vmm_malloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    printf("vmm_malloc start \n");
    va = alloc_bytes(mm, bytes, MEM_USR);
    printf("wangsl1,va is %x \n",va);
    if (!va)
        return 0;
printf("wangsl2,va is %x \n",va);
    va = mark_size(va, bytes);
    printf("wangsl3,va is %x \n",va);
    return va;
}

void enable_paging()
{
    __asm__ __volatile__("mov %%cr0, %%eax \n"
                         "or %0, %%eax \n"
                         "mov %%eax, %%cr0 \n"
                    :
                    : "n" (CR0_ENABLE_PAGING)
                    : "eax");
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

//wangsl
