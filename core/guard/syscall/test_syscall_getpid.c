#include "test_syscall.h"
#include "core_sys_call.h"
#include "test_utils.h"
#include "log.h"

int test_syscall_getpid()
{
    uint32_t result = test_sys_call(CORE_SYS_CALL_GETPID,1,2,3,4,5);
    //LOGD("test result is %d \n",result);
    LOGD("test_syscall_getpid,result is %d \n",result);
}

int start_test_syscall_getpid()
{
    TEST_ASSERT(test_syscall_getpid);
}
