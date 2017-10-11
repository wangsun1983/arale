#ifndef _MM_H_
#define _MM_H_

#include "ctype.h"
#include "rbtree.h"
#include "list.h"
#include "const.h"

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

#define MEMORY_CORE_RATIO 1
#define MEMORY_USER_RATIO 3

#define MEM_MARK_SIZE (sizeof(addr_t))

#define CORE_PROCESS_USER_SPACE

#define RECLAIM_MM_NORMAL_THRESHOLD 1024*1024*32l
#define RECLAIM_MM_CRITICAL_THRESHOLD 1024*1024*8l


#define PAGE_SIZE_RUND_UP(x) (x - (x>>11<<11)) ==0?x:(x>>11<<11)+PAGE_SIZE
#define PAGE_SIZE_RUND_DOWN(x) x>>11<<11

#define GET_ORDER(x)                            \
({	\
    int order = 0;                             \
    do { \
        order++;\
    }while(x>>order != 0);\
\
    order-1;\
})


#define GET_ALIGN_PAGE(pagesize,p)                            \
({	\
    uint32_t al_size = 0;\
    int al_shift = GET_LEFT_SHIFT(pagesize); \
    int al_order = 0;\
    if(al_shift < 12) {\
        al_size = PAGE_SIZE;\
        al_order = 0; \
    } else if(pagesize == (1<<al_shift)) {\
        al_size = pagesize;\
        al_order = al_shift - 12;\
    } else { \
        al_order = (al_shift + 1) - 12;\
        al_size = 1 << (al_shift + 1);\
    }\
    if(p != NULL) {\
        ((align_result*)p)->order = al_order;\
        ((align_result*)p)->page_size = al_size;\
    }\
    al_size;\
})

#define GET_LEFT_SHIFT(x)                            \
({	\
    int order = 0;                             \
    while(x >> order != 1) { \
        order++;\
    } \
    order;\
})

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
    //addr_t *entry;  //pointer to 1024 linear PTEs
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

typedef struct vm_root {
    struct rb_root free_root;
    struct rb_root used_root;

    struct list_head free_nodes;
    //we use free_fragements to count free pages
    //so we can do page merge when there are to
    //many small page fragments.
    uint32_t free_fragments;
    addr_t start_va;
    uint32_t size;
}vm_root;

typedef struct vm_node {
    uint32_t page_num;
    addr_t start_va;
    addr_t end_va;
    struct rb_node rb;
    struct list_head ll;
}vm_node;


struct memory{
    addr_t *pgd; //this is used for cr0

    addr_t *pte_core; //this is kern pgt
    addr_t *pte_user; //this is user pgt

    //char *core_mem_map; //bitmap for core memroy
    //char *user_mem_map; //bitmap for user memory
    vm_root *vmroot;
    vm_root *userroot;
}__attribute__((aligned(PAGE_SIZE)));

typedef struct memory mm_struct;

//this is the core processs(process 0) memory
addr_t process_core_pgd[PD_ENTRY_CNT] __attribute__((aligned(PAGE_SIZE)));
addr_t process_core_pte[PD_ENTRY_CNT/4][PT_ENTRY_CNT] __attribute__((aligned(PAGE_SIZE)));
#ifdef CORE_PROCESS_USER_SPACE
addr_t process_user_pte[PD_ENTRY_CNT*3/4][PT_ENTRY_CNT] __attribute__((aligned(PAGE_SIZE)));
//char user_mem_reserve_map[PD_ENTRY_CNT*PT_ENTRY_CNT*3/32];
#endif
//char core_mem_reserve_map[PD_ENTRY_CNT*PT_ENTRY_CNT/32];


struct mm_operation
{
    size_t (*get_total_mem)(void *);
    size_t (*get_free_mem)(void *);
    size_t (*get_used_mem)(void *);
    void *(*malloc)(mm_struct *mm,size_t bytes);
    void *(*vmalloc)(mm_struct *mm,size_t bytes);
    void *(*kmalloc)(size_t bytes);
    void *(*pmalloc)(size_t bytes);//we use this to alloc p-memory(pure continous physical memory)
    void (*free)(mm_struct *mm,addr_t addr);
    void (*pfree)(addr_t addr); //pmemory is a special free......
};

public struct mm_operation mm_operation;
public void mm_init(struct boot_info *binfo);
public void *vmalloc(size_t bytes);
public void *kmalloc(size_t bytes);
public void *pmalloc(size_t bytes);
public void free(void *p);
public void pfree(void *p);

#endif
