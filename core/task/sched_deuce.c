#include "task.h"
#include "sched_deuce.h"
#include "independent_task.h"
#include "dependent_task.h"
#include "cpu.h"
#include "i8259.h"

/*----------------------------------------------
local data
----------------------------------------------*/
task_struct *idle_task;
int sched_process_status = SCHDE_PROCESS_IDLE;

/*----------------------------------------------
local declaration
----------------------------------------------*/
typedef void (*task_migrate_fun)(task_struct *task);
void sched_idle(void *args);
void move_to_waitq(task_struct *task);
void move_to_runningq(task_struct *task);
void move_to_runnableq(task_struct *task);
void move_to_sleepingq(task_struct *task);
void move_to_destroyq(task_struct *task);
void sched_scheduler(int type);
void sched_task_reset_ticks();
void sched_task_update(task_struct *task,int toType);
void sched_task_switch(int sched_type,task_struct *current_task,task_struct *next_task);
void dump_task_info(int pid,char *msg);

/*----------------------------------------------
extern declaration
----------------------------------------------*/
extern void switch_to(context_struct **current, context_struct *next);

int begin = 0;

//[from][to]
task_migrate_fun task_move_fun_map[TASK_STATUS_MAX][TASK_STATUS_MAX] =
{
    //FROM:TASK_STATUS_NONE
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        move_to_runnableq,   /*TO:TASK_STATUS_RUNNABLE*/
        move_to_runningq,    /*TO:TASK_STATUS_RUNNING*/
        move_to_waitq,       /*TO:TASK_STATUS_WAIT*/
        move_to_sleepingq,   /*TO:TASK_STATUS_SLEEPING*/
        move_to_destroyq     /*TO:TASK_STATUS_DESTROY*/
    },

    //FROM:TASK_STATUS_INIT
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        move_to_runnableq,   /*TO:TASK_STATUS_RUNNABLE*/
        move_to_runningq,    /*TO:TASK_STATUS_RUNNING*/
        move_to_waitq,       /*TO:TASK_STATUS_WAIT*/
        move_to_sleepingq,   /*TO:TASK_STATUS_SLEEPING*/
        move_to_destroyq     /*TO:TASK_STATUS_DESTROY*/
    },

    //FROM:TASK_STATUS_RUNNABLE
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        NULL,                /*TO:TASK_STATUS_RUNNABLE*/
        move_to_runningq,    /*TO:TASK_STATUS_RUNNING*/
        move_to_waitq,       /*TO:TASK_STATUS_WAIT*/
        move_to_sleepingq,   /*TO:TASK_STATUS_SLEEPING*/
        move_to_destroyq     /*TO:TASK_STATUS_DESTROY*/
    },

    //FROM:TASK_STATUS_RUNNING
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        move_to_runnableq,   /*TO:TASK_STATUS_RUNNABLE*/
        NULL,                /*TO:TASK_STATUS_RUNNING*/
        move_to_waitq,       /*TO:TASK_STATUS_WAIT*/
        move_to_sleepingq,   /*TO:TASK_STATUS_SLEEPING*/
        move_to_destroyq     /*TO:TASK_STATUS_DESTROY*/
    },

    //FROM:TASK_STATUS_WAIT
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        move_to_runnableq,   /*TO:TASK_STATUS_RUNNABLE*/
        move_to_runningq,    /*TO:TASK_STATUS_RUNNING*/
        NULL,                /*TO:TASK_STATUS_WAIT*/
        move_to_sleepingq,   /*TO:TASK_STATUS_SLEEPING*/
        move_to_destroyq     /*TO:TASK_STATUS_DESTROY*/
    },
    //FROM:TASK_STATUS_SLEEPING
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        move_to_runnableq,   /*TO:TASK_STATUS_RUNNABLE*/
        move_to_runningq,    /*TO:TASK_STATUS_RUNNING*/
        NULL,                /*TO:TASK_STATUS_WAIT*/
        NULL,                /*TO:TASK_STATUS_SLEEPING*/
        move_to_destroyq     /*TO:TASK_STATUS_DESTROY*/
    },
    //FROM:TASK_STATUS_DESTROY
    {
        NULL,                /*TO:TASK_STATUS_NONE*/
        NULL,                /*TO:TASK_STATUS_INIT*/
        NULL,                /*TO:TASK_STATUS_RUNNABLE*/
        NULL,                /*TO:TASK_STATUS_RUNNING*/
        NULL,                /*TO:TASK_STATUS_WAIT*/
        NULL,                /*TO:TASK_STATUS_SLEEPING*/
        NULL                 /*TO:TASK_STATUS_DESTROY*/
    }
};


void sched_init_data(void *sched_task)
{
    task_struct *task = sched_task;
    task->sched_ref.task = task;
    //kprintf("task->sched_ref.task is %x,sched_task is %x \n",task->sched_ref.task,sched_task);
    task->sched_ref.ticks = DEFAULT_TASK_TICKS;
}

