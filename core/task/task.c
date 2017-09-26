#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"
#include "mmzone.h"
#include "vm_allocator.h"
#include "i8259.h"
#include "cpu.h"
#include "sysclock.h"
#include "klibc.h"
#include "independent_task.h"
#include "dependent_task.h"
#include "sys_observer.h"

/*----------------------------------------------
local data
----------------------------------------------*/
task_struct *current_task;
task_struct init_thread;
task_module_op task_ops[TASK_TYPE_MAX];
char* init_context[THREAD_STACK_SIZE];
struct list_head destroy_task_list;

/*----------------------------------------------
local declaration
----------------------------------------------*/
void task_sys_clock_handler();


void task_init(struct boot_info *binfo)
{
    task_id = 0;

    INIT_LIST_HEAD(&destroy_task_list);

    kmemset(&init_thread,0,sizeof(task_struct));
    kmemset(init_context,0,THREAD_STACK_SIZE);

    //init_dependent_task_pool();

    init_dependent_task(&task_ops[TASK_TYPE_DEPENDENT]);
    init_independent_task(&task_ops[TASK_TYPE_INDEPENDENT]);

    current_task = &init_thread;
    current_task->pid = task_id;
    current_task->mm = get_root_mm();
    current_task->status = TASK_STATUS_RUNNING;
    current_task->context = (context_struct *)init_context;
    //current_task->context = &init_context;

    task_id++;

    sched_init(current_task);

    //reg_sys_clock_handler(task_sys_clock_handler);
    //sys_observer_regist(SYSTEM_EVENT_TIME_TICK,task_sys_clock_handler);
    //create task reclaim task
}

void do_exit()
{
    task_ops[current_task->type].revert_task(current_task);
    sched_finish_task(current_task);
    while(1){}
}

void task_reclaim_normal(void *data)
{
    task_ops[TASK_TYPE_DEPENDENT].reclaim();
    //task_ops[TASK_TYPE_INDEPENDENT].reclaim();
}

void task_reclaim_critical(void *data)
{
    task_ops[TASK_TYPE_DEPENDENT].reclaim();
    task_ops[TASK_TYPE_INDEPENDENT].reclaim();
}

void task_entry()
{
    current_task->_entry(current_task->_entry_data);
    do_exit();
}

task_struct *task_create(task_entry_fun runnable,void *data,int type)
{
    //task_struct *task = kmalloc(sizeof(task_struct));
    task_struct *task = task_ops[type].create();

    //we should init sched data
    sched_init_data(task);

    task->pid = task_id;
    task->context->eip = (uint32_t)task_entry;
    //kprintf("eip is %x,task is %d \n",task->context->eip,task->pid);
    task->_entry = runnable;
    task->_entry_data = data;
    task_id++;

    return task;
}

void task_start(task_struct *task)
{
    //kprintf("start task pid is %d \n",task->pid);
    sched_start_task(task);
}

task_struct* GET_CURRENT_TASK()
{
    return current_task;
}

void update_current_task(task_struct *task)
{
    current_task = task;
}

void task_scheduler()
{
    //sched_scheduler();
}

void task_sys_clock_handler()
{
    sched_scheduler(SCHED_TYPE_CLOCK);
}

void task_wake_up(task_struct *task)
{
    kprintf("task_wake_up task is %x \n",task);
    sched_wake_up(task);
}

void task_sleep(task_struct *task)
{
    sched_sleep(task);
}

void task_start_sched()
{
    sys_observer_regist(SYSTEM_EVENT_TIME_TICK,task_sys_clock_handler);
    sys_observer_regist(SYSTEM_EVENT_SHRINK_MEM_NORMAL,task_reclaim_normal);
    sys_observer_regist(SYSTEM_EVENT_SHRINK_MEM_CRITICAL,task_reclaim_critical);
}
