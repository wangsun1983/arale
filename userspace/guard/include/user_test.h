#ifndef _USER_TEST_H__
#define _USER_TEST_H__

#include "common.h"

#define USER_TEST_ASSERT(x)                            \
({ \
    if(x() < 0){\
        return -1;\
    }\
})\

int start_test_ipcthread();

#endif
