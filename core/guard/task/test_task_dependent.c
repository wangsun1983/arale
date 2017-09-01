#include "test_task.h"
#include "task.h"
#include "test_utils.h"
#include "pmm.h"
#include "klibc.h"
#include "time.h"

extern void dump_task_info(int pid,char *msg);

//public task_struct *task_create(task_entry_fun runnable,void *data,int type);
int test_single_2_data = 2;
void test_single_2_fun(void *args)
{

    kprintf("--------------------------test_single_2_fun \n");
    test_single_2_data--;
}

void test_single_3_fun(void *args)
{
    kprintf("--------------------------test_single_3_fun \n");
    test_single_2_data--;
}

int start_test_single_2thread()
{
    task_struct *task1 = task_create(test_single_2_fun,NULL,TASK_TYPE_DEPENTENT);
    task_struct *task2 = task_create(test_single_3_fun,NULL,TASK_TYPE_DEPENTENT);
    task_start(task1);
    task_start(task2);

    ksleep(100000);
    if(test_single_2_data == 0)
    {
        return 1;
    }

    return -1;
}

int start_test_single_2thread_from_pool()
{
    kprintf("start_test_single_2thread_from_pool\n");
    task_struct *task1 = task_create(test_single_2_fun,NULL,TASK_TYPE_DEPENTENT);
    task_struct *task2 = task_create(test_single_3_fun,NULL,TASK_TYPE_DEPENTENT);
    task_start(task1);
    task_start(task2);

    ksleep(100000);
    if(test_single_2_data == 0)
    {
        return 1;
    }

    return -1;
}


int start_test_dependent()
{
    TEST_ASSERT(start_test_single_2thread);
    TEST_ASSERT(start_test_single_2thread_from_pool);
}
