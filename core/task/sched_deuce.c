/**************************************************************
 CopyRight     :No
 FileName      :sched_deuce.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :simple process scheduler
***************************************************************/

#include "task.h"
#include "sched_deuce.h"
#include "independent_task.h"
#include "dependent_task.h"
#include "cpu.h"
#include "i8259.h"
#include "atomic.h"
#include "log.h"

/*----------------------------------------------
                  local data
----------------------------------------------*/
private task_struct *idle_task;
private int sched_process_status = SCHDE_PROCESS_IDLE;
private spinlock_t sched_lock;

/*----------------------------------------------
                  local declaration
----------------------------------------------*/
typedef void (*task_migrate_fun)(task_struct *task);
public void sched_scheduler(int type);
private void sched_idle(void *args);
private void move_to_waitq(task_struct *task);
private void move_to_runningq(task_struct *task);
private void move_to_runnableq(task_struct *task);
private void move_to_sleepingq(task_struct *task);
private void move_to_destroyq(task_struct *task);
private void sched_task_reset_ticks();
private void sched_task_update(task_struct *task,int toType);
private void sched_task_switch(int sched_type,task_struct *current_task,task_struct *next_task);
private void dump_task_info(int pid,char *msg);

//[from][to]
private task_migrate_fun task_move_fun_map[TASK_STATUS_MAX][TASK_STATUS_MAX] =
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
        NULL,    /*TO:TASK_STATUS_RUNNING*/
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

/*----------------------------------------------
                  extern declaration
----------------------------------------------*/
extern void switch_to(context_struct **current, context_struct *next);

/*----------------------------------------------
                  public method
----------------------------------------------*/
public void sched_init_data(void *sched_task)
{
    task_struct *task = sched_task;
    task->sched_ref.task = task;
    //LOGD("task->sched_ref.task is %x,sched_task is %x \n",task->sched_ref.task,sched_task);
    task->sched_ref.ticks = DEFAULT_TASK_TICKS;
}

public void sched_init(void *sched_task)
{
    task_struct *init_task = sched_task;
    INIT_LIST_HEAD(&taskgroup.runningq);
    INIT_LIST_HEAD(&taskgroup.runnableq);
    INIT_LIST_HEAD(&taskgroup.sleepingq);
    INIT_LIST_HEAD(&taskgroup.waitq);
    SPIN_LOCK_INIT(&sched_lock);
    //INIT_LIST_HEAD(&taskgroup.destroyq);
    //kprintf("===== init,sched_lock is %x \n",&sched_lock);

    sched_init_data(init_task);
    sched_task_update(init_task,TASK_STATUS_RUNNING);

    //create a idle task
    //idle_task = create_dependent_task(sched_idle,NULL);
    //idle_task = task_create(sched_idle,NULL,TASK_TYPE_INDEPENDENT);

    idle_task = task_create(sched_idle,NULL,TASK_TYPE_DEPENDENT);

    sched_task_update(idle_task,TASK_STATUS_SLEEPING);
    //sched_init_data(idle_task);

}

public void sched_finish_task(void *sched_task)
{
    task_struct *task = sched_task;
    //move_to_destroyq(current_task);
    sched_task_update(task,TASK_STATUS_DESTROY);
    sched_scheduler(SCHED_TYPE_FORCE);
    while(1){}
}

