#include "sysclock.h"
#include "i8253.h"
#include "cpu.h"

#define STATUS_IDLE 1
#define STATUS_INIT 2
int status = STATUS_IDLE;

extern void refresh();

static unsigned long long pit_jiffy = 0;

sys_clock_notifer clock_notifer = {NULL,{NULL,NULL}};

void sys_clock_notify()
{
    pit_jiffy++;

    //not initialized,return;
    if(status == STATUS_IDLE)
    {
        return;
    }

    list_head *p;
    
    list_for_each(p,&clock_notifer.ll) {
        sys_clock_notifer *notifer = list_entry(p,sys_clock_notifer,ll);
        if(notifer->handler != NULL) {
            notifer->handler();
        }
    }
}

void init_sysclock()
{
    INIT_LIST_HEAD(&clock_notifer.ll);
}

void start_sysclock() 
{
    status = STATUS_INIT;
}

void reg_sys_clock_handler(sys_clock_handler handler)
{
     sys_clock_notifer *notifer = (sys_clock_notifer *)kmalloc(sizeof(sys_clock_notifer));
     notifer->handler = handler;
     list_add(&notifer->ll,&clock_notifer.ll);
}


unsigned long long get_jiffy()
{
    return pit_jiffy;
}