void sched_init(void *sched_task)
{
    task_struct *init_task = sched_task;
    INIT_LIST_HEAD(&taskgroup.runningq);
    INIT_LIST_HEAD(&taskgroup.runnableq);
    INIT_LIST_HEAD(&taskgroup.sleepingq);
    INIT_LIST_HEAD(&taskgroup.waitq);
    //INIT_LIST_HEAD(&taskgroup.destroyq);

    sched_init_data(init_task);
    sched_task_update(init_task,TASK_STATUS_RUNNING);

    //create a idle task
    //idle_task = create_dependent_task(sched_idle,NULL);
    //idle_task = task_create(sched_idle,NULL,TASK_TYPE_INDEPENDENT);

    idle_task = task_create(sched_idle,NULL,TASK_TYPE_DEPENTENT);

    sched_task_update(idle_task,TASK_STATUS_SLEEPING);
    //sched_init_data(idle_task);

}

void sched_finish_task(void *sched_task)
{
    task_struct *task = sched_task;
    //move_to_destroyq(current_task);
    sched_task_update(task,TASK_STATUS_DESTROY);
    sched_scheduler(SCHED_TYPE_FORCE);
    while(1){}
}

void sched_scheduler(int type)
{
    if(sched_process_status == SCHDE_PROCESS_DOING)
    {
        return;
    }

    sched_process_status = SCHDE_PROCESS_DOING;

    task_struct *current_task = GET_CURRENT_TASK();

    if(current_task->sched_ref.ticks > 0)
    {
        current_task->sched_ref.ticks--;
        sched_process_status = SCHDE_PROCESS_IDLE;
        return;
    }

    //move current_task to waitq
    //kprintf("before current pid is %d status is %d,next status is %d \n"
    //         ,current_task->pid,current_task->status,TASK_STATUS_WAIT);

    sched_task_update(current_task,TASK_STATUS_WAIT);
    if(begin)
    {
       kprintf("sched_scheduler trace3 \n");
    }
    //kprintf("after current pid is %d status is %d,next status is %d \n"
    //         ,current_task->pid,current_task->status,TASK_STATUS_WAIT);
    if(!list_empty(&taskgroup.runnableq))
    {
        struct list_head *p = taskgroup.runnableq.next;
        sched_reference *sched_data = list_entry(p,sched_reference,rq_ll);
        //kprintf("sched1,sched_data is %x\n",sched_data);
        task_struct *task = sched_data->task;
        kprintf("sched1,task pid is %d,status is %d,task is %x \n",task->pid,task->status,task);
        sched_task_switch(type,current_task,task);
        //kprintf("sched1,sched_task_switch finish \n");
        return;
    }
    if(begin)
    {
       kprintf("sched_scheduler trace4 \n");
    }
    if(list_empty(&taskgroup.waitq))
    {
        //this means all the task are sleeping!!!!!!
        //we should use a idle task to run.......
        if(current_task != idle_task)
        {
            //kprintf("sched2,current_task is %x,task is %x",current_task,idle_task);
            kprintf("sched2,%d \n",type);
            sched_task_switch(type,current_task,idle_task);
            return;
        }
    }
    if(begin)
    {
       kprintf("sched_scheduler trace5 \n");
    }
    //kprintf("sched2,trace2 \n");
    sched_task_reset_ticks();
    //kprintf("sched2,trace3 \n");
    sched_task_update(current_task,TASK_STATUS_RUNNING);

end:
    if(type == SCHED_TYPE_CLOCK)
    {
        sti();
        irq_done(IRQ0_VECTOR);
    }
    sched_process_status = SCHDE_PROCESS_IDLE;
}

//change all the waitq to runnable queue
void sched_task_reset_ticks()
{
    struct list_head *p = taskgroup.waitq.next;
    while(p != &taskgroup.waitq && p!= NULL)
    {
        sched_reference *sched_data = list_entry(p,sched_reference,rq_ll);
        task_struct *reset_task = sched_data->task;
        sched_data->ticks = DEFAULT_TASK_TICKS;
        p = sched_data->rq_ll.next;
        sched_task_update(reset_task,TASK_STATUS_RUNNABLE);
    }
}

void sched_task_update(task_struct *task,int toType)
{
    if(task_move_fun_map[task->status][toType] != NULL)
    {
        task_move_fun_map[task->status][toType](task);
    }
}

