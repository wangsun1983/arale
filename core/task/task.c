#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"

task_struct task_table[TASK_MAX]; 
void sys_clock_handler();

/*
*
*    Arale only supports 2048 process.haha
*/
void task_init(struct boot_info *binfo)
{
    current_task->pid = 0;
    current_pid = 0;
    current_task->mm = get_root_pd();
    reg_sys_clock_handler(sys_clock_handler);

    memset(task_table,0,sizeof(task_struct)*TASK_MAX);

    //we should set all the
    //we can pre init all the ldt for the task
    //for (int i = 0; i < TASK_MAX; i++) {
    //    task_table[i].flags = 0;
	  //    task_table[i].sel = (TASK_GDT0 + i) * 8;
	  //    task_table[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
	  //    set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) task_table[i].tss, AR_TSS32);
	  //    set_segmdesc(gdt + TASK_GDT0 + TASK_MAX + i, 15, (int) task_table[i].ldt, AR_LDT);
    //}
}


task_struct* task_alloc()
{
    task_struct *task = (task_struct *)kmalloc(sizeof(task_struct));
    //TODO

}

task_struct* GET_CURRENT_TASK()
{
    return &task_table[current_pid];
}


void sys_clock_handler()
{
    task_struct *task = (task_struct *)GET_CURRENT_TASK();
    //TODO




}
