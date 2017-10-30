/**************************************************************
 CopyRight     :No
 FileName      :semaphore.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :task interface(include create/start/sleep),
                all the archievements is in sched_XXX.c
***************************************************************/

#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"
#include "mmzone.h"
#include "vm_allocator.h"
#include "i8259.h"
#include "cpu.h"
#include "sysclock.h"
#include "gdt.h"
#include "klibc.h"
#include "independent_task.h"
#include "dependent_task.h"
#include "sys_observer.h"
#include "log.h"

/*----------------------------------------------
                local data
----------------------------------------------*/
private task_struct *current_task;
private task_struct init_thread;
private uint32_t task_id;
private task_module_op task_ops[TASK_TYPE_MAX];
private char* init_context[THREAD_STACK_SIZE];
private struct list_head destroy_task_list;
private struct list_head user_process_list;

/*----------------------------------------------
               local declaration
----------------------------------------------*/
private void task_sys_clock_handler();
private void task_entry();
private void task_reclaim_normal(void *data);
private void task_reclaim_critical(void *data);
private task_struct *task_search_user_process(uint32_t pid);

public task_struct *main_thread;

/*----------------------------------------------
                public method
----------------------------------------------*/
public void task_init(struct boot_info *binfo)
{
    task_id = 0;

    INIT_LIST_HEAD(&destroy_task_list);
    INIT_LIST_HEAD(&user_process_list);


    kmemset(&init_thread,0,sizeof(task_struct));
    kmemset(init_context,0,THREAD_STACK_SIZE);

    //init_dependent_task_pool();

    init_dependent_task(&task_ops[TASK_TYPE_DEPENDENT]);
    init_independent_task(&task_ops[TASK_TYPE_INDEPENDENT]);

    current_task = &init_thread;
    current_task->pid = task_id;
    current_task->mm = get_root_mm();
    current_task->status = TASK_STATUS_RUNNING;
    current_task->context = NULL;//(context_struct *)init_context;
    //current_task->context = &init_context;
    //addr_t cs;
    //addr_t ds;
    //__asm __volatile("movl %%cs,%0" : "=r" (cs));
    //__asm __volatile("movl %%ds,%0" : "=r" (ds));
    current_task->thread_cs = _KERNEL_CS_;
    current_task->thread_ds = _KERNEL_DS_;

    main_thread = current_task;

    task_id++;

    sched_init(current_task);

    //reg_sys_clock_handler(task_sys_clock_handler);
    //sys_observer_regist(SYSTEM_EVENT_TIME_TICK,task_sys_clock_handler);
    //create task reclaim task
}

public task_struct *task_create(task_entry_fun runnable,void *data,int type)
{
    //task_struct *task = kmalloc(sizeof(task_struct));
    task_struct *task = task_ops[type].create();

    //we should init sched data
    sched_init_data(task);

    task->pid = task_id;
    task->context->eip = (uint32_t)task_entry;
    //LOGD("eip is %x,task is %d \n",task->context->eip,task->pid);
    task->_entry = runnable;
    task->_entry_data = data;
    task_id++;
    task->thread_ds = _KERNEL_DS_;
    task->thread_cs = _KERNEL_CS_;

    return task;
}

public task_struct *task_create_user_process(task_entry_fun runnable,void *data,int type)
{
    task_struct *task = task_create(runnable,data,type);
    task->thread_ds = _USER_DS_ ;
    task->thread_cs = _USER_CS_ ;
    //LOGD("user process tid is %d,runnable is %x \n",task->pid,runnable);
    //record user list_head
    list_add(&task->user_task_ll,&user_process_list);
    return task;
}

public void task_start(task_struct *task)
{
    //LOGD("start task pid is %d \n",task->pid);
    sched_start_task(task);
}

public task_struct* GET_CURRENT_TASK()
{
    return current_task;
}

public void update_current_task(task_struct *task)
{
    current_task = task;
}

public void task_wake_up(task_struct *task)
{
    //LOGD("task_wake_up task is %x \n",task);
    sched_wake_up(task);
}

public void task_sleep(task_struct *task)
{
    //LOGD("task_sleep pid is %x \n",task->pid);
    sched_sleep(task);
}

public void task_start_sched()
{
    sys_observer_regist(SYSTEM_EVENT_TIME_TICK,task_sys_clock_handler);
    sys_observer_regist(SYSTEM_EVENT_SHRINK_MEM_NORMAL,task_reclaim_normal);
    sys_observer_regist(SYSTEM_EVENT_SHRINK_MEM_CRITICAL,task_reclaim_critical);
}

public uint32_t get_all_task_pool_size()
{
    return task_ops[TASK_TYPE_DEPENDENT].get_task_pool_size()
           + task_ops[TASK_TYPE_INDEPENDENT].get_task_pool_size();
}

public void task_start_user_process(int pid)
{
    task_struct *task = task_search_user_process(pid);
    //LOGD("task start pid is %d \n",task->pid);
    task_start(task);
}
/*----------------------------------------------
                private method
----------------------------------------------*/
private void do_exit()
{
    task_ops[current_task->type].revert_task(current_task);
    sched_finish_task(current_task);
    while(1){}
}

private void task_reclaim_normal(void *data)
{
    task_ops[TASK_TYPE_DEPENDENT].reclaim();
    //task_ops[TASK_TYPE_INDEPENDENT].reclaim();
}

private void task_reclaim_critical(void *data)
{
    //task_ops[TASK_TYPE_DEPENDENT].reclaim();
    //task_ops[TASK_TYPE_INDEPENDENT].reclaim();
}

private void task_entry()
{
    current_task->_entry(current_task->_entry_data);
    do_exit();
}

private void task_sys_clock_handler()
{
    sched_scheduler(SCHED_TYPE_CLOCK);
}

private task_struct *task_search_user_process(uint32_t pid)
{
    struct list_head *p;
    list_for_each(p,&user_process_list) {
        task_struct *task = list_entry(p,task_struct,user_task_ll);
        if(task->pid == pid)
        {
            return task;
        }
    }
    return NULL;
}
