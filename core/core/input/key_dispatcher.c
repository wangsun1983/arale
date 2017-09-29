#include "key_dispatcher.h"
#include "task.h"
#include "list.h"
#include "semaphore.h"
#include "linkqueue.h"
#include "sys_observer.h"
#include "log.h"

static semaphore *sem;

task_struct *key_dispatch_task = NULL;
static link_queue_t *key_pool;

void key_notifer(void *data)
{
    uint32_t *key = (uint32_t *)data;
    key_event *event = kmalloc(sizeof(key_event));
    kmemset(event,0,sizeof(event));
    event->event = *key;
    link_queue_add_tail(key_pool,event);
    LOGD("key_notifer \n");
    sem_up(sem);
}

void key_dispather(void *data)
{
    while(1)
    {
        uint32_t size = link_queue_size(key_pool);
        //LOGD("size is %x \n",size);
        if(size == 0)
        {
            LOGD("sem1 count is %d \n",sem->count);
            sem_down(sem);
            //LOGD("sem2 count is %d \n",sem->count);
        }

        key_event *event = link_queue_remove_head(key_pool);

        //LOGD("key event is %d \n",event->event);
    }
}

void key_dispatcher_init()
{
    //we should create a task to save
    key_pool = link_queue_create();
    sem = sem_create();
    key_dispatch_task = task_create(key_dispather,NULL,TASK_TYPE_DEPENDENT);
    LOGD("key_dispatch_task pid is %d \n",key_dispatch_task->pid);
    task_start(key_dispatch_task);

    sys_observer_regist(SYSTEM_EVENT_KEY,key_notifer);
}
