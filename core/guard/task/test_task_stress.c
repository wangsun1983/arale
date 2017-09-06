#include "test_task.h"
#include "test_utils.h"
#include "task.h"

int test_data = 0;

void test_create_1_callback(void *args)
{
    kprintf("test_create_1_callback \n");
    test_data++;
}

int test_task_create_1()
{
    int index = 0;

    for(;index < 10;index++)
    {
        task_struct*task = task_create(test_create_1_callback);
        task_start(task);
    }

    if(test_data == index)
    {
        kprintf("test_task_create_1,test_data is %d,index is %d \n",test_data,index);
        return -1;
    }

    return 1;
}


int start_test_stress()
{
    TEST_ASSERT(test_task_create_1);

}
