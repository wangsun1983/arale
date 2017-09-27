#include "test_task.h"
#include "task.h"
#include "test_utils.h"
#include "pmm.h"
#include "klibc.h"
#include "time.h"
#include "log.h"

//public task_struct *task_create(task_entry_fun runnable,void *data,int type);
int test_indep_data = 2;
void test_indep_fun(void *args)
{

    LOGD("--------------------------test_ff_single_2_fun \n");
    test_indep_data--;
}

int start_test_indep_single_2thread()
{
    task_struct *task1 = task_create(test_indep_fun,NULL,TASK_TYPE_INDEPENDENT);
    task_struct *task2 = task_create(test_indep_fun,NULL,TASK_TYPE_INDEPENDENT);
    task_start(task1);
    task_start(task2);

    ksleep(100000);
    if(test_indep_data == 0)
    {
        return 1;
    }

    return -1;
}


int start_test_independent()
{
    TEST_ASSERT(start_test_indep_single_2thread);
}