void sched_task_switch(int type,task_struct *current,task_struct *next)
{
    kprintf("switch0 current pid is %d,next is %d \n",current->pid,next->pid);
    //kprintf("switch0 next eip is %x,next is %d \n",next->context->eip,next->pid);
    //it is too complex to update current task's status here
    //so we update current status at every scence.
    if(current == idle_task)
    {
        sched_task_update(current,TASK_STATUS_SLEEPING);
    }
    //kprintf("sched_task_switch 1 \n");
    sched_task_update(next,TASK_STATUS_RUNNING);
    //kprintf("sched_task_switch trace1 \n");
    //kprintf("sched_task_switch 2 \n");
    //kprintf("current pid %d,next pid is %d \n",current->pid,next->pid);
    //if current && next is the same process(different thread)
    //there is no need to load pgd again.
    if(current->mm->pgd != next->mm->pgd)
    {
        //kprintf("sched_task_switch 3 \n");
        load_pd((addr_t)next->mm->pgd);
    }
    //kprintf("sched_task_switch trace2 \n");
    //kprintf("sched_task_switch 4 \n");

    //if(type == SCHED_TYPE_CLOCK)
    //{
    //    sti();
    //    irq_done(IRQ0_VECTOR);
    //}
    if(type == SCHED_TYPE_CLOCK)
    {
        irq_done(IRQ0_VECTOR);
    }
    sched_process_status = SCHDE_PROCESS_IDLE;
    //kprintf("switch1 current eip is %x,current is %d \n",current->context->eip,current->pid);
    //kprintf("switch1 next eip is %x,next is %d \n",next->context->eip,next->pid);
    switch_to(&current->context,next->context);
    //kprintf("sched_task_switch trace3 \n");
}

void sched_start_task(void *task)
{
    //move_to_runnableq(task);
    sched_task_update(task,TASK_STATUS_RUNNABLE);
}

void sched_sleep(void *sched_task)
{
    task_struct *task = sched_task;
    sched_task_update(task,TASK_STATUS_SLEEPING);
    task->sched_ref.remainder_ticks = task->sched_ref.ticks;
    task->sched_ref.ticks = 0;
    sched_scheduler(SCHED_TYPE_FORCE);
    //kprintf("sched_sleep \n");
}

void sched_wake_up(void *sched_task)
{
    //kprintf("sched_wake_up task is %x \n",sched_task);
    task_struct *task = sched_task;
    sched_task_update(task,TASK_STATUS_RUNNABLE);
    //kprintf("sched_wake_up trace \n");
    task->sched_ref.ticks = task->sched_ref.remainder_ticks;
    task->sched_ref.remainder_ticks = 0;
    //kprintf("sched_wake_up trace1 \n");
    //begin = 1;
    sched_scheduler(SCHED_TYPE_FORCE);
    //kprintf("sched_wake_up trace2 \n");
}

void move_to_waitq(task_struct *task)
{
    if(task->pid != 1)
    {
        //kprintf("move_to_waitq,pid is %d \n",task->pid);
    }
    //kprintf("move to wait queue, pid is %d \n",task->pid);
    task->status = TASK_STATUS_WAIT;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.waitq);
}

void move_to_runningq(task_struct *task)
{
    if(task->pid != 1)
    {
        //kprintf("move_to_runningq,pid is %d \n",task->pid);
    }
    //kprintf("move_to_runningq,task pid is %d \n",task->pid);
    task->status = TASK_STATUS_RUNNING;
    list_del(&task->sched_ref.rq_ll);
    //current_task = task;
    update_current_task(task);
}

void move_to_runnableq(task_struct *task)
{
    if(task->pid != 1)
    {
        //kprintf("move_to_runnableq,pid is %d \n",task->pid);
    }
    task->status = TASK_STATUS_RUNNABLE;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.runnableq);
    //kprintf("move_to_runnableq,task pid is %d,eip is %x \n",task->pid,task->context->eip);
    //dump_task_info(task->pid);
}

void move_to_sleepingq(task_struct *task)
{
    if(task->pid != 1)
    {
        //kprintf("move_to_sleepingq,pid is %d \n",task->pid);
    }
    task->status = TASK_STATUS_SLEEPING;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.sleepingq);
}

void move_to_destroyq(task_struct *task)
{
    task->status = TASK_STATUS_DESTROY;
    task->sched_ref.ticks = 0;

    list_del(&task->sched_ref.rq_ll);
    //because task is reclaimed by #reclaim_dependent_task@task.c
    // or #reclaim_independent_task@task.c
    //list_add(&task->sched_ref.rq_ll,&taskgroup.destroyq);
}


void sched_idle(void *args)
{
    while(1)
    {
        //Do nothing,just for idle.
    };
}

void dump_task_info(int pid,char *msg)
{
    struct list_head *p;
    list_for_each(p,&taskgroup.runnableq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            kprintf("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }

    list_for_each(p,&taskgroup.waitq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            kprintf("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }

    list_for_each(p,&taskgroup.runningq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            kprintf("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }

    list_for_each(p,&taskgroup.sleepingq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            kprintf("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }
}
