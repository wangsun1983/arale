#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include "common.h"

int pthread_create(addr_t runnable,void *data);
int pthread_destroy(int tid);
int pthread_join(int tid);

#endif
