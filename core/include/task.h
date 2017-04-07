#include "vmm.h"

#ifndef _TASK_STRUCT_H_
#define _TASK_STRUCT_H_

struct task_struct{
    int pid;
    struct mm_area_struct *mm;

    int prio;
    int state;
};

#endif
