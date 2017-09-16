#include "test_main.h"
#include "test_mm.h"
#include "test_task.h"
#include "test_utils.h"

int start_test()
{
    kprintf("guard test start\n");
    //TEST_ASSERT(start_test_mm);
    TEST_ASSERT(start_test_task);

    kprintf("guard test complete\n");
    return 1;
}
