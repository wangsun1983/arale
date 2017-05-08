#ifndef _MM_H_
#define _MM_H_

#include "ctype.h"

#define CR0_ENABLE_PAGING 0x80000000

#define BIOS_PD_PA 0x9C000
#define BIOS_PD_VA (BIOS_PD_PA)
#define BIOS_PT_PA 0x9D000
#define BIOS_PT_VA (BIOS_PT_PA)
#define BIOS_KRNL_PT_PA 0x9E000
#define BIOS_KRNL_PT_VA (BIOS_KRNL_PT_PA)

#define KRNL_PA_BASE 0x100000
#define KRNL_VA_BASE 0xC0000000
#define KRNL_AREA_BYTE_COUNT    ((UINT_MAX) - ((KRNL_VA_BASE) - 1))
#define KRNL_AREA_BLOCK_COUNT   ((KRNL_AREA_BYTE_COUNT) / (PAGE_SIZE))

#define PAGE_SIZE 4096
#define BYTES_PER_PTE (sizeof(union entry_t))
#define PT_ENTRY_CNT 1024
#define PD_ENTRY_CNT 1024
#define PT_SIZE ((PT_ENTRY_CNT) * (BYTES_PER_PTE))
#define PD_SIZE ((TABLE_SIZE) * (DIR_ENTRIES))

#define MEM_MARK_SIZE (sizeof(unsigned int))

struct boot_info {
    unsigned int mem_size;
    unsigned int krnl_size;
    unsigned int krnl_loc;
//wangsl
    //unsigned int gdt_addr;
//wangsl
} __attribute__((__packed__));

enum pte_flag {
    ENTRY_PRESENT     = 0x1,      /* 000000000001 */
    ENTRY_RW          = 0x2,      /* 000000000010 */
    ENTRY_SUPERVISOR  = 0x4,      /* 000000000100 */
                         /* RESERVED BY INTEL: 000000011000 */
    /* Following flags are set by CPU */
    ENTRY_PAGE_ACCESSED = 0x10,   /* 000000100000 */
    ENTRY_PAGE_DIRTY  = 0x20, /* 000001000000 */
                         /* RESERVED BY INTEL: 000110000000 */
    /* Flags free to use by OS */
    ENTRY_FREE_FLAG_0  = 0x100,    /* 001000000000 */
    ENTRY_FREE_FLAG_1  = 0x200,    /* 010000000000 */
    ENTRY_FREE_FLAG_2  = 0x400,    /* 100000000000 */
                                        /* Remaining 12-31 bits: */
    ENTRY_FRAME_ADDR  = 0xfffff000    /* 11111111111111111111000000000000 */
};

union entry_t {
    addr_t addr;
    unsigned char flag;
};

struct pt_t {
    //addr_t *entry; /* pointer to 1024 linear PTEs */
    union entry_t *table;
    union entry_t pt_pa;
    size_t used_entries; /* number of allocated PTEs */
    size_t full_entries; /* number of full entries */
    /* FULL_PTE_LIMIT is used to tell if the entry is full */
};
#define FULL_PTE_LIMIT 3072  /* 3/4 of a PAGE_SIZE */

struct pd_t {
    addr_t *pd_va; /* pointer to PD with 1024 linear PAs */
    addr_t pd_pa; /* goes into CR3 */
    struct pt_t pt[PT_ENTRY_CNT]; /* page tables */
    struct pd_t *next, *prev; /* pointers to next/prev PDs */
};

typedef struct memory{
    addr_t pgd_kern[PD_ENTRY_CNT] __attribute__((aligned(PAGE_SIZE)));
    addr_t pte_kern[PD_ENTRY_CNT][PT_ENTRY_CNT] __attribute__((aligned(PAGE_SIZE)));
    char mem_map[PD_ENTRY_CNT][PT_ENTRY_CNT];  
}mm_struct;


struct mm_operation
{
    size_t (*get_total_mem)(void *);
    size_t (*get_free_mem)(void *);
    size_t (*get_used_mem)(void *);
    void *(*malloc)(mm_struct *mm,size_t bytes);
    void *(*kmalloc)(mm_struct *mm,size_t bytes);
    void (*free)(void *);
};

struct mm_operation mm_operation;

//wangsl
void mm_init(struct boot_info *binfo);
mm_struct* create_mm();

void *malloc(size_t bytes);
void *kmalloc(size_t bytes);
//wangsl

//we should use a vmm struct for per process
//struct mm_area_struct {
//    addr_t *pd; //this is the full pd/pt
//};

#endif
