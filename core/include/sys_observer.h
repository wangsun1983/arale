#ifndef __SYS_OBSERVER_H__
#define __SYS_OBSERVER_H__

#include "ctype.h"

enum SYSTEM_EVENT{
    SYSTEM_EVENT_TIME_TICK = 0,
    SYSTEM_EVENT_SHRINK_MEM_NORMAL,
    SYSTEM_EVENT_SHRINK_MEM_CRITICAL,
    SYSTEM_EVENT_MAX
};

typedef void (*system_monitor)(void *data);

void sys_observer_init();
void sys_observer_regist(uint32_t event,system_monitor monitor);
void sys_observer_remove(uint32_t event,system_monitor monitor);
void sys_observer_notify(uint32_t event,void *data);
void sys_observer_destroy();

#endif
