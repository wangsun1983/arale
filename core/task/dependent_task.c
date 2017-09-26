#include "dependent_task.h"
#include "task.h"
#include "mutex.h"

/*----------------------------------------------
local data
----------------------------------------------*/
struct list_head dependent_task_pool;

/*----------------------------------------------
local declaration
----------------------------------------------*/
void revert_dependent_task(task_struct *task);
void reclaim_dependent_task();
task_struct *create_dependent_task();
static mutex *task_pool_mutex;

void init_dependent_task(task_module_op *op)
{
    INIT_LIST_HEAD(&dependent_task_pool);
    op->revert_task = revert_dependent_task;
    op->reclaim = reclaim_dependent_task;
    op->create = create_dependent_task;
    task_pool_mutex = create_mutex();
}

void revert_dependent_task(task_struct *task)
{
    acquire_lock(task_pool_mutex);
    list_add(&task->task_pool_ll,&dependent_task_pool);
    release_lock(task_pool_mutex);
}

void reclaim_dependent_task()
{
    //kprintf("reclaim_dependent_task \n");
    acquire_lock(task_pool_mutex);
    //we should free all the task to release memory
    if(!list_empty(&dependent_task_pool))
    {
        struct list_head *p = dependent_task_pool.next;
        while(p != NULL && p != &dependent_task_pool)
        {
            task_struct *task = list_entry(p,task_struct,task_pool_ll);
            p = p->next;
            //free(task->mm);
            free((char *)task->stack_addr);
            free(task);
        }
    }
    release_lock(task_pool_mutex);
}

task_struct *create_dependent_task()
{
    task_struct *task = NULL;
    context_struct *context = NULL;
    int stack_addr = 0;

    acquire_lock(task_pool_mutex);
    if(!list_empty(&dependent_task_pool))
    {
        //kprintf("create_dependent_task trace1 \n");
        struct list_head *p = dependent_task_pool.next;
        list_del(p);

        task = list_entry(p,task_struct,task_pool_ll);
        stack_addr = task->stack_addr;
    }
    else
    {
        task = (task_struct *)kmalloc(sizeof(task_struct));
        context = (context_struct *)kmalloc(THREAD_STACK_SIZE);
        stack_addr = (addr_t)context;
    }
    release_lock(task_pool_mutex);

    kmemset(task,0,sizeof(task_struct));
    context = (context_struct *)stack_addr;
    kmemset(context,0,THREAD_STACK_SIZE);

    task->context = context;
    task->mm = GET_CURRENT_TASK()->mm;
    task->status = TASK_STATUS_INIT;
    task->type = TASK_TYPE_DEPENDENT;
    return task;
}
