#ifndef __SCHED_DEUCE_H__
#define __SCHED_DEUCE_H__
#include "ctype.h"
#include "list.h"
#include "sched.h"

typedef struct task_queue_group {
    struct list_head runningq;
    struct list_head runnableq;
    struct list_head sleepingq;
    struct list_head waitq;
    //struct list_head destroyq;
}task_queue_group;

typedef struct sched_reference {
    void *task;
    uint32_t ticks;
    uint32_t remainder_ticks;
    struct list_head rq_ll;
    char pad[32];
}sched_reference;

enum sched_process_status{
    SCHDE_PROCESS_DOING = 0,
    SCHDE_PROCESS_IDLE
};

task_queue_group taskgroup;

#endif
