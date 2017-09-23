#include "test_mm.h"
#include "mm.h"
#include "mmzone.h"
#include "test_utils.h"

int test_mm_free()
{
    uint32_t mfree = pmm_free_mem_statistic();
    char *p = (char *)kmalloc(123);
    uint32_t new_free = pmm_free_mem_statistic();
    kprintf("test_mm_getfree free is %x \n",mfree);
    kprintf("test_mm_getfree new_free is %x \n",new_free);

    return 1;
}

int test_mm_statistic()
{
    TEST_ASSERT(test_mm_free);
}
