#include "test_libc.h"
#include "atomic.h"
#include "test_utils.h"


int test_spin_lock_1()
{
    spinlock_t lock;
    SPIN_LOCK_INIT(&lock);

    spin_lock(&lock);
    //LOGD("lock is %d \n",lock.locked);
    if(lock.locked != 1)
    {
        return -1;
    }
    spin_unlock(&lock);
    //LOGD("lock again is %d \n",lock.locked);
    if(lock.locked != 0)
    {
        return -1;
    }
}

int start_test_spin_lock()
{
    LOGD("start_test_spin_lock \n");
    TEST_ASSERT(test_spin_lock_1);
}
