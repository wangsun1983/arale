#include "test_task.h"
#include "test_utils.h"
#include "task.h"
#include "time.h"


int start_test_sleep_1()
{
    unsigned long long jiffies = get_jiffy();
    sleep(10000);
    kprintf("sleep finish \n");;
    //TODO
    return 1;
}


int start_test_sleep()
{
    TEST_ASSERT(start_test_sleep_1);

}
