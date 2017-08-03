#include "list.h"
#include "ctype.h"
#include "task.h"
#include "sysclock.h"
#include "time.h"

struct list_head timer_list;

void sys_timer_handler()
{
    unsigned long long jiffies = get_jiffy();

    if(!list_empty(&timer_list)) 
    {
       //TODO
    }
}

void reg_timer(uint32_t expire,task_struct *task)
{
    timer_struct *timer = (timer_struct *)kmalloc(sizeof(timer_struct));
    timer->expire = expire;
    timer->task = task;
    //list_add(&timer->entry,&timer_list)
    struct list_head *p;
    list_for_each(p,&timer_list) {
        timer_struct *t = list_entry(p,timer_struct,entry);
        if(t->expire > expire)
        {
            list_add(&timer->entry,t->entry.prev);
            return;
        }
    }

    if(p == NULL) 
    {
        p = &timer_list;
    }

    list_add(&timer->entry,p);
}

void init_timer()
{
    INIT_LIST_HEAD(&timer_list);
    reg_sys_clock_handler(sys_timer_handler);
}

void sleep(uint32_t sleeptime)
{
    task_struct *current = GET_CURRENT_TASK();
    reg_timer(sleeptime + get_jiffy(),current);
    dormant_task(current);
}



void time_out(timer_struct *timer)
{
    wake_up_task(timer->task);
    free(timer);
}
