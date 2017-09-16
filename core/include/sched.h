#ifndef __SCHED_H__
#define __SCHED_H__

#include "ctype.h"

enum SCHED_TYPE {
    SCHED_TYPE_FORCE = 0,
    SCHED_TYPE_CLOCK
};

//public interface
public void sched_scheduler(int type);

public void sched_init();

public void sched_start_task(void *task);

public void sched_finish_task(void *task);

public void sched_remove_task(void *task);

public void sched_add_task(void *task);

public void sched_init_data(void *task);

public void sched_sleep(void *task);

public void sched_wake_up(void *task);

#endif
