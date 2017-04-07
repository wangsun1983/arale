/******************************************************************************
 *      Virtual Memory Manager
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"

void *vmm_malloc(struct pd_t *pd,size_t bytes);
void *vmm_kmalloc(struct pd_t *pd,size_t bytes);

static struct vmm_t vmm = {
    .cur_pd = NULL,
    .pd_count = 0,
    .krnl_pt_pa = 0,
    .krnl_pt_va = 0,
    .krnl_pt_offset = 0,
    .krnl_pt_idx = 0,
    .mem_kb = 0
};

static void load_pd(struct pd_t *pd)
{
    __asm__ __volatile__("movl %0, %%eax \n"
                         "movl %%eax, %%cr3 \n"
                    :
                    : "m" (pd->pd_pa)
                    : "eax");
}

/*
 * Entry flag check routines.
 */

static inline int is_present(union entry_t *entry)
{
    return entry->flag & ENTRY_PRESENT;
}

static inline int is_rw(union entry_t *entry)
{
    return entry->flag & ENTRY_RW;
}

static inline int is_supervisor(union entry_t *entry)
{
    return entry->flag & ENTRY_SUPERVISOR;
}

static inline int is_page_accessed(union entry_t *entry)
{
    return entry->flag & ENTRY_PAGE_ACCESSED;
}

static inline int is_page_dirty(union entry_t *entry)
{
    return entry->flag & ENTRY_PAGE_DIRTY;
}

/*
 * Check a sequence of `cnt` entries for a present flag.
 * Returns 1 if all passed,
 * or negative position of the first reached false condition.
 */
static int sequence_is_present(union entry_t *entry, size_t cnt)
{
    size_t i;

    for (i = 0; i < cnt; i++, entry += BYTES_PER_PTE)
    {
        if (is_present(entry))
            return -i;
    }

    return 1;
}

/*
 * Entry flag manipulation routines.
 */

static inline union entry_t *entry_add_frame(union entry_t *entry, addr_t frame)
{
    entry->addr = frame & ENTRY_FRAME_ADDR;
    return entry;
}

static inline union entry_t *entry_add_flag(union entry_t *entry, unsigned char flag)
{
    entry->flag |= flag;
    return entry;
}

static inline union entry_t *entry_rm_flag(union entry_t *entry, unsigned char flag)
{
    entry->flag &= (~flag);
    return entry;
}

/*
 * Sets a sequence of entries as present
 * Returns 0 on success.
 */
static int sequence_make_present(union entry_t *entry, size_t cnt)
{
    for (; cnt > 0; cnt--)
        entry_add_flag(entry, ENTRY_PRESENT);

    return cnt;
}

/*
 * Unsets a sequence of entries as not present.
 * Returns 0 on success.
 */
static int sequence_unmake_present(union entry_t *entry, size_t cnt)
{
    for (; cnt > 0; cnt--)
        entry_rm_flag(entry, ENTRY_PRESENT);

    return cnt;
}

/*
 * Returns a pointer to the memory address where specified `va`
 * page info resides.
 */
static union entry_t *va_to_pt_entry(struct pd_t *pd, addr_t va)
{
    size_t pt_idx, pte_idx;

    pt_idx = va_to_pt_idx(va);
    pte_idx = va_to_pte_idx(va);

    /* return (addr_t *) &pd->pt[pt_idx].entry[pte_idx]; */
    return &pd->pt[pt_idx].table[pte_idx];
}

/*
 * Returns a pointer to a page table where given `va` is in.
 */
static struct pt_t *va_to_pd_pt(struct pd_t *pd, addr_t va)
{
    size_t pt_idx;

    pt_idx = va_to_pt_idx(va);

    return &pd->pt[pt_idx];
}

/*
 * Initializes VMM.
 */
