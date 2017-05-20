#include "mm.h"
#include "task.h"

void mm_init(struct boot_info *binfo)
{
    addr_t pmm_tbl_loc = binfo->krnl_loc + KB_TO_BYTE(binfo->krnl_size);
    //printf("binfo->krnl_size is %d",binfo->krnl_size);
    //printf("binfo->memsize is %d",binfo->mem_size);
    //printf("binfo->krnl_loc is %x",binfo->krnl_loc);
    //printf("pmm_tbl_loc is %x",pmm_tbl_loc);
    addr_t pmm_end = pmm_init(binfo->mem_size, pmm_tbl_loc);
    /* first 5MB reserved in boot loader */
    int mem_avail_begin = MB_TO_BYTE(10);
    int mem_avail_end = KB_TO_BYTE(binfo->mem_size);
    pmm_init_region(mem_avail_begin, mem_avail_end - mem_avail_begin);
    set_krnl_size(binfo->krnl_size * 512);

    /* init VMM */
    if (vmm_init(binfo->mem_size, pmm_end,mem_avail_begin))
        kernel_panic("VMM init error");

    return;
}

mm_struct* create_mm()
{
    //struct mm_area_struct *mm = (struct mm_area_struct *)kmalloc();
}

//
void *malloc(size_t bytes)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.malloc(task->mm,bytes);
}

void *vmalloc(size_t bytes)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.vmalloc(task->mm,bytes);
}

void *kmalloc(size_t bytes)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    return (void *)mm_operation.kmalloc(task->mm,bytes);
}

void free(void *p)
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    mm_operation.free(task->mm,p);
}
