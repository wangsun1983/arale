#ifndef __MUTEX_H__
#define __MUTEX_H__
#include "list.h"
#include "atomic.h"

public enum MUTEX_STATUS {
    MUTEX_IDLE = 0,
    MUTEX_HELD
};

public typedef struct mutex{
    uint8_t status;
    spinlock_t spin_lock;
    struct list_head wait_list;
    uint32_t held_pid;
}mutex;

public mutex *create_mutex();
public void acquire_lock(mutex *lock);
public void release_lock(mutex *lock);
public void free_lock(mutex *lock);

#endif
