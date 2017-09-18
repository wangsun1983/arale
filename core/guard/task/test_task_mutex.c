#include "test_task.h"
#include "mutex.h"
#include "task.h"
#include "test_utils.h"

int test_mutex_2_data = 2;
mutex *lock1;

void test_mutex_1_fun(void *args)
{
    acquire_lock(lock1);
    kprintf("----test_mutex_1_fun start \n");
    ksleep(200000);
    test_mutex_2_data--;
    kprintf("----test_mutex_1_fun trace1 \n");
    release_lock(lock1);
    kprintf("----test_mutex_1_fun end \n");
}

void test_mutex_2_fun(void *args)
{

    //kprintf("--------------------------test_single_2_fun \n");
    kprintf("----test_mutex_2_fun start \n");
    acquire_lock(lock1);
    kprintf("----test_mutex_2_fun trace1 \n");
    test_mutex_2_data--;
    release_lock(lock1);
    kprintf("----test_mutex_2_fun trace2 \n");
}

int test_mutex_thread_2()
{
    kprintf("test_mutex_thread_2 start \n");
    lock1 = create_mutex();
    kprintf("test_mutex_thread_2 trace1 \n");
    task_struct *task1 = task_create(test_mutex_1_fun,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task2 = task_create(test_mutex_2_fun,NULL,TASK_TYPE_DEPENDENT);
    kprintf("test_mutex_thread_2 trace2,task1 pid is %d,task2 pid is %d \n",task1->pid,task2->pid);
    task_start(task1);
    ksleep(100000);
    task_start(task2);
    ksleep(200000);
    kprintf("test_mutex_thread_2 trace3 \n");
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
