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
#include "cache_allocator.h"
#include "time.h"

//extern void write_mem8(int addr,int data);
extern void init_font();
extern void init_graphic();
extern void start_refresh();
extern void hdd_init();

void run(void *args);
int show_bt = 0;

void testAllMalloc();

//struct boot_info *binfo;
static int screen_init()
{
    set_color(VID_CLR_BLACK, VID_CLR_LIGHT_BLUE);
    clear_screen();
    goto_xy(0, 0);

    return 0;
}

//int mm = 0;

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

    //kprintf("start..... eip is %x \n",start_core);

    hdd_init();
//kprintf("wangsl,start trace1 \n");
    init_sysclock();
//kprintf("wangsl,start trace2 \n");
    task_init(binfo);
//kprintf("wangsl,start trace3 \n");
    start_sysclock();
//kprintf("wangsl,start trace4 \n");
//#ifdef TASK_TEST
    task_struct*task = task_create(run);
//kprintf("wangsl,start trace5 \n");

    task_start(task);
//kprintf("wangsl,start trace6 \n");
//#endif
    //doTest();
    //changeTaskMm(task);

    init_timer();
    //kprintf("wangsl,start test1 \n");
    sleep(9000);
    //kprintf("wangsl,start test2 \n");
    //testAllMalloc();
    //char *p = kmalloc(15);
    //kprintf("p is %x \n",p);
    //p[2] = 12;
    //kprintf("p[2] is %d \n",p[2]);
    //core_mem_cache *cache = creat_core_mem_cache(sizeof(struct test));
    //struct test *t1 = cache_alloc(cache);
    //struct test *t2 = cache_alloc(cache);
    //t1->i = 1;
    //kprintf("t1 is %x,t2 is %X \n",t1,t2);

    //char *malloc_str = (char *)malloc(1024*1024*3);
    //char *malloc_str = (char *)vmalloc(1024);
    //malloc_str[1024*1024 + 32] = 89;
    //malloc_str[2] = 8;
    //kprintf("malloc_str2 is 0x%x\n",malloc_str);
    //kprintf("malloc_str[1] is %d\n",malloc_str[1024*1024 + 32]);
    //kprintf("malloc_str[2] is %d\n",malloc_str[2]);


    //free(malloc_str);
    //kprintf("wangsl,start trace1 \n");
    //malloc_str =  (char *)malloc(1024);
    //malloc_str[5] = 12;
    //kprintf("wangsl,malloc_str[5] is %d \n",malloc_str[5]);
    //kprintf("wangsl,start trace2 \n");

    //task_struct *current = (task_struct *)GET_CURRENT_TASK();
    //int pt = va_to_pt_idx((addr_t)malloc_str);
    //int pte = va_to_pte_idx((addr_t)malloc_str);
    //kprintf("malloc pmm is %x \n",current->mm->pte_core[pt*PD_ENTRY_CNT + pte+1]);
    //kprintf("malloc pmm is %x \n",current->mm->pte_core[pt*PD_ENTRY_CNT + pte]);
    //kprintf("malloc pmm is %x \n",current->mm->pte_core[pt*PD_ENTRY_CNT + pte-1]);


#if 0
    char *malloc_str3 = (char *)kmalloc(1024);
    kprintf("malloc_str3 is %x\n",malloc_str3);
    malloc_str3[2] = 8;
    kprintf("malloc_str3[2] is %d\n",&malloc_str3[2]);
    kprintf("malloc_str3[3] is %d\n",&malloc_str3[3]);
    kprintf("malloc_str3[2] is %d\n",malloc_str3[2]);
    kprintf("malloc_str[2] is %d\n",malloc_str[2]);


    kprintf("wangsl,start test alloc \n");
    char *malloc_str = (char *)malloc(16);
    kprintf("malloc_str is 0x%x\n",malloc_str);
    malloc_str[2] = 8;
    kprintf("malloc_str[2] is %d\n",malloc_str[2]);

    char *malloc_str2 = (char *)malloc(1024*4);

    task_struct *current = (task_struct *)GET_CURRENT_TASK();
    kprintf("current->mm is %x",current->mm);
#endif

/*    kprintf("malloc_str2 is 0x%x\n",malloc_str2);
    malloc_str2[3] = 8;
    kprintf("trace 1:malloc_str[2] is %d\n",malloc_str2[2]);
    kprintf("trace 1:malloc_str[2] is %d\n",malloc_str2[3]);

    kprintf("core_mem.pgd[0] %x \n",core_mem.pgd[0]);
    kprintf("core_mem.pte_core %x \n",(addr_t)&core_mem.pte_core[0]);
*/
    kprintf("start...... complete \n");
    int index = 0;
    while(1)
    {
       //if(mm != 0) {
           goto_xy(20,20);
           kprintf("start...... 123123.index is %x \n",index);
           index++;
       //}
    }
}

task_struct task_table[TASK_MAX];

void run(void *args){
    kprintf("task1 \n");
    //task_struct* maintask = &task_table[0];
    //kprintf("main task eip is %x \n",maintask->context->eip);
    //task_struct *current = (task_struct *)GET_CURRENT_TASK();
    //char *malloc_str = (char *)malloc(1024);
    //kprintf("malloc_str is %x\n",malloc_str);

    //malloc_str[300] = 15;
    //kprintf("malloc_str[1] is %x\n",malloc_str[300]);
    //task_struct *current = (task_struct *)GET_CURRENT_TASK();
    //kprintf("current->mm is %x",current->mm);

    //int pt = va_to_pt_idx((addr_t)malloc_str);
    //int pte = va_to_pte_idx((addr_t)malloc_str);
    //kprintf("pt is %d,pte is %d \n",pt,pte);
    //mm = 1;
    int index = 0;
    kprintf("wangsl,task trace1 \n");
    //while(1){
    //     goto_xy(10,10);
    //     kprintf("task1...... 66666 index is %x\n",index++);
         //index++;
    //}
    //kprintf("task1:mm pmm[%d][%d] is %x,pte_kern is %x,virtual addr is %x \n",
    //           pt,pte,current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&current->mm->pte_kern[pt][pte],&malloc_str[0]);

}


void testAllMalloc()
{
    kprintf("test all malloc ->kmalloc(cache) \n");
    //test kmalloc (cache)
    char *p = kmalloc(18);
    p[2] = 2;
    if(p[2] != 2)
    {
        kprintf("kmalloc cache fail \n");
    }

    //test free
    free(p);

    kprintf("test all malloc ->kmalloc \n");
    p = kmalloc(1024*1024*1);
    p[1024*9] = 2;
    if(p[1024*9] != 2)
    {
        kprintf("kmalloc fail \n");
    }
    
    free(p);
    
    kprintf("test all malloc ->vmalloc \n");
    p = vmalloc(1024*1024*1);
    p[1024*9] = 2;
    if(p[1024*9] != 2)
    {
        kprintf("vmalloc fail \n");
    }
    
    free(p);

    kprintf("test all malloc end\n");

}
