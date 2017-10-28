#include "common.h"
#include "sys_call.h"

public int sys_call_thread_create(addr_t runnable,void *data)
{
    return sys_call(SYS_THREAD_CREAT,runnable,(uint32_t)data,0,0,0);
    //return sys_call(SYS_THREAD_CREAT,0,0,0,0,0);
}

public int sys_call_thread_destroy(int tid)
{
    return sys_call(SYS_THREAD_DESTROY,tid,0,0,0,0);
}

public int sys_call_thread_join(int tid)
{
    return sys_call(SYS_THREAD_JOIN,tid,0,0,0,0);
}

public int sys_call_thread_start(uint32_t tid)
{
    return sys_call(SYS_THREAD_START,tid,0,0,0,0);
}

public int sys_call_thread_stop(uint32_t tid)
{
    return sys_call(SYS_THREAD_STOP,tid,0,0,0,0);
}
