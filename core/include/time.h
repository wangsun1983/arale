#include "task.h"
#include "list.h"

#ifndef __TIME_H__
#define __TIME_H__

//typedef void (*time_out)(task_struct *task);

typedef struct timer_struct
{
    task_struct *task;
    struct list_head entry;
    uint64_t expire;
    char pad[32]; //TODO if we dont use pad,system may crash,why???
}timer_struct;

void init_timer();
void ksleep(uint32_t sleeptime);

#endif