public void sched_scheduler(int type)
{
    if(type == SCHED_TYPE_FORCE)
    {
          //kprintf("----------sched_scheduler start \n");
    //    cli();
    }

    if(type == SCHED_TYPE_CLOCK && sched_process_status == SCHDE_PROCESS_DOING)
    {
        return;
    }

    spin_lock(&sched_lock);
    sched_process_status = SCHDE_PROCESS_DOING;

    task_struct *current_task = GET_CURRENT_TASK();
    if(type == SCHED_TYPE_FORCE)
    {
        //kprintf("status is %d,pid %d,task is %x\n",current_task->status,current_task->pid,current_task);
    }
    if(current_task->sched_ref.ticks > 0)
    {
      if(type == SCHED_TYPE_FORCE)
      {
          //kprintf("----------sched_scheduler start_2222222222 \n");
      }
        current_task->sched_ref.ticks--;
        sched_process_status = SCHDE_PROCESS_IDLE;
        spin_unlock(&sched_lock);
        return;
    }
    if(type == SCHED_TYPE_FORCE)
    {
        //kprintf("----------sched_scheduler start_2 \n");
    }
    if(type == SCHED_TYPE_FORCE)
    {
        //LOGD("----------sched_scheduler trace \n");
    }
    //move current_task to waitq
    //LOGD("before current pid is %d status is %d,next status is %d \n"
    //         ,current_task->pid,current_task->status,TASK_STATUS_WAIT);
    if(type == SCHED_TYPE_FORCE)
    {
        //kprintf("------current pid is %d.status is %d \n",current_task->pid,current_task->status);
    }
    sched_task_update(current_task,TASK_STATUS_WAIT);

    //LOGD("after current pid is %d status is %d,next status is %d \n"
    //         ,current_task->pid,current_task->status,TASK_STATUS_WAIT);
    if(!list_empty(&taskgroup.runnableq))
    {
        struct list_head *p = taskgroup.runnableq.next;
        sched_reference *sched_data = list_entry(p,sched_reference,rq_ll);
        //LOGD("sched1,sched_data is %x\n",sched_data);
        task_struct *task = sched_data->task;
        //LOGD("sched1,task pid is %d,status is %d,task is %x \n",task->pid,task->status,task);
        spin_unlock(&sched_lock);
        sched_task_switch(type,current_task,task);
        //LOGD("sched1,sched_task_switch finish \n");
        return;
    }
    if(type == SCHED_TYPE_FORCE)
    {
      //  LOGD("----------sched_scheduler trace3 \n");
    }
    if(list_empty(&taskgroup.waitq))
    {
        //this means all the task are sleeping!!!!!!
        //we should use a idle task to run.......
        if(current_task != idle_task)
        {
            //LOGD("sched2,current_task is %x,task is %x",current_task,idle_task);
            //LOGD("sched2,%d \n",type);
            spin_unlock(&sched_lock);
            sched_task_switch(type,current_task,idle_task);
            return;
        }
    }

    sched_task_reset_ticks();
    sched_task_update(current_task,TASK_STATUS_RUNNING);

end:
    sched_process_status = SCHDE_PROCESS_IDLE;
    spin_unlock(&sched_lock);

    if(type == SCHED_TYPE_CLOCK)
    {
        sti();
        irq_done(IRQ0_VECTOR);
    }
}

public void sched_start_task(void *task)
{
    //move_to_runnableq(task);
    sched_task_update(task,TASK_STATUS_RUNNABLE);
}

//task_struct *sleep_task;
public void sched_sleep(void *sched_task)
{
    //cli();
    task_struct *sleep_task = sched_task;
    //LOGD("sched_sleep1,task111 pid is %d,sched_task is %x,status is %d \n",sleep_task->pid,sleep_task,sleep_task->status);
    //list_del(task);
    sched_task_update(sleep_task,TASK_STATUS_SLEEPING);
    sleep_task->sched_ref.remainder_ticks = sleep_task->sched_ref.ticks;
    sleep_task->sched_ref.ticks = 0;
    //sti();
    //LOGD("sched_sleep2,task222 pid is %d,sched_task is %x,status is %d \n",sleep_task->pid,sleep_task,sleep_task->status);
    sched_scheduler(SCHED_TYPE_FORCE);
    //LOGD("sched_sleep \n");
}

public void sched_wake_up(void *sched_task)
{
    //LOGD("sched_wake_up task is %x \n",sched_task);
    task_struct *task = sched_task;
    sched_task_update(task,TASK_STATUS_RUNNABLE);
    //LOGD("sched_wake_up trace \n");
    task->sched_ref.ticks = task->sched_ref.remainder_ticks;
    task->sched_ref.remainder_ticks = 0;
    //LOGD("sched_wake_up trace1 \n");
    sched_scheduler(SCHED_TYPE_FORCE);
    //LOGD("sched_wake_up trace2 \n");
}

/*----------------------------------------------
                  private method
----------------------------------------------*/
//change all the waitq to runnable queue
private void sched_task_reset_ticks()
{
    struct list_head *p = taskgroup.waitq.next;
    while(p != &taskgroup.waitq && p!= NULL)
    {
        sched_reference *sched_data = list_entry(p,sched_reference,rq_ll);
        task_struct *reset_task = sched_data->task;
        sched_data->ticks = DEFAULT_TASK_TICKS;
        p = sched_data->rq_ll.next;
        //kprintf("\n reset_task pid is %d,reset_task is %x,status is %d \n",reset_task->pid,reset_task,reset_task->status);
        sched_task_update(reset_task,TASK_STATUS_RUNNABLE);
    }
}

