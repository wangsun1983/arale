#include "test_main.h"
#include "test_mm.h"
#include "test_task.h"
#include "test_libc.h"
#include "test_utils.h"
#include "test_syscall.h"
#include "log.h"

int start_test()
{
    LOGD("guard test start\n");
    //TEST_ASSERT(start_test_mm);
    //TEST_ASSERT(start_test_task);
    TEST_ASSERT(start_test_libc);
    //TEST_ASSERT(start_test_syscall);

    LOGD("guard test complete\n");
    return 1;
}
