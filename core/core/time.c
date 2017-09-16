#include "list.h"
#include "ctype.h"
#include "task.h"
#include "sysclock.h"
#include "time.h"
#include "mm.h"

struct list_head timer_list;

/*
typedef struct test_struct {
  uint64_t expire;
  task_struct *task;
  struct list_head entry;
  char pad[32];
}test_struct;

test_struct *data;
*/

void reg_timer(uint32_t expire,task_struct *task)
{
    timer_struct *timer = (timer_struct *)kmalloc(sizeof(timer_struct));
    //kprintf("reg_timer start,timer is %x \n",timer);
    timer->expire = expire;
    timer->task = task;
    //kprintf("reg_timer start,timer->task is %x \n",timer->task);
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
    task_struct *current = GET_CURRENT_TASK();
    reg_timer(sleeptime + get_jiffy(),current);
    kprintf("sleep start!!!!!,current is %x \n",current);
    task_sleep(current);
}

void time_out(task_struct *task)
{
    kprintf("time_out1,task is %x\n",task);
    task_wake_up(task);
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
                task_struct *task = timer->task;
                free(timer);
                time_out(task);
                continue;
            }
        }
    }
}

void init_timer()
{
    INIT_LIST_HEAD(&timer_list);
    reg_sys_clock_handler(sys_timer_handler);
}
