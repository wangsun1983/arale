#include "pthread.h"
#include "sys_call.h"

int pthread_create(addr_t runnable,void *data)
{
    return sys_call_thread_create(runnable,data);
}

int pthread_start(uint32_t tid)
{
    return sys_call_thread_start(tid);
}

int pthread_destroy(int tid)
{
    return 0;
}

int pthread_join(int tid)
{
    return 0;
}
