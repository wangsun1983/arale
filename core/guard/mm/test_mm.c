#include "test_mm.h"
#include "mm.h"
#include "test_utils.h"

int start_test_mm()
{
    test_kmalloc_4();
    //TEST_ASSERT(test_kmalloc_1);
    //TEST_ASSERT(test_kmalloc_2);
    //TEST_ASSERT(test_kmalloc_3);

    //TEST_ASSERT(test_vmalloc_1);
    //TEST_ASSERT(test_vmalloc_2);
    //TEST_ASSERT(test_vmalloc_3);

    //TEST_ASSERT(test_pmalloc_1);
    //TEST_ASSERT(test_pmalloc_2);
    //TEST_ASSERT(test_pmalloc_3);

    return 1;
}
