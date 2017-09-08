#include "list.h"
#include "ctype.h"
#include "task.h"
#include "sysclock.h"
#include "time.h"
#include "mm.h"

struct list_head timer_list;

void reg_timer(uint32_t expire,task_struct *task)
{
    timer_struct *timer = (timer_struct *)kmalloc(sizeof(timer_struct));
    kprintf("reg_timer start,timer is %x \n",timer);
    timer->expire = expire;
    timer->task = task;
    //list_add(&timer->entry,&timer_list)
    //kprintf("reg_timer trace1 \n");

    if(list_empty(&timer_list))
    {
        list_add(&timer->entry,&timer_list);
        return;
    }

    struct list_head *p = NULL;
    list_for_each(p,&timer_list) {
        //kprintf("reg_timer trace1_1 \n");
        timer_struct *t = list_entry(p,timer_struct,entry);
        //kprintf("reg_timer trace1_2 t is %x \n",t);
        if(t->expire > expire)
        {
            //kprintf("reg_timer trace1_3 \n");
            list_add(&timer->entry,t->entry.prev);
            //kprintf("reg_timer trace1_4 \n");
            return;
        }
    }
}

void sleep(uint32_t sleeptime)
{
    kprintf("sleep start!!!!! \n");
    task_struct *current = GET_CURRENT_TASK();
    reg_timer(sleeptime + get_jiffy(),current);
    task_sleep(current);
}

void time_out(timer_struct *timer)
{
    kprintf("time_out \n");
    task_wake_up(timer->task);
    kprintf("time_out0,timer is %x \n",timer);
    free(timer);
    kprintf("time_out1 \n");
}

void sys_timer_handler()
{
    unsigned long long jiffies = get_jiffy();

    if(!list_empty(&timer_list))
    {
        struct list_head *p = timer_list.next;
        while(p != &timer_list && p!= NULL)
        {
            timer_struct *timer = list_entry(p,timer_struct,entry);
            p = timer->entry.next;
            if(timer->expire <= jiffies)
            {
                list_del(&timer->entry);
                time_out(timer);
                continue;
            }
        }
        //kprintf("time_out3 \n");
    }
}

void init_timer()
{
    INIT_LIST_HEAD(&timer_list);
    reg_sys_clock_handler(sys_timer_handler);
}
