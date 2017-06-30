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
int show_bt = 0;

//struct boot_info *binfo;
static int screen_init()
{
    set_color(VID_CLR_BLACK, VID_CLR_LIGHT_BLUE);
    clear_screen();
    goto_xy(0, 0);

    return 0;
}

void doTest() 
{
    printf("doTest is %d",doTest);
    task_struct *current = (task_struct *)GET_CURRENT_TASK();

    char *p = (char *)malloc(1024*1024);
    p[12] = 8;
    printf("fast malloc2 p[12] is %x",p[12]);

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

#ifdef TASK_TEST
    task_struct*task = task_create(run);

    task_start(task);
#endif

    

    //doTest();
    //changeTaskMm(task);


    printf("wangsl,start test \n");
    char *malloc_str = (char *)kmalloc(1024);
    printf("malloc_str is 0x%x\n",malloc_str);

    malloc_str[2] = 1;
    printf("valloc_str after is %d\n",&malloc_str[0]);
    printf("malloc_str after is %d\n",&malloc_str[2]);
    printf("malloc_str after is %d\n",malloc_str[2]);

#if 0
    char *malloc_str3 = (char *)kmalloc(1024);
    printf("malloc_str3 is %x\n",malloc_str3);
    malloc_str3[2] = 8;
    printf("malloc_str3[2] is %d\n",&malloc_str3[2]);
    printf("malloc_str3[3] is %d\n",&malloc_str3[3]);
    printf("malloc_str3[2] is %d\n",malloc_str3[2]);
    printf("malloc_str[2] is %d\n",malloc_str[2]);


    printf("wangsl,start test alloc \n");
    char *malloc_str = (char *)malloc(16);
    printf("malloc_str is 0x%x\n",malloc_str);
    malloc_str[2] = 8;
    printf("malloc_str[2] is %d\n",malloc_str[2]);

    char *malloc_str2 = (char *)malloc(1024*4);

    task_struct *current = (task_struct *)GET_CURRENT_TASK();
    printf("current->mm is %x",current->mm);
#endif

/*    printf("malloc_str2 is 0x%x\n",malloc_str2);
    malloc_str2[3] = 8;
    printf("trace 1:malloc_str[2] is %d\n",malloc_str2[2]);
    printf("trace 1:malloc_str[2] is %d\n",malloc_str2[3]);

    printf("core_mem.pgd[0] %x \n",core_mem.pgd[0]);
    printf("core_mem.pte_core %x \n",(addr_t)&core_mem.pte_core[0]);
*/

    printf("start...... complete \n");
    while(1){}
}


void run(void *args){
    printf("task1 \n");


    //task_struct *current = (task_struct *)GET_CURRENT_TASK();
    char *malloc_str = (char *)malloc(1024*1024);
    printf("malloc_str is %x\n",malloc_str);

    malloc_str[300] = 15;
    printf("malloc_str[1] is %x\n",malloc_str[300]);
    task_struct *current = (task_struct *)GET_CURRENT_TASK();
    printf("current->mm is %x",current->mm);

    int pt = va_to_pt_idx((addr_t)malloc_str);
    int pte = va_to_pte_idx((addr_t)malloc_str);
    printf("pt is %d,pte is %d \n",pt,pte);

    //printf("task1:mm pmm[%d][%d] is %x,pte_kern is %x,virtual addr is %x \n",
    //           pt,pte,current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&malloc_str[0]);

}
