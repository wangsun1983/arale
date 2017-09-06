#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include "klibc.h"

#define TEST_ASSERT(x)                            \
({ \
    if(x() < 0){\
        kprintf("%s fail \n",#x);\
        return -1;\
    }\
})\



#endif
