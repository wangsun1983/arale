#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "list.h"
#include "atomic.h"
#include "const.h"

typedef struct semaphore {
    struct list_head wait_list;
    uint32_t count;
    spinlock_t lock;
}semaphore;

public semaphore *sem_create();
public void sem_down(semaphore *semaphore);
public void sem_up(semaphore *sem);
public void sem_up_all(semaphore *semaphore);
public void sem_destroy(semaphore *semaphore);

#endif