int vmm_init(size_t mem_kb, addr_t krnl_bin_end)
{
    size_t i, cnt, step;
    addr_t addr = page_align(krnl_bin_end);
    addr_t va;
    struct pt_t *krnl_bin_pt;
    union entry_t *entry;

    vmm.mem_kb = mem_kb;

    /* Initialize a new VMM */
    empty_vmm(&vmm, true);
    vmm.krnl_pt_va = addr;
    vmm.krnl_pt_pa = krnl_va_to_pa(addr);
    vmm.krnl_pt_idx = PD_ENTRY_CNT - (KRNL_AREA_BLOCK_COUNT / PD_ENTRY_CNT);
    vmm.krnl_pt_offset = vmm.krnl_pt_idx * BYTES_PER_PTE;

    /* Use BIOS created PD for the moment */
    vmm.bios_pd.pd_pa = BIOS_PD_PA;
    vmm.bios_pd.pd_va = (addr_t *) BIOS_PD_VA;
    vmm.cur_pd = &vmm.bios_pd;
    vmm.pd_count = 0;

    /* clear kernel PTs */                  /* 1 block = 1 PTE */
    memset((void *) vmm.krnl_pt_va, 0, KRNL_AREA_BLOCK_COUNT * BYTES_PER_PTE);

    /* prepare kernel PTs */
    for (i = vmm.krnl_pt_idx,
            va = vmm.krnl_pt_va, step = (PT_ENTRY_CNT * BYTES_PER_PTE);
         i < PD_ENTRY_CNT; i++, va += step)
    {
        empty_pt(&vmm.cur_pd->pt[i]);
        vmm.cur_pd->pt[i].table = (union entry_t *) va;
        vmm.cur_pd->pt[i].pt_pa.addr = krnl_va_to_pa(va);
        entry_add_flag(&vmm.cur_pd->pt[i].pt_pa, ENTRY_PRESENT);
        entry_add_flag(&vmm.cur_pd->pt[i].pt_pa, ENTRY_RW);
        vmm.cur_pd->pt[i].full_entries = 0;
        vmm.cur_pd->pt[i].used_entries = 0;
    }

    /* Copy bootloader's prepared 768th table content to new loc */
    entry = va_to_pt_entry(vmm.cur_pd, KRNL_VA_BASE);
    cnt = PT_ENTRY_CNT * BYTES_PER_PTE;
    memcpy((void *) entry, (void *) ((addr_t *) BIOS_KRNL_PT_VA), cnt);
    /* set that table as present and full */
    krnl_bin_pt = va_to_pd_pt(vmm.cur_pd, KRNL_VA_BASE);
    krnl_bin_pt->full_entries = PT_ENTRY_CNT;
    krnl_bin_pt->used_entries = PT_ENTRY_CNT;

    /* after PTs are ready, map them to PD */
    for (i = vmm.krnl_pt_idx; i < PD_ENTRY_CNT; i++)
        vmm.cur_pd->pd_va[i] = vmm.cur_pd->pt[i].pt_pa.addr;

    /* load newly constructed PD */
    load_pd(vmm.cur_pd);

//wangsl
    mm_operation.malloc = vmm_malloc;
    mm_operation.kmalloc = vmm_kmalloc;
    mm_operation.free = free;
//wangsl

    return 0;
}

/*
 * Returns a VA of `b` size free space inside reusable PTE.
 * TODO: not finished!
 */
static addr_t find_block_reuse(struct pd_t *pd, size_t b, range_t *lookup_range)
{
    addr_t va;

    if (b < FULL_PTE_LIMIT)
        return 0;

    for (va = lookup_range->from; va < lookup_range->to; va += PAGE_SIZE)
    {
        struct pt_t *pt = va_to_pd_pt(pd, va);

        if (!is_present(&pt->pt_pa))
            continue;
        if (pt->full_entries == PT_ENTRY_CNT || pt->used_entries == 0)
            continue;

        /* TODO: got the table with reusable entries. Do something... */
    }

    return 0;
}

/*
 * Finds a sequence of available VA addresses to fit `cnt` pages.
 */
