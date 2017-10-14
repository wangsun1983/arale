#include "test_task.h"
#include "mutex.h"
#include "task.h"
#include "test_utils.h"
#include "log.h"

int test_mutex_2_data = 2;
mutex *lock1;

void test_mutex_1_fun(void *args)
{
    acquire_lock(lock1);
    LOGD("----test_mutex_1_fun start \n");
    ksleep(200000);
    test_mutex_2_data--;
    LOGD("----test_mutex_1_fun trace1 \n");
    release_lock(lock1);
    LOGD("----test_mutex_1_fun end \n");
}

void test_mutex_2_fun(void *args)
{

    //LOGD("--------------------------test_single_2_fun \n");
    LOGD("----test_mutex_2_fun start \n");
    acquire_lock(lock1);
    LOGD("----test_mutex_2_fun trace1 \n");
    test_mutex_2_data--;
    release_lock(lock1);
    LOGD("----test_mutex_2_fun trace2 \n");
}

int test_mutex_thread_2()
{
    LOGD("test_mutex_thread_2 start \n");
    lock1 = create_mutex();
    LOGD("test_mutex_thread_2 trace1 \n");
    task_struct *task1 = task_create(test_mutex_1_fun,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task2 = task_create(test_mutex_2_fun,NULL,TASK_TYPE_DEPENDENT);
    LOGD("test_mutex_thread_2 trace2,task1 pid is %d,task2 pid is %d \n",task1->pid,task2->pid);
    task_start(task1);
    ksleep(10000);
    task_start(task2);
    ksleep(20000);
    LOGD("test_mutex_thread_2 trace3 \n");
    if(test_mutex_2_data == 0)
    {
        return 1;
    }

    return -1;
}



int start_test_mutex()
{
    TEST_ASSERT(test_mutex_thread_2);
}
