#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"
#include "mmzone.h"
#include "vm_allocator.h"
#include "i8259.h"

//task_struct task_table[TASK_MAX];
void sys_clock_handler();
task_struct* task_alloc();

task_struct init_thread;

uint32_t task_id;

task_queue_group taskgroup;

//we should create a idle task for all the task 
//sleep.
void idle()
{
    while(1)
    {
        //Do nothing,just for idle.
    };
}

task_struct *idle_task;

void create_idle()
{
    idle_task = task_create(idle,NULL);
}

void move_to_waitq(task_struct *task)
{
    task->status = TASK_STATUS_WAIT;
    list_del(&task->rq_ll);
    list_add(&task->rq_ll,&taskgroup.waitq);
}

void move_to_runningq(task_struct *task)
{
    task->status = TASK_STATUS_RUNNING;
    list_del(&task->rq_ll);
    current_task = task;
}

void move_to_runnableq(task_struct *task)
{
    task->status = TASK_STATUS_RUNNABLE;
    list_del(&task->rq_ll);
    list_add(&task->rq_ll,&taskgroup.runnableq);
}

void move_to_sleepingq(task_struct *task)
{
    task->status = TASK_STATUS_SLEEPING;
    list_del(&task->rq_ll);
    list_add(&task->rq_ll,&taskgroup.sleepingq);
}

void clear_task_link()
{
     list_del(&idle_task->rq_ll);
}

void task_init(struct boot_info *binfo)
{
    //we use task group to manage all the task
    INIT_LIST_HEAD(&taskgroup.runningq);
    INIT_LIST_HEAD(&taskgroup.runnableq);
    INIT_LIST_HEAD(&taskgroup.sleepingq);
    INIT_LIST_HEAD(&taskgroup.waitq);

    task_id = 0;
    task_struct *init_task = &init_thread;

    init_task->pid = task_id;
    //current_pid = task_id;
    init_task->mm = get_root_mm();
    init_task->ticks = DEFAULT_TASK_TICKS;
    init_task->status = TASK_STATUS_RUNNING;
    task_id++;
    //kprintf("task_init current ticks is %d,DEFAULT is %x \n",current_task->ticks,DEFAULT_TASK_TICKS);
    //list_add(&current_task->rq_ll,&taskgroup.runningq);
    move_to_runningq(init_task);
    reg_sys_clock_handler(sys_clock_handler);

    create_idle();

}

void scheduler_for_exit()
{
    if(list_empty(&taskgroup.runnableq))
    {
        reset_ticks();
    }

    scheduler();
}

void do_exit(task_struct *task)
{
    //todo;
    //remove task
    kprintf("exit task pid is %d \n",task->pid);
    clear_task_link(task);
    task->ticks = 0;
    task->status = TASK_STATUS_DESTROY;
    cli();
    kprintf("exit trace1 \n");
    //scheduler();
    scheduler_for_exit();
    kprintf("exit trace2 \n");
    sti();
}

void _entry()
{
    current_task->_entry(current_task->_entry_data);
    do_exit(current_task);
}

task_struct* task_create(void *runnable,void *data)
{
    task_struct* task = task_alloc();
    //task->context->eip = (uint32_t)runnable;
    task->context->eip = (uint32_t)_entry;    
    task->_entry = runnable;
    task->_entry_data = data;

    return task;
}

void task_switch_idle(task_struct *current)
{
    irq_done(IRQ0_VECTOR);
    move_to_runningq(idle_task);
    load_pd((addr_t)idle_task->mm->pgd);
    switch_to(&current->context,idle_task->context);
}

void task_switch(task_struct *current,task_struct *next) 
{
    //kprintf("task_switch,next pid is %d \n",next->pid);
    irq_done(IRQ0_VECTOR);
    //sti();

    if(current_task->status != TASK_STATUS_SLEEPING 
       && current_task->status != TASK_STATUS_DESTROY)
    {
        move_to_waitq(current);
    } 
    else if(current_task == idle_task) 
    {
        clear_task_link(idle_task);
    }

    move_to_runningq(next);

    load_pd((addr_t)next->mm->pgd);

    switch_to(&current->context,next->context);
}

int task_start(task_struct *task)
{
    //kprintf("task_start pid is %d\n",task->pid);
    //task->status = TASK_STATUS_RUNNABLE;
    move_to_runnableq(task);
    return -1;
}


