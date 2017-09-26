#include "mutex.h"
#include "list.h"
#include "klibc.h"
#include "atomic.h"
#include "task.h"

mutex *create_mutex()
{
    mutex *lock = (mutex *)kmalloc(sizeof(mutex));
    kmemset(lock,0,sizeof(mutex));
    INIT_LIST_HEAD(&lock->wait_list);
    SPIN_LOCK_INIT(&lock->spin_lock);
    lock->held_pid = -1;
    return lock;
}

void acquire_lock(mutex *lock)
{
    if(lock == NULL)
    {
        kprintf("acquire null lock \n");  
    }

    spin_lock(&lock->spin_lock);
    task_struct *current = GET_CURRENT_TASK();
    if(lock->held_pid == current->pid)
    {
        return;
    }

    if(lock->status == MUTEX_IDLE)
    {
        lock->status = MUTEX_HELD;
        lock->held_pid = current->pid;
        spin_unlock(&lock->spin_lock);
    }
    else
    {
        list_add(&current->lock_ll,&lock->wait_list);
        spin_unlock(&lock->spin_lock);
        task_sleep(current);
    }
}

void release_lock(mutex *lock)
{
    //kprintf("mutex release lock \n");
    spin_lock(&lock->spin_lock);
    task_struct *current = GET_CURRENT_TASK();
    if(lock->status == MUTEX_IDLE)
    {
        kprintf("double free lock \n");
        spin_unlock(&lock->spin_lock);
        return ;
    }

    if(!list_empty(&lock->wait_list))
    {
        //kprintf("mutex release lock list_empty\n");
        struct list_head *p = lock->wait_list.next;
        task_struct *task = list_entry(p,task_struct,lock_ll);
        list_del(p);
        lock->held_pid = task->pid;
        spin_unlock(&lock->spin_lock);
        task_wake_up(task);
        return;
    }

    //kprintf("mutex release lock list_empty\n");
    lock->status = MUTEX_IDLE;
    lock->held_pid = -1;
    spin_unlock(&lock->spin_lock);
}
