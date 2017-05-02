/******************************************************************************
 *      Virtual Memory Manager
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"

void *vmm_malloc(size_t  *pd,size_t bytes);
void *vmm_kmalloc(size_t *pd,size_t bytes);

static struct vmm_t vmm = {
    .cur_pd = NULL,
    .pd_count = 0,
    .krnl_pt_pa = 0,
    .krnl_pt_va = 0,
    .krnl_pt_offset = 0,
    .krnl_pt_idx = 0,
    .mem_kb = 0
};

static struct vmm_s vmm_s;

static void load_pd(addr_t pde)
{
     __asm__ volatile ("mov %0, %%cr3": :"r"(pde));
}

addr_t *get_root_pd() {
    return vmm_s.pgd_kern;
}

static void *alloc_bytes(addr_t *pd, size_t b, enum mem_area area)
{
    //TODO:area is useless now
    int block = bytes_to_blocks(b);

    //find block
    int pgd = 0;
    int pte = 0;

    for(;pgd < PD_ENTRY_CNT;pgd++)
    {
        pte = 0;
        int find_block = 0;
        int *pte_t = pd[pgd];
        for(;pte < PD_ENTRY_CNT;pte++) {
            if((pte_t[pte]&(pte <<12) == pte_t[pte])
               ||(pte_t[pte]&ENTRY_PRESENT !=0)) {
               find_block++;
            }

            if(find_block == block) {
                break;
            }
        }
    }
    
    //va is pgd<<22 |pte<<12|ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    pte -= (block - 1);
    int *pte_t = pd[pgd];
    int start = 0;
    for(;start<block;start++) {
        addr_t mm = 0;
        if(b/PAGE_SIZE == 0) {
            mm = pmm_alloc(b);
        } else {
            mm = pmm_alloc(PAGE_SIZE);
        }
        
        printf("pa is %x \n",mm);
        pte_t[start + pte] = mm;

        b -= PAGE_SIZE;
    }
    printf("pgd is %d \n",pgd);
    printf("pte is %d \n",pte);
    return pgd<<22 | pte<<12 | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;;
}

/*
 * Initializes VMM.
 */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end)
{
    addr_t i;

    // map 4G memory, physcial address = virtual address
    for (i = 0; i < PD_ENTRY_CNT; i++) {
        vmm_s.pgd_kern[i] = (addr_t)vmm_s.pte_kern[i] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    }
    
    addr_t *pte = (addr_t *)vmm_s.pte_kern;

    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT; i++) {
        pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
    }
    printf("vmm_s.pgd_kern is %x \n",vmm_s.pgd_kern);
    load_pd((addr_t)vmm_s.pgd_kern);
    enable_paging();
    
//wangsl
    mm_operation.malloc = vmm_malloc;
    mm_operation.kmalloc = vmm_kmalloc;
    mm_operation.free = free;
//wangsl

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
void *vmm_kmalloc(size_t *pd,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    va = alloc_bytes(pd, bytes + MEM_MARK_SIZE, MEM_KRNL);
    if (!va)
        return 0;
    va = mark_size(va, bytes);

    return va;
}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *vmm_malloc(size_t *pd,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    printf("vmm_malloc start \n");
    va = alloc_bytes(pd, bytes, MEM_USR);
    if (!va)
        return 0;
    va = mark_size(va, bytes);

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
