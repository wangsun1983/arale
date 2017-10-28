#include "core_sys_call.h"
#include "task.h"
#include "log.h"

public int core_sys_call_thread_create(addr_t runnable,void *data)
{
    task_struct *task = task_create_user_process(runnable,data,TASK_TYPE_DEPENDENT);
    //LOGD("creat task_pid is %d \n",task->pid);
    return task->pid;
    //return 1;
}

public int core_sys_call_thread_destroy(int tid)
{
    //TODO
    return  0;
}

public int core_sys_call_thread_join(int tid)
{
    //TODO
    return  0;
}

public int core_sys_call_thread_start(uint32_t tid)
{
    //LOGD("core call tid is %d \n",tid);
    task_start_user_process(tid);

    return  0;
}

public int core_sys_call_thread_stop(uint32_t tid)
{
    //TODO
    return  0;
}
