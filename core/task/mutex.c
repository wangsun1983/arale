#include "mutex.h"
#include "list.h"

mutex *create_mutex()
{
    mutex *lock = (mutex *)kmalloc(sizeof(mutex));
    INIT_LIST_HEAD(lock->wait_list);
    return lock;
}

void acquire_mutex(mutex)
{
  
}
