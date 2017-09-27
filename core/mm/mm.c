#include "mm.h"
#include "error.h"
#include "task.h"
#include "mmzone.h"
#include "log.h"

void mm_init(struct boot_info *binfo)
{

    addr_t pmm_tbl_loc = binfo->krnl_loc + KB_TO_BYTE(binfo->krnl_size);
    //LOGD("binfo->krnl_size is %d",binfo->krnl_size);
    //LOGD("binfo->memsize is %d",binfo->mem_size);
    //LOGD("binfo->krnl_loc is %x",binfo->krnl_loc);
    //LOGD("pmm_tbl_loc is %x",pmm_tbl_loc);
    //addr_t pmm_end = pmm_init(binfo->mem_size, pmm_tbl_loc);

    /* first 5MB reserved in boot loader */
    int mem_avail_begin = MB_TO_BYTE(10);
    int mem_avail_end = KB_TO_BYTE(binfo->mem_size);

    //pmm_init_region(mem_avail_begin, mem_avail_end - mem_avail_begin);
    //set_krnl_size(binfo->krnl_size * 512);
    mm_zone_init(mem_avail_begin,mem_avail_end - mem_avail_begin);

    /* init VMM */
    if (vmm_init(0, 0,mem_avail_begin))
        kernel_panic("VMM init error");

    //LOGD("mm.c memory_range_user is %x,memory_range_user.start_pgd is %x,start_pte is %x \n",
    //&memory_range_user,
    //memory_range_user.start_pgd,
    //m/emory_range_user.start_pte);

    return;
}

//
void *malloc(size_t bytes)
{
    //LOGD("malloc \n");
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.malloc(task->mm,bytes);
}

void *vmalloc(size_t bytes)
{
    //LOGD("vmalloc \n");
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.vmalloc(task->mm,bytes);
}

void *kmalloc(size_t bytes)
{
    //LOGD("kvmalloc \n");
    //LOGD("wangsl,mm:kmalloc start \n");
    //task_struct *task = (task_struct *)GET_CURRENT_TASK();
    //LOGD("wangsl,mm:kmalloc start \n");
    return (void *)mm_operation.kmalloc(bytes);
}

void *pmalloc(size_t bytes)
{
    //LOGD("wangsl,mm:kmalloc start \n");
    //task_struct *task = (task_struct *)GET_CURRENT_TASK();
    //LOGD("wangsl,mm:kmalloc start \n");
    return (void *)mm_operation.pmalloc(bytes);
}

void pfree(void *p)
{
    mm_operation.pfree((addr_t)p);
}

void free(void *p)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    //LOGD("free p is %x,mm is %x \n",p,task->mm);
    mm_operation.free(task->mm,(addr_t)p);
}
