#include "dependent_task.h"
#include "task.h"

task_struct *create_dependent_task()
{
    task_struct *task = (task_struct *)kmalloc(sizeof(task_struct));
    task->mm = GET_CURRENT_TASK()->mm;
    task->context = (context_struct *)kmalloc(sizeof(context_struct));
    kmemset(task->context,0,sizeof(context_struct));

    return task;
}
