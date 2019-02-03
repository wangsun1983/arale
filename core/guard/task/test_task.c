#include "test_task.h"
#include "test_utils.h"
#include "log.h"

int start_test_task()
{
    //TEST_ASSERT(start_test_stress);
    TEST_ASSERT(start_test_sleep);
    //LOGD("start test task \n");
    TEST_ASSERT(start_test_dependent);
    //LOGD("finish test task \n");
    TEST_ASSERT(start_test_independent);
    TEST_ASSERT(start_test_mutex);
    TEST_ASSERT(start_test_semaphore);
}
