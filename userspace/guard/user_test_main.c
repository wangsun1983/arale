#include "common.h"
#include "user_test.h"

int start_user_test()
{
    USER_TEST_ASSERT(start_test_ipcthread);
    //TEST_ASSERT(start_test_syscall);
    return 1;
}
