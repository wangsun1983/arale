/******************************************************************************
 *      Physical Memory Manager
 *
 *          Author: Arvydas Sidorenko
 ******************************************************************************/

#include "klibc.h"
#include "kerror.h"
#include "mm.h"
#include "bitmap.h"
#include "pmm.h"

#define BLOCK_SIZE 4096 /* same size as VMM block size */
#define BITMAP_BIT_CNT CHAR_BIT

/* conversion macros */
#define MEM_TO_BLOCK_IDX(b) \
    ((b) / BLOCK_SIZE)
#define MEM_TO_BLOCK_OFFSET(b)  \
    ((b) % BLOCK_SIZE )
#define SIZE_KB_TO_BLOCKS(kb)   \
    ((kb) * 1024 / BLOCK_SIZE)
#define SIZE_B_TO_BLOCKS(b) \
    ((b) / BLOCK_SIZE + ((b) % BLOCK_SIZE != 0 ? 1 : 0))

#define BLOCK_IDX_TO_BITMAP_IDX(b_idx)  \
    ((b_idx) / BITMAP_BIT_CNT)
#define BLOCK_IDX_TO_BIT_OFFSET(b_idx)  \
    ((b_idx) % BITMAP_BIT_CNT)
#define BITMAP_IDX_TO_BLOCK_IDX(block_idx)  \
    ((block_idx) * BITMAP_BIT_CNT)

#define BLOCK_TO_MEM(idx)   \
    ((void *)((idx) * BLOCK_SIZE))

enum ALIGN {
    ALIGN_LOW,
    ALIGN_HIGH
};

struct pmm_t {
    unsigned int block_cnt;
    unsigned int blocks_free;
    size_t krnl_size;
};

//static char bitmap[1024*1024];
//we use bitmap to mark memory;
static char pmm_map[1024*1024/8];

#define MEMORY_NO_USE 0
#define MEMORY_IN_USE 1

static struct pmm_t pmm;
static unsigned char *mem_bitmap;

size_t get_total_mem_b();
size_t get_free_mem_b();
size_t get_used_mem_b();

int start_block = 0;

/*
 * Initializes PMM.
 * Returns the end of mem_bitmap array
 */
addr_t pmm_init(unsigned int mem_kb, addr_t bitmap_loc)
{
    /* on init set all memory as reservered */
    mem_bitmap = (unsigned char *) bitmap_loc;
    memset(mem_bitmap, 0xFF, SIZE_KB_TO_BLOCKS(mem_kb) / BITMAP_BIT_CNT);
    pmm.block_cnt = SIZE_KB_TO_BLOCKS(mem_kb);
    pmm.blocks_free = 0;
    pmm.krnl_size = 0;
    //wangsl
    mm_operation.get_total_mem = get_total_mem_b;
    mm_operation.get_free_mem = get_free_mem_b;
    mm_operation.get_used_mem = get_used_mem_b;
    memset(pmm_map,0,1024*1024/8);
    //wangsl
    return ((addr_t) mem_bitmap) + (pmm.block_cnt / BITMAP_BIT_CNT) + INT_BIT;
}

/*
 * Returns byte count of total memory.
 */
size_t get_total_mem_b()
{
    return pmm.block_cnt * BLOCK_SIZE;
}

/*
 * Returns byte count of free memory.
 */
size_t get_free_mem_b()
{
    return pmm.blocks_free * BLOCK_SIZE;
}

/*
 * Returns byte count of used memory.
 */
size_t get_used_mem_b()
{
    return (pmm.block_cnt - pmm.blocks_free) * BLOCK_SIZE;
}

size_t get_krnl_size()
{
    return pmm.krnl_size;
}

void set_krnl_size(size_t sz)
{
    pmm.krnl_size = sz;
}

void *pmm_alloc(unsigned int bytes) {

    int block_count = SIZE_B_TO_BLOCKS(bytes);
    int start = start_block;
    int mark_start = start;
    int find_block = 0;

    int mark_index;

    for(;start < pmm.block_cnt;start++) {
        if(get_bit(pmm_map,start) == MEMORY_NO_USE) {
            find_block++;
            mark_start = start;
        } else {
            find_block= 0;
        }

        if(find_block == block_count) {
            
            break;
        }
    }

    if(find_block != block_count) {
        printf("find no memory!!!! \n");
        return NULL;
    }

    //printf("pmm_alloc mark_start is %d,find_block is %d pmm.block_cnt is %d \n",mark_start,find_block,pmm.block_cnt);
    for(;find_block > 0;find_block--) {
        //goto_xy(10,10);
        //printf("mark_start is %d \n",(mark_start + find_block - 1);
        //bitmap[mark_start + find_block - 1] = MEMORY_IN_USE;
        int pos = mark_start + find_block - 1;
        set_bit(pmm_map,pos,MEMORY_IN_USE);
    } 
    
    return mark_start*BLOCK_SIZE;

}

/*
 * Deallocates previously allocated `size` bytes memory area starting
 * as `addr`
 */
void pmm_dealloc_page(addr_t addr)
{
    int block_indx = MEM_TO_BLOCK_IDX(addr);
    set_bit(pmm_map,block_indx,MEMORY_NO_USE);
}

/*
 * Registers a chunk of memory as usable
 * NOTE: BLOCK_SIZE aligned
 * NOTE2: everything what exceeds physical memory address is silently ignored.
 */
int pmm_init_region(unsigned int addr, size_t size)
{
    size_t i, block_cnt;
    unsigned int block_idx;
        
    block_idx = MEM_TO_BLOCK_IDX(addr);
    block_cnt = SIZE_B_TO_BLOCKS(size);

    start_block = block_idx;
    return i;
}


