#ifndef MM_ZPVRK7R1
#define MM_ZPVRK7R1

#include <klibc.h>
#include "mm.h"


//uint64_t VIRTUAL_MEM_TOTAL = 1024*1024*1024*4;


#define PAGE_MASK 0xfffff000

enum mem_area {
    MEM_USR, MEM_CORE
};

#if 0
struct vmm_t {
    struct pd_t *cur_pd;
    size_t pd_count;   /* currently active PD count */
    /* kernel PTs is linear 1MB chunk just above the kernel */
    addr_t krnl_pt_pa; /* physical addr of kernel PTs*/
    addr_t krnl_pt_va; /* VA of kernel PTs */
    size_t krnl_pt_offset; /* offset in PD which starts kernel's area */
    size_t krnl_pt_idx; /* index to PD which starts kernel's area */
    /* kernel's PTs */
    struct pd_t bios_pd;
    struct pt_t krnl_pts[KRNL_AREA_BLOCK_COUNT / PD_ENTRY_CNT];
    size_t mem_kb;
};
#endif

/* XXX: below macros works on 0xC0000000 - 0xC0400000 VA range only! */
#define krnl_va_to_pa(va)   \
    ((va) - (KRNL_VA_BASE) + (KRNL_PA_BASE))
#define krnl_pa_to_va(pa)   \
    ((pa) + (KRNL_VA_BASE) - (KRNL_PA_BASE))

#define va_to_pt_idx(va)    \
    (((va) >> 22) & 0x3FF) /* last 10 bits */
#define va_to_pte_idx(va)   \
    (((va) >> 12) & 0x3FF) /* first 10 bits */

#define bytes_to_blocks(b)  \
    ((b) / PAGE_SIZE + ((b) % PAGE_SIZE != 0 ? 1 : 0))

#define page_align(addr)    \
    ((addr) + ((PAGE_SIZE) - ((addr) % (PAGE_SIZE))))

/*
 * Empty default initializators
 */
#define empty_vmm(vmm, arr_clean)  \
    do {    \
        (vmm)->cur_pd = NULL;   \
        (vmm)->pd_count = 0;    \
        (vmm)->krnl_pt_pa = 0;  \
        (vmm)->krnl_pt_va = 0;  \
        (vmm)->krnl_pt_offset = 0;  \
        (vmm)->krnl_pt_idx = 0; \
        if ((arr_clean))    \
        {   \
            memset((void *) &(vmm)->bios_pd, 0, \
                    sizeof(typeof((vmm)->bios_pd)));   \
            memset((void *) &(vmm)->krnl_pts[0], 0, \
                    ARRAY_LENGTH((vmm)->krnl_pts)); \
        }   \
    } while (0);

#define empty_pd(pd, arr_clean)    \
    do {    \
        (pd)->pd_va = NULL; \
        (pd)->pd_pa = 0;    \
        (pd)->next = (pd)->prev = pd;   \
        if ((arr_clean))  \
            memset((void *) &(pd)->pt, 0, ARRAY_LENGTH((pd)->pt));  \
    } while (0);

#define empty_pt(pt)   \
    do {    \
        (pt)->table = NULL;   \
        (pt)->pt_pa.addr = 0;      \
        (pt)->full_entries = 0;   \
        (pt)->used_entries = 0;   \
    } while (0);

typedef struct memory_range {
    size_t start_pgd;
    size_t start_pte;
}memory_range;

extern memory_range memory_range_core;

extern memory_range memory_range_user;

/* VMM */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end,size_t reserve);
mm_struct *get_root_pd();
void load_pd(addr_t pde);
void enable_paging();
void refresh_tlb(addr_t *pgd, addr_t va);

void *malloc_frame(mm_struct *mm,size_t bytes);
mm_struct core_mem;

//test
void* fast_malloc(mm_struct *mm,size_t size);
//test

#endif /* end of include guard: MM_ZPVRK7R1 */
