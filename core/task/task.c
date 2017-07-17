#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"
#include "mmzone.h"

task_struct task_table[TASK_MAX];
void sys_clock_handler();
task_struct* task_alloc();

/*
*
*    Arale only supports 2048 process.haha
*/
void task_init(struct boot_info *binfo)
{
    memset(task_table,0,sizeof(task_struct)*TASK_MAX);
    task_struct *current_task = &task_table[0];
    current_task->pid = 0;
    current_pid = 0;
    current_task->mm = get_root_pd();
    current_task->ticks = DEFAULT_TASK_TICKS;
    current_task->status = TASK_STATUS_RUNNING;
    //printf("current_task is %x,ticks is %d",current_task,current_task->ticks);
    reg_sys_clock_handler(sys_clock_handler);
}

task_struct* task_create(void *runnable)
{

    task_struct* task = task_alloc();
    task->context->eip = runnable;

    return task;
}

void switch_ref(task_struct *task) 
{
    switch_to(task->context,task->context);
}

void task_switch(task_struct *current,task_struct *next) 
{
    addr_t va = next->mm->pgd;
    int pd = (va >>22) & 0x3FF; //max is 1024=>2^10
    int pt = (va >>12) & 0x3FF;
    
    //addr_t pa = next->mm->pte_core[pd*PD_ENTRY_CNT + pt];
    //printf("task_switch before pa is %x",pa);
    //pa = (pa >>12)<<12;
    //printf("task_switch after pa is %x",pa);
    printf("next->mm->pgd is %x core_mem.pgd is %x \n",next->mm->pgd,core_mem.pgd);
    load_pd((addr_t)next->mm->pgd);
    switch_to(current->context,next->context);
}

int task_start(task_struct *task)
{
    //printf("task_start pid is %d\n",task->pid);
    task->status = TASK_STATUS_RUNNABLE;
    return -1;
}


task_struct* task_alloc()
{
    addr_t i = 0;
    int index = 0;
    //task_struct *task = (task_struct *)malloc(sizeof(task_struct));

    task_struct *task = NULL;
    for(;index < TASK_MAX;index++)
    {
        task = &task_table[index];

        if(task->status == TASK_STATUS_NONE) {
            task->pid = index;
            task->status = TASK_STATUS_INIT;
            break;
        }
    }

    //create memory struct
    //printf("alloc task trace1,size is %d,pid is %d \n",sizeof(mm_struct),task->pid);
    mm_struct *_mm = (mm_struct *)kmalloc(sizeof(mm_struct)); //struct need physical address
    _mm->pte_user = (addr_t *)pmalloc(_mm,sizeof(addr_t)*PD_ENTRY_CNT*PT_ENTRY_CNT*3/4);
    _mm->pgd = pmalloc(_mm,sizeof(addr_t) * PD_ENTRY_CNT);
    _mm->userroot = vm_allocator_init(1024*1024*1024,1024*1024*1024*3); //user space is 1~3G
    _mm->vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);

    //_mm->pgd = core_mem.pgd;
    _mm->pte_core = core_mem.pte_core;
    
    //core memory is always same~~~~~
    for (i = 0; i < memory_range_user.start_pgd; i++) {
        _mm->pgd[i] = core_mem.pgd[i];
    }

    printf("task init end i is %d, start i is %d \n",i,memory_range_user.start_pgd);
    int user_index = 0;
    //because kmalloc is continus memory,so there is no need to 
    //compute pa again.
    for(i = memory_range_user.start_pgd; i < PD_ENTRY_CNT; i++) {
        _mm->pgd[i] = (addr_t)&_mm->pte_user[user_index*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        user_index++;
    }

    addr_t *_pte = (addr_t *)_mm->pte_user;    
    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT*3/4; i++) {
        _pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
    }

    task->mm = _mm;
    task->context = (context_struct *)kmalloc(sizeof(context_struct));
    memset(task->context,0,sizeof(context_struct));

    return task;
}

task_struct* GET_CURRENT_TASK()
{
    return &task_table[current_pid];
}


void sys_clock_handler()
{
    //printf("wahahahah sys_clock_handler \n");
    scheduler();
}


void scheduler(){
    task_struct *pp;
    task_struct *current = (task_struct *)GET_CURRENT_TASK();

    //printf("sched,%d,%d",current->pid,current->ticks);
    int index = 0;
    if(current->ticks > 0) {
        //printf("current ticks is %d",current->ticks);
        current->ticks--;
        return;
    }

    //printf("sched trace2 \n");
    for(;index < TASK_MAX;index++)
    {
        pp = &task_table[index];

        if(pp != NULL && pp->ticks > 0 && pp->status == TASK_STATUS_RUNNABLE) {
            current_pid = index;
            //printf("wangsl1,switch to pid is %d ticks is %d \n",pp->pid,pp->ticks);
            current->status = TASK_STATUS_RUNNABLE;
            pp->status = TASK_STATUS_RUNNING;
            printf("task schedule , pp->mm->pgd is %x \n",pp->mm->pgd);
            task_switch(current,pp);
            return;
        }
    }

    //all the task finish,we reset the time

    reset_ticks();

    if(current->pid != 0) {
        pp = &task_table[0];
        current_pid = 0;
        //printf("wangsl2,switch to pid is %d ticks is %d \n",pp->pid,pp->ticks);
        current->status = TASK_STATUS_RUNNABLE;
        pp->status = TASK_STATUS_RUNNING;
        //switch_to(current->context,pp->context);
        //load_pd(pp->mm->pgd);
        task_switch(current,pp);
    }

}

void reset_ticks()
{
    int i = TASK_MAX;
    task_struct *pp;

    for(;i >=0;i--)
    {
        pp = &task_table[i];

        if(pp != NULL) {
            pp-> ticks = DEFAULT_TASK_TICKS;
        }
    }

}
