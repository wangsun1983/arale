/******************************************************************************
 *      Virtual Memory Manager
 *
 ******************************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"

void *vmm_malloc(mm_struct *pd,size_t bytes);
void *vmm_kmalloc(mm_struct *pd,size_t bytes);

#define MEM_TO_PGD(b) \
    ((b) / (1024*1024))

#define MEM_TO_PTE(b)  \
    ((b) % (1024*1024) )
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
int scan_start_pgd = 0;
int scan_start_pte = 0;

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
    //printf("block is %d \n",block);

    //find block
    addr_t pgd = scan_start_pgd;
    addr_t pte = scan_start_pte;
    int find_block = 0;
    int hit = PDT_LOSS;

    addr_t start_pgd = -1;
    addr_t start_pte = -1;

    int i = 0;

    char *mem_ptr = mm->mem_map;
    //printf("set 2 mem_ptr is %d \n",mem_ptr[2562]);

    for(;pgd < PD_ENTRY_CNT;pgd++)
    {
        //because 0<<22|0<<12 is still 0
        //so we start from pte 1
        if(pgd == 0) {
            pte = 1;
        } else {
            pte = 0;
        }

        for(;pte < PD_ENTRY_CNT;pte++) {

            if((mem_ptr[pgd*PD_ENTRY_CNT + pte]&ENTRY_PRESENT) != ENTRY_PRESENT) {
               find_block++;
               if(start_pgd == -1) {
                   start_pgd = pgd;
                   start_pte = pte;
               }
            } else {
               //printf("wangsl,not fit pgd is %d,pte is %d,mm->mem_map[pgd][pte] is %d \n",pgd,pte,mm->mem_map[pgd][pte]);
               find_block = 0;
               start_pgd = -1;
               start_pte = -1;
            }

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

    //printf("mm is %x,start pte is %d \n",mm,start_pte);
    //printf("mm is %x,start pgd %d \n",mm,start_pgd);
    //printf("mm is %x,find pte is %d \n",mm,pte);
    //printf("mm is %x,find pgd is %d \n",mm,pgd);

    addr_t *ptem = (addr_t *)mm->pte_kern;
    addr_t mem = 0;

    for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {  
        //int page = 0;   
        if(b/PAGE_SIZE == 0) {
            mem = pmm_alloc(b);
            //page = b;
        } else {
            mem = pmm_alloc(PAGE_SIZE);
            //page = PAGE_SIZE;
        }
        //goto_xy(10,10);
        //printf("index i is %d ,mem alloc is %x,page is %d",i,mem,page);
        ptem[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        mem_ptr[i] |= ENTRY_PRESENT;
        b -= PAGE_SIZE;
        if(b < 0) {
            break;
        }
    }

    //printf("alloc finish,b is %d,i is %d,start_pgd is %d,start_pte is %d mem is %x \n",b,i,start_pgd,start_pte,mem);
    //printf("mm->pte_kern is %x,ptem[253] is %x\n",mm->pte_kern[0][253],ptem[253]);
    //load_pd((addr_t)mm->pgd_kern);
    //printf("ret is %x \n",(start_pgd<<22) | (start_pte<<12));
    return (start_pgd<<22) | (start_pte<<12);
}

int vmm_init(size_t mem_kb, addr_t krnl_bin_end,size_t reserve)
{
    addr_t i;

    scan_start_pgd = MEM_TO_PGD(reserve);
    scan_start_pte = MEM_TO_PTE(reserve);

    printf("scan_start_pgd is %d scan_start_pte is %d \n",scan_start_pgd,scan_start_pte);
    // map 4G memory, physcial address = virtual address
    for (i = 0; i < PD_ENTRY_CNT; i++) {
        core_mem.pgd_kern[i] = (addr_t)core_mem.pte_kern[i] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    }
    
    addr_t *pte = (addr_t *)core_mem.pte_kern;
    char *mem_ptr = core_mem.mem_map;

    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT ; i++) {
        pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
        mem_ptr[i]= 0;
    }

    load_pd((addr_t)core_mem.pgd_kern);
    enable_paging();

    //reconfig struct mm
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
    va = alloc_bytes(mm, bytes, MEM_KRNL);
    if (!va)
        return 0;
    //va = mark_size(va, bytes);
    return va;
}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *vmm_malloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    va = alloc_bytes(mm, bytes, MEM_USR);
    if (!va)
        return 0;
    //va = mark_size(va, bytes);
    return va;
}


void enable_paging()
{
    uint32_t cr0;
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

//wangsl
