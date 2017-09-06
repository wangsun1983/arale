#include "test_mm.h"
#include "mm.h"



//small page test
int test_pmalloc_1()
{
    //start check
    int test_loop = 0;
    while(test_loop < TEST_LOOPS)
    {
        char *test_buff = (char *)pmalloc(TEST_MEMORY_SMALL);
        kmemset(test_buff,TEST_LOOPS%255,TEST_MEMORY_SMALL);
        int index = 0;
        for(;index < TEST_MEMORY_SMALL;index++)
        {
            if(test_buff[index] != TEST_LOOPS%255)
            {
                kprintf("test_pmalloc_1 test fail,index is %d,value is %d \n",index,test_buff[index]);
                return  -1;
            }
        }

        free(test_buff);
        test_loop++;
    }

    return 1;
}

//small page test
int test_pmalloc_2()
{
    //start check
    int test_loop = 0;
    while(test_loop < TEST_LOOPS)
    {
        char *test_buff = (char *)pmalloc(TEST_MEMORY_MEDIUM);
        kmemset(test_buff,TEST_LOOPS%255,TEST_MEMORY_MEDIUM);
        int index = 0;
        for(;index < TEST_MEMORY_MEDIUM;index++)
        {
            if(test_buff[index] != TEST_LOOPS%255)
            {
                kprintf("test_pmalloc_2 test fail,index is %d,value is %d \n",index,test_buff[index]);
                return  -1;
            }
        }

        free(test_buff);
        test_loop++;
    }

    return 1;
}

//large page test
int test_pmalloc_3()
{
    //start check
    int test_loop = 0;
    while(test_loop < TEST_LOOPS)
    {
        char *test_buff = (char *)pmalloc(TEST_MEMORY_LARGE);
        kmemset(test_buff,TEST_LOOPS%255,TEST_MEMORY_LARGE);
        int index = 0;
        for(;index < TEST_MEMORY_LARGE;index++)
        {
            if(test_buff[index] != TEST_LOOPS%255)
            {
                kprintf("test_pmalloc_3 test fail,index is %d,value is %d \n",index,test_buff[index]);
                return  -1;
            }
        }

        free(test_buff);
        test_loop++;
    }

    return 1;
}
