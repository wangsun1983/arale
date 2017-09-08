#ifndef __SCHED_DEUCE_H__
#define __SCHED_DEUCE_H__
#include "ctype.h"
#include "list.h"

typedef struct task_queue_group {
    struct list_head runningq;
    struct list_head runnableq;
    struct list_head sleepingq;
    struct list_head waitq;
    struct list_head destroyq;
}task_queue_group;

typedef struct sched_reference {
    void *task;
    uint32_t ticks;
    uint32_t remainder_ticks;
    struct list_head rq_ll;
}sched_reference;

task_queue_group taskgroup;

//public interface
public void scheduler();

public void sched_init();

public void sched_start_task(void *task);

public void sched_finish_task(void *task);

public void sched_remove_task(void *task);

public void sched_add_task(void *task);

public void sched_init_data(void *task);

public void sched_sleep(void *task);

public void sched_wake_up(void *task);

#endif