task_struct* task_alloc()
{
    addr_t i = 0;
    int index = 0;
    task_struct *task = (task_struct *)kmalloc(sizeof(task_struct));
    
    task->ticks = DEFAULT_TASK_TICKS;
    task->status = TASK_STATUS_INIT;
    task->pid = task_id;
    task_id++;

    //create memory struct
    mm_struct *_mm = (mm_struct *)kmalloc(sizeof(mm_struct)); //struct need physical address
    _mm->pte_user = (addr_t *)pmalloc(sizeof(addr_t)*PD_ENTRY_CNT*PT_ENTRY_CNT*3/4);
    _mm->pgd = (addr_t *)pmalloc(sizeof(addr_t) * PD_ENTRY_CNT);//??
    _mm->userroot = vm_allocator_init(1024*1024*1024,(uint32_t)1024*1024*1024*3); //user space is 1~3G
    _mm->vmroot = vm_allocator_init(zone_list[ZONE_HIGH].start_pa,1024*1024*1024*1 - zone_list[ZONE_HIGH].start_pa);
    //_mm->pgd = core_mem.pgd;
    _mm->pte_core = core_mem.pte_core;
    
    //core memory is always same~~~~~
    for (i = 0; i < memory_range_user.start_pgd; i++) {
        _mm->pgd[i] = core_mem.pgd[i];
    }
    
    //kprintf("task init end i is %d, start i is %d \n",i,memory_range_user.start_pgd);
    int user_index = 0;
    //because kmalloc is continus memory,so there is no need to 
    //compute pa again.
    for(i = memory_range_user.start_pgd; i < PD_ENTRY_CNT; i++) {
        _mm->pgd[i] = (addr_t)&_mm->pte_user[user_index*PD_ENTRY_CNT] | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
        user_index++;
    }
    //kprintf("task alloc 4 \n");
    addr_t *_pte = (addr_t *)_mm->pte_user;    
    for (i = 0; i < PD_ENTRY_CNT*PT_ENTRY_CNT*3/4; i++) {
         //goto_xy(20,20);
         //kprintf("task alloc i is %x \n",i);
        _pte[i] = (i << 12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR; // i是页表号
    }
    //kprintf("task alloc 5 \n");
    task->mm = _mm;
    task->context = (context_struct *)kmalloc(sizeof(context_struct));
    kmemset(task->context,0,sizeof(context_struct));

    return task;
}

task_struct* GET_CURRENT_TASK()
{
    //return &task_table[current_pid];
    /*struct list_head *p;
    list_for_each(p,&taskgroup.runningq) {
        task_struct *task = list_entry(p,task_struct,rq_ll);
        //goto_xy(15,15);
        //kprintf("get current task ticks is %d,pid is %d \n",task->ticks,task->pid);
        return task;
    }
    
    return NULL;*/
    return current_task;
}
int s = 0;

void reset_ticks()
{
    struct list_head *p = taskgroup.waitq.next;
    while(p != &taskgroup.waitq && p!= NULL) 
    {
        task_struct *task = list_entry(p,task_struct,rq_ll);
        task->ticks = DEFAULT_TASK_TICKS;
        p = task->rq_ll.next;
        move_to_runnableq(task);
    }
}

void scheduler()
{
    if(current_task == NULL) 
    {
        return;
    }
    
    if(current_task->ticks > 0) 
    {
        current_task->ticks--;
        return;
    }

    if(!list_empty(&taskgroup.runnableq)) 
    {
        //kprintf("sched,trace2 \n");
        struct list_head *p;
        list_for_each(p,&taskgroup.runnableq) {
            task_struct *pp = list_entry(p,task_struct,rq_ll);
            kprintf("sched,pp pid is %d current pid is %d,trace2 \n",pp->pid,current_task->pid);
            task_switch(current_task,pp);
            return;
        }
    } 
    else if(list_empty(&taskgroup.waitq)) 
    {
        //we should check whtether the process is in sleep queue
        if(current_task->status == TASK_STATUS_SLEEPING && current_task != idle_task) 
        {
            task_switch_idle(current_task);
            return;
        }
        
        //there is one task is running;
        current_task->ticks = DEFAULT_TASK_TICKS;
        return;
    } 
    
    reset_ticks();
    //scheduler();
}

void sys_clock_handler()
{
    scheduler();
}

void wake_up_task(task_struct *task)
{
    move_to_runnableq(task);
    task->ticks = task->remainder_ticks;
    kprintf("wake_up,task->ticks is %x \n",task->ticks);
    task->remainder_ticks = 0;
    cli();
    scheduler();
    sti();
}

void dormant_task(task_struct *task)
{
    kprintf("dormant task pid is %d \n",task->pid);

    move_to_sleepingq(task);
    task->remainder_ticks = task->ticks;
    task->ticks = 0;
    cli();
    scheduler();
    sti();
}

