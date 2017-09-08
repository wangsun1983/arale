#include "task.h"
#include "sched_deuce.h"
#include "independent_task.h"
#include "dependent_task.h"
#include "cpu.h"
#include "i8259.h"

task_struct *idle_task;

/*----------------------------------------------*/
//local declar
/*----------------------------------------------*/
void sched_idle(void *args);
void move_to_waitq(task_struct *task);
void move_to_runningq(task_struct *task);
void move_to_runnableq(task_struct *task);
void move_to_sleepingq(task_struct *task);
void move_to_destroyq(task_struct *task);
void sched_scheduler();
void sched_task_reset_ticks();
void sched_task_update(task_struct *task,int toType);
void sched_task_switch(task_struct *current_task,task_struct *next_task);

typedef void (*task_migrate_fun)(task_struct *task);

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
    task->sched_ref.ticks = DEFAULT_TASK_TICKS;
}

void sched_init(void *sched_task)
{
    task_struct *init_task = sched_task;
    INIT_LIST_HEAD(&taskgroup.runningq);
    INIT_LIST_HEAD(&taskgroup.runnableq);
    INIT_LIST_HEAD(&taskgroup.sleepingq);
    INIT_LIST_HEAD(&taskgroup.waitq);
    INIT_LIST_HEAD(&taskgroup.destroyq);

    sched_init_data(init_task);
    sched_task_update(init_task,TASK_STATUS_RUNNING);

    //create a idle task
    //idle_task = create_dependent_task(sched_idle,NULL);
    idle_task = task_create(sched_idle,NULL,TASK_TYPE_DEPENTENT);
    sched_init_data(idle_task);
}

void sched_finish_task(void *sched_task)
{
    task_struct *task = sched_task;
    //move_to_destroyq(current_task);
    sched_task_update(task,TASK_STATUS_DESTROY);
    sched_scheduler();
    while(1){}
}

void sched_scheduler()
{
    if(begin)
    {
       kprintf("sched_scheduler trace1 \n");
    }

    task_struct *current_task = GET_CURRENT_TASK();

    if(begin)
    {
       kprintf("sched_scheduler trace1,current_task is %x \n",current_task);
       kprintf("sched_scheduler trace1,ticks is %x \n",current_task->sched_ref.ticks);
    }

    if(current_task->sched_ref.ticks > 0)
    {
        current_task->sched_ref.ticks--;
        return;
    }
    if(begin)
    {
       kprintf("sched_scheduler trace2 \n");
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
        task_struct *task = sched_data->task;

        kprintf("sched1,current_task is %x,task is %x\n",current_task->pid,task->pid);
        sched_task_switch(current_task,task);
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
            kprintf("sched2,current_task is %x,task is %x",current_task,idle_task);
            sched_task_switch(current_task,idle_task);
            return;
        }
    }
    if(begin)
    {
       kprintf("sched_scheduler trace5 \n");
    }
    kprintf("sched2,trace2 \n");
    sched_task_reset_ticks();
    kprintf("sched2,trace3 \n");
    sched_task_update(current_task,TASK_STATUS_RUNNING);
}

//change all the waitq to runnable queue
void sched_task_reset_ticks()
{
    struct list_head *p = taskgroup.waitq.next;
    while(p != &taskgroup.waitq && p!= NULL)
    {
        sched_reference *sched_data = list_entry(p,sched_reference,rq_ll);
        task_struct *task = sched_data->task;
        sched_data->ticks = DEFAULT_TASK_TICKS;
        p = sched_data->rq_ll.next;

        //move_to_runnableq(task);
        sched_task_update(task,TASK_STATUS_RUNNABLE);
    }
}

void sched_task_update(task_struct *task,int toType)
{
    if(task_move_fun_map[task->status][toType] != NULL)
    {
        task_move_fun_map[task->status][toType](task);
    }
}

void sched_task_switch(task_struct *current,task_struct *next)
{
    irq_done(IRQ0_VECTOR);
    //kprintf("sched_task_switch,current pid is %d,next pid is %d",current->pid,next->pid);

    //it is too complex to update current task's status here
    //so we update current status at every scence.
    if(current == idle_task)
    {
        sched_task_update(current,TASK_STATUS_SLEEPING);
    }
    //kprintf("sched_task_switch 1 \n");

    sched_task_update(next,TASK_STATUS_RUNNING);
    //kprintf("sched_task_switch 2 \n");
    load_pd((addr_t)next->mm->pgd);
    //kprintf("sched_task_switch 3 \n");
    switch_to(&current->context,next->context);
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
    sched_scheduler();
}

void sched_wake_up(void *sched_task)
{
    kprintf("sched_wake_up task is %x \n",sched_task);
    task_struct *task = sched_task;
    sched_task_update(task,TASK_STATUS_RUNNABLE);
    kprintf("sched_wake_up trace \n");
    task->sched_ref.ticks = task->sched_ref.remainder_ticks;
    task->sched_ref.remainder_ticks = 0;
    kprintf("sched_wake_up trace1 \n");
    begin = 1;
    sched_scheduler();
    kprintf("sched_wake_up trace2 \n");
}

void move_to_waitq(task_struct *task)
{
    //kprintf("move to wait queue, pid is %d \n",task->pid);
    task->status = TASK_STATUS_WAIT;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.waitq);
}

void move_to_runningq(task_struct *task)
{
    //kprintf("move_to_runningq,task pid is %d \n",task->pid);
    task->status = TASK_STATUS_RUNNING;
    list_del(&task->sched_ref.rq_ll);
    //current_task = task;
    update_current_task(task);
}

void move_to_runnableq(task_struct *task)
{
    //kprintf("move_to_runnableq,task pid is %d \n",task->pid);
    task->status = TASK_STATUS_RUNNABLE;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.runnableq);
}

void move_to_sleepingq(task_struct *task)
{
    task->status = TASK_STATUS_SLEEPING;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.sleepingq);
}

void move_to_destroyq(task_struct *task)
{
    task->status = TASK_STATUS_DESTROY;
    task->ticks = 0;

    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.destroyq);
}


void sched_idle(void *args)
{
    while(1)
    {
        //Do nothing,just for idle.
    };
}
