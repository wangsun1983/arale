#include "core_sys_call.h"
#include "sys_observer.h"

public void core_sys_call_putchar(char *msg)
{
    //kputchar(msg);
    sys_observer_notify(SYSTEM_EVENT_CONSOLE,msg);
}
