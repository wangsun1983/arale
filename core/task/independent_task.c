/**************************************************************
 CopyRight     :No
 FileName      :independent_task.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :dependent task is a process.
                this file provide fuction for operations of dependent
                task
***************************************************************/

#include "dependent_task.h"
#include "task.h"
#include "mm.h"
#include "mmzone.h"
#include "vm_allocator.h"
#include "mutex.h"
#include "log.h"

/*----------------------------------------------
                local data
----------------------------------------------*/
private struct list_head independent_task_pool;
private mutex *task_pool_mutex;

/*----------------------------------------------
                local declaration
----------------------------------------------*/
private void revert_independent_task(task_struct *task);
private void reclaim_independent_task();
private task_struct *create_independent_task();
private uint32_t get_independent_pool_size();

/*----------------------------------------------
                public method
----------------------------------------------*/
public void init_independent_task(task_module_op *op)
{
    INIT_LIST_HEAD(&independent_task_pool);
    op->revert_task = revert_independent_task;
    op->reclaim = reclaim_independent_task;
    op->create = create_independent_task;
    op->get_task_pool_size = get_independent_pool_size;
    task_pool_mutex = create_mutex();
}

/*----------------------------------------------
                private method
----------------------------------------------*/
private void revert_independent_task(task_struct *task)
{
    acquire_lock(task_pool_mutex);
    list_add(&task->task_pool_ll,&independent_task_pool);
    release_lock(task_pool_mutex);
}

private void reclaim_independent_task()
{
    //LOGD("reclaim_independent_task \n");
    acquire_lock(task_pool_mutex);
    //we should free all the task to release memory
    if(!list_empty(&independent_task_pool))
    {
        struct list_head *p = independent_task_pool.next;
        while(p != NULL && p != &independent_task_pool)
        {
            task_struct *task = list_entry(p,task_struct,task_pool_ll);
            p = p->next;
            pfree(task->mm->pte_user);
            pfree(task->mm->pgd);
            free(task->mm);
            pfree((char *)task->stack_addr);
            free(task);
        }
    }

    release_lock(task_pool_mutex);
}

private task_struct *create_independent_task()
{
    addr_t i = 0;
    int index = 0;
    task_struct *task = (task_struct *)kmalloc(sizeof(task_struct));

    task->status = TASK_STATUS_INIT;
    task->pid = task_id;
    task_id++;

    //create memory struct
    mm_struct *_mm = (mm_struct *)kmalloc(sizeof(mm_struct)); //struct need physical address
    _mm->pte_user = (addr_t *)pmalloc(sizeof(addr_t)*PD_ENTRY_CNT*PT_ENTRY_CNT*3/4);
    _mm->pgd = (addr_t *)pmalloc(sizeof(addr_t) * PD_ENTRY_CNT);//??
    _mm->userroot = vm_allocator_init(1024*1024*1024,(uint32_t)1024*1024*1024*3); //user space is 1~3G
    _mm->vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);
    //_mm->pgd = core_mem.pgd;
    _mm->pte_core = core_mem.pte_core;

    //core memory is always same~~~~~
    for (i = 0; i < memory_range_user.start_pgd; i++) {
        _mm->pgd[i] = core_mem.pgd[i];
    }

    //LOGD("task init end i is %d, start i is %d \n",i,memory_range_user.start_pgd);
    int user_index = 0;
    //because kmalloc is continus memory,so there is no need to
    //compute pa again.
    for(i = memory_range_user.start_pgd; i < PD_ENTRY_CNT; i++) {
        _mm->pgd[i] = (addr_t)&_mm->pte_user[user_index*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        user_index++;
    }
    //LOGD("task alloc 4 \n");
    addr_t *_pte = (addr_t *)_mm->pte_user;
    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT*3/4; i++) {
         //goto_xy(20,20);
         //LOGD("task alloc i is %x \n",i);
        _pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
    }
    //LOGD("task alloc 5 \n");
    task->mm = _mm;
    //task->context = (context_struct *)kmalloc(THREAD_STACK_SIZE);
    task->context = (context_struct *)pmalloc(THREAD_STACK_SIZE);
    kmemset(task->context,0,THREAD_STACK_SIZE);
    //TODO?????? maybe!!!.haha
    //task->context = (context_struct *)((addr_t)task->context + THREAD_STACK_SIZE);
    task->stack_addr = (addr_t)task->context;
    task->context = (context_struct *)(task->stack_addr + THREAD_STACK_SIZE);

    return task;
}

private uint32_t get_independent_pool_size()
{
    uint32_t size = list_get_size(&independent_task_pool);
}
