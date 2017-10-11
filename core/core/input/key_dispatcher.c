#include "key_dispatcher.h"
#include "task.h"
#include "list.h"
#include "semaphore.h"
#include "linkqueue.h"
#include "sys_observer.h"
#include "fifo_list.h"
#include "log.h"

static semaphore *sem;

task_struct *key_dispatch_task = NULL;
//static link_queue_t *key_pool;
fifo_list_t *key_buffer;

void key_notifer(void *data)
{

    char *key = (char *)data;
    key_event event;
    event.event = *key;
    fifo_list_push(key_buffer,(char *)&event);
    sem_up(sem);

    #if 0
    kmemset(event,0,sizeof(event));
    event->event = *key;

    link_queue_add_tail(key_pool,event);
    //LOGD("key_notifer event is %d \n",event->event);

    sem_up(sem);
    #endif
}

void key_dispather(void *data)
{
    while(1)
    {
        uint32_t size = fifo_list_get_num(key_buffer);
        //LOGD("size is %x \n",size);
        if(size == 0)
        {
            LOGD("===== sem1 count is %d \n",sem->count);
            sem_down(sem);
            LOGD("===== sem2 count is %d \n",sem->count);
        }

        key_event *event = (key_event *)fifo_list_pop(key_buffer);

        LOGD("key event is %d \n",event->event);
    }
}

void key_dispatcher_init()
{
    //we should create a task to save
    key_buffer = fifo_list_create(128,sizeof(key_event));
    //key_pool = link_queue_create();
    sem = sem_create();
    key_dispatch_task = task_create(key_dispather,NULL,TASK_TYPE_DEPENDENT);
    LOGD("key_dispatch_task pid is %x \n",key_dispatch_task);
    task_start(key_dispatch_task);

    sys_observer_regist(SYSTEM_EVENT_KEY,key_notifer);
}
