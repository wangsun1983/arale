#include "task.h"
#include "list.h"

#ifndef __TIME_H__
#define __TIME_H__

//typedef void (*time_out)(task_struct *task);

typedef struct timer_struct
{
    uint64_t expire;
    task_struct *task;    
    struct list_head entry;
}timer_struct;

void init_timer();
void sleep(uint32_t sleeptime);

#endif