private void sched_task_update(task_struct *task,int toType)
{
    //spin_lock(&sched_lock);
    //cli();
    if(task_move_fun_map[task->status][toType] != NULL)
    {
        task_move_fun_map[task->status][toType](task);
    }
    //sti();
    //spin_unlock(&sched_lock);
}

private void sched_task_switch(int type,task_struct *current,task_struct *next)
{
    //LOGD("switch0 current pid is %d,next is %d \n",current->pid,next->pid);
    //LOGD("switch0 next eip is %x,next is %d \n",next->context->eip,next->pid);
    //it is too complex to update current task's status here
    //so we update current status at every scence.
    if(current == idle_task)
    {
        sched_task_update(current,TASK_STATUS_SLEEPING);
    }
    //LOGD("sched_task_switch 1 \n");
    sched_task_update(next,TASK_STATUS_RUNNING);
    //LOGD("sched_task_switch trace1 \n");
    //LOGD("sched_task_switch 2 \n");
    //LOGD("current pid %d,next pid is %d \n",current->pid,next->pid);
    //if current && next is the same process(different thread)
    //there is no need to load pgd again.
    if(current->mm->pgd != next->mm->pgd)
    {
        //LOGD("sched_task_switch 3 \n");
        load_pd((addr_t)next->mm->pgd);
    }
    //LOGD("sched_task_switch trace2 \n");
    //LOGD("sched_task_switch 4 \n");

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
    //LOGD("switch1 current eip is %x,current is %d \n",current->context->eip,current->pid);
    //LOGD("switch1 next eip is %x,next is %d \n",next->context->eip,next->pid);
    //kprintf("switch1 type is %d,next pid is %d \n",type,next->pid);
    switch_to(&current->context,next->context);
    //LOGD("sched_task_switch trace3 \n");
}

private void move_to_waitq(task_struct *task)
{
    if(task->pid == 2)
    {
    //    LOGD("move_to_waitq,pid is %d,task->status is %d \n",task->pid,task->status);
    }
    //LOGD("move to wait queue, pid is %d \n",task->pid);
    task->status = TASK_STATUS_WAIT;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.waitq);
}

private void move_to_runningq(task_struct *task)
{
    if(task->pid != 1)
    {
        //LOGD("move_to_runningq,pid is %d,status is %d \n",task->pid,task->status);
    }
    //LOGD("move_to_runningq,task pid is %d \n",task->pid);
    task->status = TASK_STATUS_RUNNING;
    list_del(&task->sched_ref.rq_ll);
    //current_task = task;
    update_current_task(task);
}

private void move_to_runnableq(task_struct *task)
{
    if(task->pid != 1)
    {
        //LOGD("move_to_runnableq,pid is %d \n",task->pid);
    }
    task->status = TASK_STATUS_RUNNABLE;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.runnableq);
    //LOGD("move_to_runnableq,task pid is %d,eip is %x \n",task->pid,task->context->eip);
    //dump_task_info(task->pid);
}

private void move_to_sleepingq(task_struct *task)
{
    if(task->pid != 1)
    {
        //LOGD("move_to_sleepingq start,pid is %d \n",task->pid);
    }
    task->status = TASK_STATUS_SLEEPING;
    list_del(&task->sched_ref.rq_ll);
    list_add(&task->sched_ref.rq_ll,&taskgroup.sleepingq);
    if(task->pid != 1)
    {
        //LOGD("move_to_sleepingq end,pid is %d \n",task->pid);
    }
}

private void move_to_destroyq(task_struct *task)
{
    task->status = TASK_STATUS_DESTROY;
    task->sched_ref.ticks = 0;

    list_del(&task->sched_ref.rq_ll);
    //because task is reclaimed by #reclaim_dependent_task@task.c
    // or #reclaim_independent_task@task.c
    //list_add(&task->sched_ref.rq_ll,&taskgroup.destroyq);
}


private void sched_idle(void *args)
{
    while(1)
    {
        //Do nothing,just for idle.
    };
}

private void dump_task_info(int pid,char *msg)
{
    struct list_head *p;
    list_for_each(p,&taskgroup.runnableq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            LOGD("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }

    list_for_each(p,&taskgroup.waitq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            LOGD("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }

    list_for_each(p,&taskgroup.runningq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            LOGD("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }

    list_for_each(p,&taskgroup.sleepingq) {
        sched_reference *entry = list_entry(p,sched_reference,rq_ll);
        task_struct *task = entry->task;
        if(task->pid == pid)
        {
            LOGD("%s pid is %d,eip is %x\n",msg,pid,task->context->eip);
            return;
        }
    }
}
