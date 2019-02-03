/**************************************************************
 CopyRight     :No
 FileName      :mm.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :Memory Handle Interface
 History       
 20190201    * Add debug log macro
             * Add comment
***************************************************************/

#include "mm.h"
#include "error.h"
#include "task.h"
#include "mmzone.h"
#include "log.h"
#include "mm_common.h"

//mm_init debug log
bool is_debug_mm_init = false;
#define MM_INIT_LOG(fmt,...) \
if(is_debug_mm_init) \
({\
    LOGD(fmt,##__VA_ARGS__);\
})

/*----------------------------------------------
                public method
----------------------------------------------*/

/*
* Memory init interface
*/
public void mm_init(struct boot_info *binfo)
{
    //kernel's image has been reflect to memory from 0x100000(KERNEL_PMODE_BASE)
    MM_INIT_LOG("binfo->krnl_size is %x \n",KB_TO_BYTE(binfo->krnl_size));
    MM_INIT_LOG("binfo->memsize is %d \n",binfo->mem_size);
    MM_INIT_LOG("binfo->krnl_loc is %d \n",binfo->krnl_loc);
    
    //add a memory safe barrier to make memory save,haha.
    //TODO? kerenl memory should be more exact.
    int mem_avail_begin= MB_TO_BYTE(10);
    int mem_avail_end = KB_TO_BYTE(binfo->mem_size);

    //make memory_avail_begin && mem_avail_end 4K align
    mem_avail_begin = PAGE_SIZE_RUND_UP(mem_avail_begin);
    mem_avail_end = PAGE_SIZE_RUND_DOWN(mem_avail_end);

    MM_INIT_LOG("mem_avail_begin2 is %d \n",mem_avail_begin);
    MM_INIT_LOG("mem_avail_end2 is %d \n",mem_avail_end);

    mm_zone_init(mem_avail_begin,mem_avail_end - mem_avail_begin);

    /* init VMM */
    if (mm_initiator_start(0, 0,mem_avail_begin))
    {
        kernel_panic("VMM init error");
    }
    
    MM_INIT_LOG("mm.c memory_range_user is %x,memory_range_user.start_pgd is %x,start_pte is %x \n",
        &memory_range_user,
        memory_range_user.start_pgd,
        memory_range_user.start_pte);

    return;
}

/*
* get virtual memory interface for user space
*/
public void *malloc(size_t bytes)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.malloc(task->mm,bytes);
}

/*
* get virtual memory interface for core
*/
public void *vmalloc(size_t bytes)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.vmalloc(task->mm,bytes);
}

/*
* get physical memory fro core
*/
public void *kmalloc(size_t bytes)
{
    return (void *)mm_operation.kmalloc(bytes);
}

/*
* get continuous physical memory for 4K align for core
*/
public void *pmalloc(size_t bytes)
{
    return (void *)mm_operation.pmalloc(bytes);
}

/*
* free pmemory
*/
public void pfree(void *p)
{
    mm_operation.pfree((addr_t)p);
}

/*
* free memory
*/
public void free(void *p)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    mm_operation.free(task->mm,(addr_t)p);
}
