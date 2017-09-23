#include "trigger.h"
#include "mm.h"

trigger_t *creat_trigger(int max_trigger)
{
    trigger_t *trigger = kmalloc(sizeof(trigger_t));
    kmemset(trigger,0,sizeof(trigger_t));
    //kprintf("creat_trigger start,trigger is %x \n",trigger);
    trigger->trigger_num = max_trigger;
    trigger->bitmap = (char *)create_bitmap(max_trigger);
    trigger->global_listener_list = kmalloc(sizeof(struct list_head)*max_trigger);
    kmemset((char *)trigger->global_listener_list,0,sizeof(struct list_head)*max_trigger);
    //kprintf("creat_trigger end \n");
    // /kprintf("creat_trigger start,trigger->bitmap is %x \n",trigger->bitmap);
    return trigger;
}

void register_trigger(trigger_t *trigger,uint32_t type,g_listener listener)
{
    if(get_bit(trigger->bitmap,type) == TRIGGER_IDLE)
    {
        //kprintf("------- init!!! \n");
        INIT_LIST_HEAD(&trigger->global_listener_list[type]);
        set_bit(trigger->bitmap,type,TRIGGER_INITED);
        //kprintf("------- init  end !!! \n");
    }

    global_listener *li = (global_listener *)kmalloc(sizeof(global_listener));
    kmemset(li,0,sizeof(global_listener));
    li->g_listener = listener;
    list_add(&li->ll,&trigger->global_listener_list[type]);
    //kprintf("regist trigger is %x \n",li);
}

void notify_trigger(trigger_t *trigger,uint32_t type,void *data)
{
    struct list_head *p = NULL;
    //kprintf("notify_trigger start \n");
    list_for_each(p,&trigger->global_listener_list[type]) {
        global_listener *listener = (global_listener *)list_entry(p,global_listener,ll);
        //kprintf("notify_trigger trace,listener is %x \n",listener);
        listener->g_listener(data);
    }
}

void remove_trigger(trigger_t *trigger,uint32_t type,g_listener l)
{
    struct list_head *p = NULL;
    global_listener *listener = NULL;
    list_for_each(p,&trigger->global_listener_list[type]) {
        listener = (global_listener *)list_entry(p,global_listener,ll);
        if(listener->g_listener == l)
        {
            list_del(p);
            break;
        }
    }

    if(listener != NULL)
    {
        free(listener);
    }
}

void destroy_trigger(trigger_t *trigger)
{
    uint32_t trigger_index = 0;
    while(trigger_index < trigger->trigger_num)
    {
        struct list_head *head = &trigger->global_listener_list[trigger_index];
        //we should release all listener
        struct list_head *p = head->next;
        while(p != NULL && p != head)
        {
            struct list_head *next = p->next;
            free(p);
            p = next;
        }
        trigger_index++;
    }

    free(trigger->global_listener_list);
    free(trigger->bitmap);
    free(trigger);
}
