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

task_struct *current_task; //the default pid is 0

task_struct init_thread;
context_struct init_context;
void task_sys_clock_handler();

void task_init(struct boot_info *binfo)
{
    task_id = 0;
    kmemset(&init_thread,0,sizeof(task_struct));
    kmemset(&init_context,0,sizeof(context_struct));

    current_task = &init_thread;
    current_task->pid = task_id;
    current_task->mm = get_root_mm();
    current_task->status = TASK_STATUS_RUNNING;
    current_task->context = &init_context;

    task_id++;

    sched_init(current_task);
    reg_sys_clock_handler(task_sys_clock_handler);

}

void do_exit()
{
    sched_finish_task(current_task);
    while(1){}
}

void task_entry()
{
    current_task->_entry(current_task->_entry_data);
    do_exit();
}

task_struct *task_create(task_entry_fun runnable,void *data,int type)
{
    task_struct *task = NULL;

    switch(type)
    {
        case TASK_TYPE_INDEPENDENT:
        {
            task = create_independent_task();
        }
        break;

        case TASK_TYPE_DEPENTENT:
        {
            task = create_dependent_task();
        }
        break;

        default:
            return NULL;
    }

    //we should init sched data
    sched_init_data(task);

    task->pid = task_id;
    task->context->eip = (uint32_t)task_entry;
    task->_entry = runnable;
    task->_entry_data = data;
    task_id++;

    return task;
}

void task_start(task_struct *task)
{
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
    sched_scheduler();
}

void task_sys_clock_handler()
{
    sched_scheduler();
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
