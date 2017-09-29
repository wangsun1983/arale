#include "test_task.h"
#include "semaphore.h"
#include "test_utils.h"
#include "log.h"
#include "task.h"

semaphore *sem;

void task_run_1(void *data)
{
    LOGD("task_run_1 start \n");
    sem_down(sem);
    LOGD("task_run_1 end \n");
}

void task_run_2(void *data)
{
    LOGD("task_run_2 start \n");
    sem_up(sem);
    LOGD("task_run_2 end \n");
}

int test_semaphore_1()
{
    sem = sem_create();
    task_struct *task1 = task_create(task_run_1,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task2 = task_create(task_run_2,NULL,TASK_TYPE_DEPENDENT);
    LOGD("test_semaphore_1 start \n");
    task_start(task1);
    ksleep(100000);
    LOGD("test_semaphore_1 trace1 \n");
    task_start(task2);
    LOGD("test_semaphore_1 end \n");
    return 1;
}




int start_test_semaphore()
{
    TEST_ASSERT(test_semaphore_1);

}
