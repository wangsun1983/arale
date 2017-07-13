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
    printf("vmroot start addr is %x \n",vmroot->start_va);
    addr_t va = vm_allocator_alloc(page_size,vmroot);
    printf("wangsl,vmalloc_alloc_bytes start va is %x \n",va);
    addr_t start_pgd = va_to_pt_idx((addr_t)va);
    addr_t start_pte = va_to_pte_idx((addr_t)va);
    addr_t pgd = va_to_pt_idx(va + page_size);
    addr_t pte = va_to_pte_idx(va + page_size);
    printf("wangsl,start_pgd is %x,start_pte is %x,pgd is %x,pte is %x",start_pgd,start_pte,pgd,pte);
    
    switch(type) 
    {
        case MEM_USR:
            start_pgd = start_pgd - memory_range_user.start_pgd;
            start_pte = start_pte - memory_range_user.start_pte;
            pgd -= memory_range_user.start_pgd;
            pte -= memory_range_user.start_pte;

            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = zone_get_page(ZONE_HIGH,PAGE_SIZE);
                printf("user mem is %x,i is %x \n",mem,i);
                mm->pte_user[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
                size -= PAGE_SIZE;
                if(size < 0) {
                    break;
                }
            }
            break;

        case MEM_CORE:
            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = zone_get_page(ZONE_HIGH,PAGE_SIZE);
                printf("core mem is %x,i is %x \n",mem,i);
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
    

    //load_pd((addr_t)core_mem.pgd);

    return va;
}


