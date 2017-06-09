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
    start_sysclock();

    task_struct*task = task_create(run);
    task_start(task);

#if 0
    printf("wangsl,start test \n");
    char *malloc_str = (char *)kmalloc(1024);
    printf("malloc_str is 0x%x\n",malloc_str);

    malloc_str[2] = 1;
    printf("valloc_str after is %d\n",&malloc_str[0]);
    printf("malloc_str after is %d\n",&malloc_str[2]);
    printf("malloc_str after is %d\n",malloc_str[2]);


    char *malloc_str3 = (char *)kmalloc(1024);
    printf("malloc_str3 is %x\n",malloc_str3);
    malloc_str3[2] = 8;
    printf("malloc_str3[2] is %d\n",&malloc_str3[2]);
    printf("malloc_str3[3] is %d\n",&malloc_str3[3]);
    printf("malloc_str3[2] is %d\n",malloc_str3[2]);
    printf("malloc_str[2] is %d\n",malloc_str[2]);
#endif

    printf("start...... complete \n");
    while(1){}
}


void run(void *args){
    printf("task1");

    //task_struct *current = (task_struct *)GET_CURRENT_TASK();
    char *malloc_str = (char *)kmalloc(1024*1024);
    printf("malloc_str is %x\n",malloc_str);

    malloc_str[1] = 5;
    printf("malloc_str[1] is %x\n",malloc_str[1]);
    int pt = va_to_pt_idx((addr_t)malloc_str);
    int pte = va_to_pte_idx((addr_t)malloc_str);
    printf("pt is %d,pte is %d \n",pt,pte);
    //printf("task1:mm pmm[%d][%d] is %x,pte_kern is %x,virtual addr is %x \n",
    //           pt,pte,current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&malloc_str[0]);



}
