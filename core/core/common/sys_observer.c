#include "sys_observer.h"
#include "trigger.h"
#include "log.h"

trigger_t *global_trigger_data = NULL;

void sys_observer_init()
{
    global_trigger_data = creat_trigger(SYSTEM_EVENT_MAX);
}

void sys_observer_regist(uint32_t event,system_monitor monitor)
{
    register_trigger(global_trigger_data,event,monitor);
}

void sys_observer_remove(uint32_t event,system_monitor monitor)
{
    remove_trigger(global_trigger_data,event,monitor);
}

void sys_observer_notify(uint32_t event,void *data)
{
    notify_trigger(global_trigger_data,event,data);
}

void sys_observer_destroy()
{
    destroy_trigger(global_trigger_data);
}
