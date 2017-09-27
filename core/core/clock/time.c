#include "list.h"
#include "ctype.h"
#include "task.h"
#include "sysclock.h"
#include "time.h"
#include "mm.h"
#include "sys_observer.h"
#include "log.h"

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
    //LOGD("reg_timer start,task pid is  %d \n",task->pid);
    timer->expire = expire;
    timer->task = task;
    //LOGD("reg_timer start,timer->task is %x \n",timer->task);
    //list_add(&timer->entry,&timer_list)
    //LOGD("reg_timer trace1 \n");

    if(list_empty(&timer_list))
    {
        //LOGD("reg_timer empty trace1_3 \n");
        list_add(&timer->entry,&timer_list);
        return;
    }

    struct list_head *p = NULL;
    timer_struct *loop_task;
    list_for_each(p,&timer_list) {
        //LOGD("reg_timer trace1_1 \n");
        //timer_struct *t = list_entry(p,timer_struct,entry);
        loop_task = list_entry(p,timer_struct,entry);
        //LOGD("reg_timer trace1_2 t is %x \n",t);
        if(loop_task->expire >= expire)
        {
            LOGD("reg_timer trace1_3 \n");
            list_add(&timer->entry,loop_task->entry.prev);
            //LOGD("reg_timer trace1_4 \n");
            return;
        }
    }

    //this is the final one
    list_add_tail(&timer->entry,&loop_task->entry);
}

void ksleep(uint32_t sleeptime)
{
    task_struct *current = GET_CURRENT_TASK();
    reg_timer(sleeptime + get_jiffy(),current);
    //LOGD("sleep start!!!!!,current pid is %x \n",current->pid);
    task_sleep(current);
}

void time_out(task_struct *task)
{
    //LOGD("time_out1,task pid is %d\n",task->pid);
    task_wake_up(task);
}

void sys_timer_handler(void *data)
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
    //reg_sys_clock_handler(sys_timer_handler);
    sys_observer_regist(SYSTEM_EVENT_TIME_TICK,sys_timer_handler);
}
