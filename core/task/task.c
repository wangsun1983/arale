#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"

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

    //cli();
    task_struct* task = task_alloc();
    task->context->eip = runnable;
    //sti();

    return task;
}

int task_start(task_struct *task)
{
    printf("task_start pid is %d\n",task->pid);
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
            printf("alloc task pid is %d,status is %d \n",task->pid,task->status);
            task->status = TASK_STATUS_INIT;
            break;
        }
    }

    //create memory struct
    //printf("alloc task trace1,size is %d,pid is %d \n",sizeof(mm_struct),task->pid);
    mm_struct *_mm = (mm_struct *)kmalloc(sizeof(mm_struct)); //struct need physical address
    _mm->pte_user = (addr_t *)fmalloc(sizeof(addr_t)*PD_ENTRY_CNT*PT_ENTRY_CNT*3/4);
    _mm->user_mem_map = (addr_t *)kmalloc(PD_ENTRY_CNT*PT_ENTRY_CNT*3/32);

        //_mm->pgd = core_mem.pgd;
    _mm->pte_core = core_mem.pte_core;
    _mm->core_mem_map =core_mem.core_mem_map;

    _mm->pgd = fmalloc(sizeof(addr_t) * PD_ENTRY_CNT);
    

    //core memory is always same~~~~~
    for (i = 0; i < memory_range_user.start_pgd; i++) {
        _mm->pgd[i] = (addr_t)&_mm->pte_core[i*PD_ENTRY_CNT];
        
    }

    int user_index = 0;
    for(i = memory_range_user.start_pgd;i<PD_ENTRY_CNT;i++,user_index++) {
       //physical memory should save physical memory
       addr_t va = &_mm->pte_user[user_index*PD_ENTRY_CNT];
       int pd = (va >>22) & 0x3FF; //max is 1024=>2^10
       int pt = (va >>12) & 0x3FF;
       //_mm->pgd[i] = (addr_t)&_mm->pte_user[i*PD_ENTRY_CNT]| ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
       _mm->pgd[i] = _mm->pte_core[pd *PD_ENTRY_CNT + pt];
       //if(_mm->pgd[i] != ((pd*1024*1024*4 + pt*1024*4)| ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR)) {
       //     printf("error,va is %x,new pgd[%d] is %x,value is  %x \n",va,i,_mm->pgd[i],(pd*1024*1024*4 + pt*1024*4));
       //}
       //printf("pd is %d,pt is %d",pd,pt);
    }

    task->mm = _mm;

    addr_t *_pte = (addr_t *)_mm->pte_user;
    char *_mem_ptr = _mm->user_mem_map;
    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT*3/4; i++) {
        _pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
        //_mem_ptr[i]= 0;
        set_bit(_mem_ptr,i,0);
    }

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
            switch_to(current->context,pp->context);
            load_pd(pp->mm->pgd);
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
        switch_to(current->context,pp->context);
        load_pd(pp->mm->pgd);
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
