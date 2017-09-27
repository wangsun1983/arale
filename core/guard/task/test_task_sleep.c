#include "test_task.h"
#include "test_utils.h"
#include "task.h"
#include "time.h"
#include "sysclock.h"
#include "log.h"

int start_test_sleep_1()
{
    unsigned long long jiffies = get_jiffy();
    ksleep(10000);
    LOGD("sleep finish \n");;
    //TODO
    return 1;
}

void test_sleep_1_fun(void *args)
{
    LOGD("test_sleep_1_fun start \n");
    ksleep(200000);
    LOGD("test_sleep_1_fun end \n");
}

void test_sleep_2_fun(void *args)
{
    LOGD("test_sleep_2_fun start \n");
    ksleep(100000);
    LOGD("test_sleep_2_fun end \n");
}

void test_sleep_3_fun(void *args)
{
    LOGD("test_sleep_3_fun start \n");
    ksleep(300000);
    LOGD("test_sleep_3_fun end \n");
}


int start_test_sleep_2()
{
    LOGD("test_mutex_thread_2 start \n");
    task_struct *task1 = task_create(test_sleep_1_fun,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task2 = task_create(test_sleep_2_fun,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task3 = task_create(test_sleep_3_fun,NULL,TASK_TYPE_DEPENDENT);
    task_start(task1);
    task_start(task2);
    task_start(task3);
}


int start_test_sleep()
{
    //TEST_ASSERT(start_test_sleep_1);
    TEST_ASSERT(start_test_sleep_2);

}