static void *alloc_bytes(mm_struct *mm, size_t b, enum mem_area area)
{
    int block = bytes_to_blocks(b);

    addr_t pgd = 0;
    addr_t pte = 0;


    char *mem_ptr = NULL;//mm->mem_map;
    addr_t *ptem = NULL;
    //we should do get find range(core or usr)
    addr_t offset = 0;
    int PGD_NUM = 0;

    switch(area) {
        case MEM_USR:
            pgd = 0;
            pte = 0;
            mem_ptr = mm->user_mem_map;
            ptem = mm->pte_user;
            //offset = memory_range_user.start_pgd *1024 + memory_range_user.start_pte;
            PGD_NUM = PD_ENTRY_CNT*3/4;
            break;

        case MEM_CORE:
            pgd = memory_range_core.start_pgd;
            pte = memory_range_core.start_pte;
            mem_ptr = mm->core_mem_map;
            ptem = mm->pte_core;
            PGD_NUM = PD_ENTRY_CNT/4;
            break;
    }

    //printf("alloc_bytes,pgd is %x,pte is %x,PGD_NUM is %x area is %d b is %x\n",pgd,pte,PGD_NUM,area,b);
    //find block

    int find_block = 0;
    int hit = PDT_LOSS;

    addr_t start_pgd = -1;
    addr_t start_pte = -1;

    int i = 0;

    //printf("set 2 mem_ptr is %d \n",mem_ptr[2562]);

    for(;pgd < PGD_NUM;pgd++)
    {
        //because 0<<22|0<<12 is still 0
        //so we start from pte 1
        if(pgd == 0) {
            pte = 1;
        } else if(pte == PD_ENTRY_CNT){
            pte = 0;
        }

        for(;pte < PD_ENTRY_CNT;pte++) {
            int bit = get_bit(mem_ptr,pgd*PD_ENTRY_CNT + pte);
            if(bit == 0) {
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

    //addr_t *ptem = (addr_t *)mm->pte_kern;
    addr_t mem = 0;
    //printf("start i is %x,end i is %x \n",PD_ENTRY_CNT*start_pgd + start_pte,PD_ENTRY_CNT*pgd + pte);


    for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
        mem = zone_get_page(ZONE_HIGH,PAGE_SIZE);
        ptem[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        //TODO do we need to refresh tlb??????
        set_bit(mem_ptr,i,1);

        b -= PAGE_SIZE;
        if(b < 0) {
            break;
        }
    }
    
    //printf("alloc start_pgd is %d,start_pte is %d \n",start_pgd,start_pte);

    if(area == MEM_USR) {
        start_pgd += memory_range_user.start_pgd;
    }
   
    //printf("alloc start_pgd is %d,start_pte is %d \n",start_pgd,start_pte);
   
    return (start_pgd<<22) | (start_pte<<12);
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
    //printf("scan_start_pgd is %d scan_start_pte is %d \n",scan_start_pgd,scan_start_pte);

    //map 4G memory, physcial address = virtual address
    for (i = 0; i < memory_range_user.start_pgd; i++) {
        core_mem.pgd[i] = (addr_t)&core_mem.pte_core[i*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    }
    //printf("core_mem.pgd[0] %x \n",core_mem.pgd[0]);
    //printf("core_mem.pte_core %x \n",(addr_t)&core_mem.pte_core[0]);

#ifdef CORE_PROCESS_USER_SPACE
    for(i = memory_range_user.start_pgd; i < PD_ENTRY_CNT; i++) {
        core_mem.pgd[i] = (addr_t)&core_mem.pte_user[user_index*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        user_index++;
    }
#endif
    //printf("i2 is %x,user_index is %x \n",i,user_index);


    //because there is no user memory in core process,
    //needn't init user core memory in core process~
    addr_t *pte = (addr_t *)core_mem.pte_core;
    core_mem.core_mem_map = &core_mem_reserve_map;
    addr_t core_init_end = memory_range_user.start_pgd*PD_ENTRY_CNT + memory_range_user.start_pte;

    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT/4; i++) {
        pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
        set_bit(core_mem.core_mem_map,i,0);
    }
    //printf("i3 is %x",i);
#ifdef CORE_PROCESS_USER_SPACE
    //core_mem.user_mem_map = &user_mem_reserve_map;
    //pte = (addr_t *)core_mem.pte_user;
    //for(i = 0;i<PD_ENTRY_CNT*PT_ENTRY_CNT*3/4;i++) {
    //    pte[i] = ((i + PD_ENTRY_CNT*PT_ENTRY_CNT/4)<< 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
    //   set_bit(core_mem.user_mem_map,i,0);
    //}
#endif
    //printf("i4 is %x",i);

    //before cr3
    //printf("before pte-core is %x \n",core_mem.pte_core);

    load_pd((addr_t)core_mem.pgd);
    enable_paging();
    //printf("after pte-core is %x \n",core_mem.pte_core);

    //reconfig struct mm
    mm_operation.vmalloc = vmm_vmalloc;
    mm_operation.kmalloc = vmm_kmalloc;
    mm_operation.malloc = vmm_malloc;
    mm_operation.fmalloc = malloc_frame;
    mm_operation.free = dealloc;

    //printf("user_start_memory is %x,memory_range_user.start_pgd is %x,start_pte is %x \n",
    //&memory_range_user,memory_range_user.start_pgd,
    //memory_range_user.start_pte);
    //init for high memory TODO
    core_mem.vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);
    core_mem.userroot = vm_allocator_init(1024*1024*1024,1024*1024*1024*3); //user space is 1~3G
    
    printf("vmm_init vmroot is %x \n",core_mem.vmroot);
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
inline static void *mark_frame_size(void *mem, size_t bytes) {

    char * p = mem + PAGE_SIZE - MEM_MARK_SIZE;
    *p = bytes;

    return mem + PAGE_SIZE;
}

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
    printf("dealloc 1_1 ptr is %x,zone_list[ZONE_NORMAL].end_pa is %x \n",ptr,zone_list[ZONE_NORMAL].end_pa);
    if (!b)
        return;

    ptr = unmark_size(ptr);
    //we should distribute the vm address
    if(ptr < zone_list[ZONE_NORMAL].end_pa) 
    {
        printf("dealloc 1 ptr is %x \n",ptr);
        zone_list[ZONE_NORMAL].alloctor_free(ptr);
    } 
    else
    {
        printf("dealloc 2 \n");
        vm_allocator_free(ptr,mm->vmroot);
        addr_t lptr = ptr;

        for(;lptr < ptr + b;lptr+=PAGE_SIZE)
        {
            //get physical address
            int pt = va_to_pt_idx(lptr);
            int pte = va_to_pte_idx(lptr);
            addr_t pa = mm->pte_core[pt*PD_ENTRY_CNT + pte];
            //printf("dealloc 3,pt is %x,pte is %x pa is %x ,index is %x \n",pt,pte,pa,pt*PD_ENTRY_CNT + pte);
            zone_list[ZONE_HIGH].alloctor_free(pa);
        }   
    }
    
    //dealloc_bytes(mm,ptr, b);
}

/*
void dealloc_bytes(mm_struct *mm,void *ptr,size_t size) {
    //we should make virtual memroy to dirty
    int start_pgd = va_to_pt_idx((size_t)ptr);
    int start_pte = va_to_pte_idx((size_t)ptr);

    int end_pgd = va_to_pt_idx((size_t)ptr + size);
    int end_pte = va_to_pte_idx((size_t)ptr + size);

#if 0 //TODO
    char *mem_ptr = mm->mem_map;
    addr_t *ptem = (addr_t *)mm->pte_kern;
    int i = PD_ENTRY_CNT*start_pgd + start_pte;
    //printf("ptr is %x,dealloc_bytes pgd is %d,start_pte is %d \n",ptr,start_pgd,start_pte);
    for (;i <= PD_ENTRY_CNT*end_pgd + end_pte; i++) {
        addr_t pmm = (ptem[i]&0xFFFF000);
        ptem[i] == pmm|ENTRY_PAGE_DIRTY;

        set_bit(mem_ptr,i,0);
        pmm_dealloc_page(pmm);
    }
#endif //TODO
}*/


//this is used from userspace
void *vmm_malloc(mm_struct *mm,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
    va = vmalloc_alloc_bytes(mm, MEM_USR,bytes);
    if (!va)
        return 0;

    //do clean for safe,haha
    //memset(va,0,bytes);
    //do clean for safe,haha

    va = mark_size(va, bytes);
    return va;
}


/*
 * Allocates `bytes` sized memory chunk in kernel space.
 */
void *vmm_kmalloc(mm_struct *mm,size_t bytes)
{
    void *va;
    //printf("vmm_kmalloc start \n");
    bytes += MEM_MARK_SIZE;
    //because core's physical memory is one-one correspondence
    //so we use coalition_alloctor to alloc memory directly.
    //va = alloc_bytes(mm, bytes, MEM_CORE);
    va = zone_get_page(ZONE_NORMAL,bytes);
    va = mark_size(va, bytes);
    //printf("vmm_kmalloc end,va is %x \n",va);
    if (!va)
        return 0;
    //do clean for safe,haha
    //memset(va,0,bytes);
    //do clean for safe,haha
    //va = mark_size(va, bytes);
    return va;
}

/*
 * Allocates `bytes` sized memory chunk in user space.
 */
void *vmm_vmalloc(mm_struct *mm,size_t bytes)
{
    void *va;
    printf("vmm_malloc start \n");

    bytes += MEM_MARK_SIZE;
    //va = alloc_bytes(mm, bytes, MEM_CORE);
    va = vmalloc_alloc_bytes(mm,MEM_CORE,bytes);
    if (!va)
        return 0;

    //do clean for safe,haha
    //memset(va,0,bytes);
    //do clean for safe,haha

    va = mark_size(va, bytes);

    return va;

}

void *malloc_frame(mm_struct *mm,size_t bytes) {

    void *va;

    bytes += PAGE_SIZE;
    va = alloc_bytes(mm, bytes, MEM_CORE);
    if (!va)
        return 0;

    //do clean for safe,haha
    //memset(va,0,bytes);
    //do clean for safe,haha
    //printf("before malloc va is %x \n",va);
    va = mark_frame_size(va, bytes);
    //printf("after malloc va is %x \n",va);
    return va;
}

//test
static void *alloc_bytes_fast(mm_struct *mm, size_t b, enum mem_area area)
{
    int block = bytes_to_blocks(b);

    addr_t pgd = 0;
    addr_t pte = 0;


    char *mem_ptr = NULL;//mm->mem_map;
    addr_t *ptem = NULL;
    //we should do get find range(core or usr)
    addr_t offset = 0;
    int PGD_NUM = 0;

    switch(area) {
        case MEM_USR:
            pgd = 0;
            pte = 0;
            mem_ptr = mm->user_mem_map;
            ptem = mm->pte_user;
            //offset = memory_range_user.start_pgd *1024 + memory_range_user.start_pte;
            PGD_NUM = PD_ENTRY_CNT*3/4;
            break;

        case MEM_CORE:
            pgd = memory_range_core.start_pgd;
            pte = memory_range_core.start_pte;
            mem_ptr = mm->core_mem_map;
            ptem = mm->pte_core;
            PGD_NUM = PD_ENTRY_CNT/4;
            break;
    }

    //printf("alloc_bytes,pgd is %x,pte is %x,PGD_NUM is %x area is %d b is %x\n",pgd,pte,PGD_NUM,area,b);
    //find block

    int find_block = 0;
    int hit = PDT_LOSS;

    addr_t start_pgd = -1;
    addr_t start_pte = -1;

    int i = 0;

    //printf("set 2 mem_ptr is %d \n",mem_ptr[2562]);

    for(;pgd < PGD_NUM;pgd++)
    {
        //because 0<<22|0<<12 is still 0
        //so we start from pte 1
        if(pgd == 0) {
            pte = 1;
        } else if(pte == PD_ENTRY_CNT){
            pte = 0;
        }

        for(;pte < PD_ENTRY_CNT;pte++) {
            int bit = get_bit(mem_ptr,pgd*PD_ENTRY_CNT + pte);
            if(bit == 0) {
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

    addr_t mem = 0;

    if(area == MEM_USR) {
        start_pgd += memory_range_user.start_pgd;
    }
   
    //printf("alloc start_pgd is %d,start_pte is %d \n",start_pgd,start_pte);

    return (start_pgd<<22) | (start_pte<<12);
}

void *fast_malloc(mm_struct *mm,size_t bytes) 
{

    void *va;

    
    bytes += PAGE_SIZE;

    va = alloc_bytes_fast(mm, bytes, MEM_USR);
    if (!va)
        return 0;

    //do clean for safe,haha
    //memset(va,0,bytes);
    //do clean for safe,haha
    //printf("before malloc va is %x \n",va);
    //va = mark_frame_size(va, bytes);
    //printf("after malloc va is %x \n",va);
    return va;
}

//test


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
