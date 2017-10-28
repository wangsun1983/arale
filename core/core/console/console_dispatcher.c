#include "console_dispatcher.h"
#include "task.h"
#include "list.h"
#include "semaphore.h"
#include "linkqueue.h"
#include "sys_observer.h"
#include "fifo_list.h"
#include "log.h"

static semaphore *sem;

task_struct *console_dispatch_task = NULL;
//static link_queue_t *key_pool;
fifo_list_t *console_buffer;

void console_notifer(void *data)
{
    //LOGD("console_notify \n");

    char *key = (char *)data;
    console_event event;
    event.event = *key;
    fifo_list_push(console_buffer,(char *)&event);
    sem_up(sem);

}

void console_dispather(void *data)
{
    while(1)
    {
        uint32_t size = fifo_list_get_num(console_buffer);
        //LOGD("size is %x \n",size);
        if(size == 0)
        {
            //LOGD("===== sem1 count is %d \n",sem->count);
            sem_down(sem);
            //LOGD("===== sem2 count is %d \n",sem->count);
        }

        console_event *event = (console_event *)fifo_list_pop(console_buffer);

        LOGD("%c",event->event);
    }
}

void console_dispatcher_init()
{
    //we should create a task to save
    console_buffer = fifo_list_create(128,sizeof(console_event));
    //key_pool = link_queue_create();
    sem = sem_create();
    console_dispatch_task = task_create(console_dispather,NULL,TASK_TYPE_DEPENDENT);
    LOGD("console_dispatcher_init");
    task_start(console_dispatch_task);

    sys_observer_regist(SYSTEM_EVENT_CONSOLE,console_notifer);
}
