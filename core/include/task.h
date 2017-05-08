#include "vmm.h"
#include "mm.h"

#ifndef _TASK_STRUCT_H_
#define _TASK_STRUCT_H_

#define TASK_GDT0 3 //because the first 2 GDT is defined in stage2.asm
#define TASK_MAX 2048

#define PN_MAX_LEN 16

int current_task; //the default pid is 0

//#define GET_CURRENT_TASK() \
//{\
//    return &task_table[current_task]; \
//}
typedef struct _tss_struct_ {
    int backlink;
    int esp0;
    int ss0;
    int esp1;
    int ss1;
    int esp2;
    int ss2;
    int cr3;
    int eip;
    int eflags;
    int eax;
    int ecx;
    int edx;
    int ebx;
    int esp;
    int ebp;
    int esi;
    int edi;
    int es;
    int cs;
    int ss;
    int ds;
    int fs;
    int gs;
    int ldtr;
    int iomap;
}tss_struct;

typedef struct _task_struct_ {
    uint32_t pid;

    mm_struct *mm;
    tss_struct *tss;

    uint32_t prio;
    int16_t state;
    char name[PN_MAX_LEN];

    struct _task_struct_ *parent;

}task_struct;

task_struct task_table[TASK_MAX];

task_struct *GET_CURRENT_TASK();
void task_set_root();
void task_init(struct boot_info *binfo);

void task_alloc();


#endif
