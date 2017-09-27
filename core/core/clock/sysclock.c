#include "sysclock.h"
#include "sys_observer.h"
#include "i8253.h"
#include "cpu.h"
#include "klibc.h"
#include "mm.h"
#include "log.h"

#define STATUS_IDLE 1
#define STATUS_INIT 2
int status = STATUS_IDLE;

static unsigned long long pit_jiffy = 0;

void sys_clock_notify()
{
    pit_jiffy++;

    //not initialized,return;
    if(status == STATUS_IDLE)
    {
        return;
    }

    sys_observer_notify(SYSTEM_EVENT_TIME_TICK,NULL);
}

void start_sysclock()
{
    status = STATUS_INIT;
}

/*
void reg_sys_clock_handler(sys_clock_handler handler)
{
     sys_clock_notifer *notifer = (sys_clock_notifer *)kmalloc(sizeof(sys_clock_notifer));
     notifer->handler = handler;
     list_add(&notifer->ll,&clock_notifer.ll);
}
*/

unsigned long long get_jiffy()
{
    return pit_jiffy;
}
