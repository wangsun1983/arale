#include "test_task.h"
#include "test_utils.h"
#include "task.h"
#include "time.h"


int start_test_sleep_1()
{
    unsigned long long jiffies = get_jiffy();
    kprintf("start sleep 1\n");
    sleep(10000);
    kprintf("start sleep 2\n");
    if((jiffies - get_jiffy()) > 1000)
    {
        return  -1;
    }

    return 1;
}


int start_test_sleep()
{
    TEST_ASSERT(start_test_sleep_1);

}
