/**************************************************************
 CopyRight     :No
 FileName      :semaphore.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :lock,like wait/notify
***************************************************************/

#include "semaphore.h"
#include "mm.h"
#include "task.h"
#include "log.h"

/*----------------------------------------------
                  public method
----------------------------------------------*/
public semaphore *sem_create()
{
    semaphore *sem = (semaphore*)kmalloc(sizeof(semaphore));
    SPIN_LOCK_INIT(&sem->lock);
    INIT_LIST_HEAD(&sem->wait_list);
    sem->count = 0;
    return sem;
}

public void sem_down(semaphore *sem)
{
    //if(sem->count != 0)
    //{
        sem->count++;
    //}
    //else
    //{
        //this sema has been locked,
        //sechedule my sele
        task_struct *current = GET_CURRENT_TASK();
        list_add(&current->lock_ll,&sem->wait_list);
        //yield_current();
        //LOGD("sem_down,pid is %d \n",current->pid);
        task_sleep(current);
    //}
}

public void sem_up(semaphore *sem)
{
    if(sem->count == 0)
    {
        //no wait,ignore.
        return;
    }

    sem->count--;
    if(!list_empty(&sem->wait_list))
    {
        struct list_head *p = sem->wait_list.next;
        if(p != NULL && p != &sem->wait_list)
        {
            task_struct *task = list_entry(p,task_struct,lock_ll);
            list_del(p);
            task_wake_up(task);
        }
    }
}

public void sem_up_all(semaphore *sem)
{
    sem->count = 0;
    if(!sem->count)
    {
        //we should start to notify all the thread.
        if(!list_empty(&sem->wait_list))
        {
            struct list_head *p = sem->wait_list.next;
            while(p != NULL && p != &sem->wait_list)
            {
                task_struct *task = list_entry(p,task_struct,lock_ll);
                list_del(p);
                task_wake_up(task);
                p = p->next;
            }
        }
    }
}

public void sem_destroy(semaphore *sem)
{
    if(!list_empty(&sem->wait_list))
    {
        LOGE("sem free while other thread hold sem \n");
    }

    free(sem);
}
