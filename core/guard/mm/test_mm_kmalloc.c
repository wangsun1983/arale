#include "test_mm.h"
#include "mm.h"
#include "task.h"
#include "log.h"

//small page test
int test_kmalloc_1()
{
    //start check
    int test_loop = 0;
    while(test_loop < TEST_LOOPS)
    {
        char *test_buff = (char *)kmalloc(TEST_MEMORY_SMALL);
        kmemset(test_buff,TEST_LOOPS%255,TEST_MEMORY_SMALL);
        int index = 0;
        for(;index < TEST_MEMORY_SMALL;index++)
        {
            if(test_buff[index] != TEST_LOOPS%255)
            {
                LOGD("test_kmalloc_1 test fail,index is %d,value is %d \n",index,test_buff[index]);
                return  -1;
            }
        }

        free(test_buff);
        test_loop++;
    }

    return 1;
}

//small page test
int test_kmalloc_2()
{
    //start check
    int test_loop = 0;
    while(test_loop < TEST_LOOPS)
    {
        char *test_buff = (char *)kmalloc(TEST_MEMORY_MEDIUM);
        kmemset(test_buff,TEST_LOOPS%255,TEST_MEMORY_MEDIUM);
        int index = 0;
        for(;index < TEST_MEMORY_MEDIUM;index++)
        {
            if(test_buff[index] != TEST_LOOPS%255)
            {
                LOGD("test_kmalloc_2 test fail,index is %d,value is %d \n",index,test_buff[index]);
                return  -1;
            }
        }

        free(test_buff);
        test_loop++;
    }

    return 1;
}

//large page test
int test_kmalloc_3()
{
    //start check
    int test_loop = 0;
    while(test_loop < TEST_LOOPS)
    {
        char *test_buff = (char *)kmalloc(TEST_MEMORY_LARGE);
        kmemset(test_buff,TEST_LOOPS%255,TEST_MEMORY_LARGE);
        int index = 0;
        for(;index < TEST_MEMORY_LARGE;index++)
        {
            if(test_buff[index] != TEST_LOOPS%255)
            {
                LOGD("test_kmalloc_3 test fail,index is %d,value is %d \n",index,test_buff[index]);
                return  -1;
            }
        }

        free(test_buff);
        test_loop++;
    }

    return 1;
}

int test_kmalloc_4()
{
    task_struct *task1 = task_create(NULL,NULL,TASK_TYPE_DEPENDENT);
    task1->context = kmalloc(sizeof(context_struct));
    kmemset(task1->context,0,sizeof(context_struct));
    task1->context->eip = 0x103d9e;

    task_struct *task2 = task_create(NULL,NULL,TASK_TYPE_DEPENDENT);
    kmemset(task2,0,sizeof(task_struct));
    task2->context = kmalloc(sizeof(context_struct));
    kmemset(task2->context,0,sizeof(context_struct));
    task2->context->eip = 0x103d9e;

    LOGD("context1 eip is %x,context size is %d \n",task1->context->eip,sizeof(context_struct));
    LOGD("context2 eip is %x \n",task2->context->eip);
}
