#ifndef __MUTEX_H__
#define __MUTEX_H__
#include "list.h"

typedef struct mutex{
    int count;
    struct list_head wait_list;
}mutex;


#endif
