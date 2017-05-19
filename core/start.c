#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
//#include "string.h"
#include "global.h"
#include "color.h"
#include "font.h"
#include "mm.h"
#include "vmm.h"
#include "cpu.h"
#include "task.h"
#include "gdt.h"

//extern void write_mem8(int addr,int data);
extern void init_font();
extern void init_graphic();
extern void start_refresh();
void run(void *args);

//struct boot_info *binfo;
static int screen_init()
{
    set_color(VID_CLR_BLACK, VID_CLR_LIGHT_BLUE);
    clear_screen();
    goto_xy(0, 0);
    
    return 0;
}


void start_core(struct boot_info bootinfo)
{
    //use new gdt.
    

    if (screen_init())
    {

    }

    init_gdt();
    
    if (x86_init())
    {
        //TODO exception
    }

    struct boot_info * binfo = &bootinfo;
    
    mm_init(binfo);
    init_sysclock();
    task_init(binfo);
    //task_struct *task = task_create(run);

#if 0
    

    task_struct *task = task_create(run);
    //task_start(task);
    //start_sysclock();

    //printf("hello \n");
    //char *p = (char *)malloc(1024*1024 + 32);
    //printf("2222p is %x \n",p);
#endif
    
    printf("start...... complete \n");
    while(1){}
}


void run(void *args){
    printf("task1");

    task_struct *current = (task_struct *)GET_CURRENT_TASK();
    char *malloc_str = (char *)malloc(1024*1024);
    printf("malloc_str is %x\n",malloc_str);
    int pt = va_to_pt_idx((addr_t)malloc_str);
    int pte = va_to_pte_idx((addr_t)malloc_str);
    printf("task1:mm pmm[%d][%d] is %x,pte_kern is %x,virtual addr is %x \n",
               pt,pte,current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&malloc_str[0]);

}