static addr_t find_blocks(struct pd_t *pd, size_t cnt, range_t *lookup_range)
{
    addr_t va;
    size_t i, j;
    size_t found;
    addr_t found_start;

    if (cnt == 0 || cnt > (lookup_range->to - lookup_range->from) / PAGE_SIZE)
        return 0;

    for (va = lookup_range->from, found = 0, found_start = 0;
          (va + (cnt * PAGE_SIZE)) < lookup_range->to;
           va += PT_ENTRY_CNT * PAGE_SIZE)
    {
        struct pt_t *pt = va_to_pd_pt(pd, va);

        /* check first if the table itself is present */
        if (!is_present(&pt->pt_pa))
            continue;
        /* and that it is not full */
        if (pt->full_entries == PT_ENTRY_CNT || pt->used_entries == PT_ENTRY_CNT)
            continue;

        for (i = 0; i < PT_ENTRY_CNT; i++)
        {
            if (is_present(&pt->table[i]))
                continue;
            
            for (j = i; j < PT_ENTRY_CNT; j++)
            {
                int look_cnt = MIN(PT_ENTRY_CNT - j, cnt);
                int seq = sequence_is_present(&pt->table[j], look_cnt);
                if (seq == 1)
                {
                    if (found == 0)
                        found_start = va + (j * PAGE_SIZE);
                    found += look_cnt;
                }
                else
                {
                    found = 0;
                    found_start = 0;
                    j += ABS(seq);
                }

                if (found >= cnt)
                    return found_start;
            }
        }
    }

    return 0;
}

/*
 * Does the actual allocation of memory by mapping VA with PA
 * and setting correct flags.
 * Returns 0 on success.
 */
static int do_alloc_pages(addr_t va, size_t pg_count)
{
    size_t i;
    void *mem;
    union entry_t *entry;
    struct pt_t *pt;

    for (i = 0; i < pg_count; i++)
    {
        /* get a physical memory */
        mem = pmm_alloc(PAGE_SIZE);
        if (!mem)
            return -ENOMEM;

        /* map physical memory to va */
        entry = va_to_pt_entry(vmm.cur_pd, va + (i * PAGE_SIZE));
        entry_add_frame(entry, (addr_t) mem);
        entry_add_flag(entry, ENTRY_PRESENT);
        entry_add_flag(entry, ENTRY_RW);

        /* update the PT */
        pt = va_to_pd_pt(vmm.cur_pd, va);
        pt->used_entries++;
        if (pt->used_entries == FULL_PTE_LIMIT)
            pt->full_entries++;
    }

    return 0;
}

/*
 * Returns a VA to a free memory chunk of `b` size.
 * Returns 0 on error.
 */
static void *alloc_bytes(struct pd_t *pd, size_t b, enum mem_area area)
{
    size_t block_cnt = bytes_to_blocks(b);   
    size_t last_blc_sz = b % PAGE_SIZE;
    range_t *lookup_range =
        (area == MEM_USR ? &lookup_range_usr : &lookup_range_krnl);
    addr_t va;

    if (!block_cnt && !last_blc_sz)
        return 0;

/*
 *  Re-usable blocks are not implemented yet
 */
    /* if (block_cnt == 1 && last_blc_sz < FULL_PTE_LIMIT) */
    /* { */
    /*     va = find_block_reuse(pd, b, lookup_range); */
    /*     if (!va) */
    /*         va = find_blocks(pd, 1, lookup_range); */
    /*     if (!va) */
    /*         return 0; */
    /* } */
    /* else */
    {
        va = find_blocks(pd, block_cnt, lookup_range);
        if (!va)
            return 0;
    }

    if (do_alloc_pages(va, block_cnt))
        return 0;

    return (void *) va;
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
 * Does the actual memory deallocation, so that [km]alloc could use it.
 */
static int dealloc_bytes(void *ptr, size_t b)
{
    size_t i;
    size_t block_cnt = bytes_to_blocks(b);
    addr_t addr = (addr_t) ptr;
    struct pt_t *pt;
    union entry_t *entry;
    
    if (!block_cnt || !ptr)
        return -EBADADDR;

    for (i = 0; i < block_cnt; i++, addr += PAGE_SIZE)
    {
        entry = va_to_pt_entry(vmm.cur_pd, addr);   
        entry_rm_flag(entry, ENTRY_PRESENT);
        
        pt = va_to_pd_pt(vmm.cur_pd, addr);
        pt->used_entries--;
        if (pt->full_entries == FULL_PTE_LIMIT - 1)
            pt->full_entries--;
        /* if (!pt->used_entries) */
        /*     entry_rm_flag(&pt->pt_pa, ENTRY_PRESENT); */
    }

    return 0;
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
    dealloc_bytes(ptr, b);
}

/*
 * Allocates `bytes` sized memory chunk in kernel space.
 */
void *vmm_kmalloc(struct pd_t *pd,size_t bytes)
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
void *vmm_malloc(struct pd_t *pd,size_t bytes)
{
    void *va;

    bytes += MEM_MARK_SIZE;
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
