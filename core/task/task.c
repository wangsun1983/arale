#include "task.h"
#include "mm.h"
#include "klibc.h"
#include "vmm.h"

/*
*
*    Arale only supports 2048 process~~~.haha
*/
void task_init(struct boot_info *binfo)
{
    //struct segment_descriptor *gdt = (struct segment_descriptor *) binfo->gdt_addr;

    memset(task_table,0,sizeof(struct task_struct)*TASK_MAX);

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

void task_set_root()
{
    current_task = 0;
    struct task_struct *task = GET_CURRENT_TASK();
    task->pid = current_task;
    memset(&task->mm,0,sizeof(struct mm_area_struct));
    task->mm.pd = get_root_pd();
}

struct task_struct* creat_ktask()
{
    //struct task_struct *task = (struct task_struct *)kmalloc()


}

struct task_struct* GET_CURRENT_TASK()
{
    return &task_table[current_task];
}
