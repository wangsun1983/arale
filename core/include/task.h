#include "vmm.h"
#include "mm.h"
#include "list.h"
#include "sched_deuce.h"

#ifndef _TASK_STRUCT_H_
#define _TASK_STRUCT_H_

#define TASK_GDT0 3 //because the first 2 GDT is defined in stage2.asm
#define TASK_MAX 2048

#define PN_MAX_LEN 16

#define TASK_MAX_PITS 100

#define DEFAULT_TASK_TICKS 5000

#define THREAD_STACK_SIZE 1024*32

enum task_type {
    TASK_TYPE_INDEPENDENT = 0,
    TASK_TYPE_DEPENDENT,
    TASK_TYPE_MAX
};

enum task_status_type {
    TASK_STATUS_NONE = 0,
    TASK_STATUS_INIT,
    TASK_STATUS_RUNNABLE,
    TASK_STATUS_RUNNING,
    TASK_STATUS_WAIT, //reset ticks
    TASK_STATUS_SLEEPING,
    TASK_STATUS_DESTROY,
    TASK_STATUS_MAX
};

typedef struct _tss_struct_ {
    uint32_t ts_link;    // Old ts selector
    uint32_t ts_esp0;    // Stack pointers and segment selectors
    uint16_t ts_ss0;    //   after an increase in privilege level
    uint16_t ts_padding1;
    uint32_t ts_esp1;
    uint16_t ts_ss1;
    uint16_t ts_padding2;
    uint32_t ts_esp2;
    uint16_t ts_ss2;
    uint16_t ts_padding3;
    uint32_t ts_cr3;    // Page directory base
    uint32_t ts_eip;    // Saved state from last task switch
    uint32_t ts_eflags;
    uint32_t ts_eax;    // More saved state (registers)
    uint32_t ts_ecx;
    uint32_t ts_edx;
    uint32_t ts_ebx;
    uint32_t ts_esp;
    uint32_t ts_ebp;
    uint32_t ts_esi;
    uint32_t ts_edi;
    uint16_t ts_es;        // Even more saved state (segment selectors)
    uint16_t ts_padding4;
    uint16_t ts_cs;
    uint16_t ts_padding5;
    uint16_t ts_ss;
    uint16_t ts_padding6;
    uint16_t ts_ds;
    uint16_t ts_padding7;
    uint16_t ts_fs;
    uint16_t ts_padding8;
    uint16_t ts_gs;
    uint16_t ts_padding9;
    uint16_t ts_ldt;
    uint16_t ts_padding10;
    uint16_t ts_t;        // Trap on task switch
    uint16_t ts_iomb;    // I/O map base address
}tss_struct;

typedef struct context{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
}context_struct;

typedef void (*task_entry_fun)(void *);

typedef struct _task_struct_ {

    //unique process id
    uint32_t pid;

    //dependent_task & independent_task management
    uint16_t type;
    struct list_head task_pool_ll;

    //memory management
    mm_struct *mm;

    //task's stack,now it's size is 64K
    context_struct *context;

    //task's stack address.when switch task,esp will be
    //saved in context,so when context is always changed.
    //we need a member to mark the context address.
    addr_t stack_addr;
    //task's status
    int16_t status;

    //we want to destroy task,when all the operation complete.
    //so,we use #task_entry@task.c to decorate all the runnable.
    //when runnable finished,task_entry will excute #do_exit@task.c.
    task_entry_fun _entry;
    void * _entry_data;

    struct list_head lock_ll;

    //every sched strategy need its own data to statics some information
    sched_reference sched_ref;

    //nouse
    char name[PN_MAX_LEN];
    struct _task_struct_ *parent;
}task_struct;

typedef struct _task_module_opertion {
    void (*revert_task)(task_struct *task);
    void (*reclaim)();
    task_struct * (*create)();
}task_module_op;

//task_struct task_table[TASK_MAX];
public task_struct *GET_CURRENT_TASK();

public void task_init(struct boot_info *binfo);

public task_struct *task_create(task_entry_fun runnable,void *data,int type);

public void task_start(task_struct *task);

public void task_sleep(task_struct *task);

public void task_wake_up(task_struct *task);

public void scheduler();

public void update_current_task(task_struct *task);

uint32_t task_id;

#endif
