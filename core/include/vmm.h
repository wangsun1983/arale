#ifndef MM_ZPVRK7R1
#define MM_ZPVRK7R1

#include <klibc.h>
#include "mm.h"

#define PAGE_MASK 0xfffff000

enum mem_area {
    MEM_USR, MEM_KRNL
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


static range_t lookup_range_usr = {
    .from = 0x4000000,
    .to = 0xC0000000
};
static range_t lookup_range_krnl = {
    .from = 0xC0000000,
    .to = UINT_MAX
};

/* PMM */
addr_t pmm_init(unsigned int mem_kb, addr_t bitmap_loc);
int pmm_init_region(unsigned int addr, size_t size);
extern void *pmm_alloc(unsigned int bytes);
size_t get_total_mem_b();
size_t get_free_mem_b();
size_t get_used_mem_b();
size_t get_krnl_size();

/* VMM */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end);
void free(void *ptr);
void *kalloc(size_t bytes);
void *malloc(size_t bytes);
mm_struct *get_root_pd();

#endif /* end of include guard: MM_ZPVRK7R1 */
