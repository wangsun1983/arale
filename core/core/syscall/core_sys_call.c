#include "core_sys_call.h"
#include "log.h"

public int core_syscall_handler(uint32_t sys_call_id,uint32_t a1, uint32_t a2,
  uint32_t a3, uint32_t a4, uint32_t a5)
{
    //LOGD("wangsl,core_syscall_handler,sys_call_id is %d, a1 is %d \n",sys_call_id,a1);

    switch(sys_call_id)
    {
        case CORE_SYS_CALL_GETPID:
            return core_sys_call_getpid();
        break;

        case CORE_SYS_CALL_MALLOC:
        //TODO
        break;

        case CORE_SYS_CALL_FREE:
        //TODO
        break;

        case CORE_SYS_PUTCHAR:
            core_sys_call_putchar((char *)&a1);
        break;

        case CORE_SYS_THREAD_CREAT:
            return core_sys_call_thread_create(a1,a2);
        break;

        case CORE_SYS_THREAD_START:
            return core_sys_call_thread_start(a1);
        break;

        case CORE_SYS_THREAD_STOP:
        //TODO
        break;

        case CORE_SYS_THREAD_DESTROY:
        //TODO
        break;

        case CORE_SYS_THREAD_JOIN:
        //TODO
        break;
    }

    return 0;
}
