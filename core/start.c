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
    init_gdt();

    if (screen_init())
    {

    }

    if (x86_init())
    {
        //TODO exception
    }

    struct boot_info * binfo = &bootinfo;
    
    mm_init(binfo);
#if 0
    char *malloc_str = (char *)malloc(1024);
    printf("malloc_str is 0x%x\n",malloc_str);
    malloc_str[2] = 1;
    printf("valloc_str after is %d\n",&malloc_str[0]);
    printf("malloc_str after is %d\n",&malloc_str[2]);
    printf("malloc_str after is %d\n",malloc_str[2]);


    //char *malloc_str3 = (char *)malloc(1024);
    //printf("malloc_str3 is %x\n",malloc_str3);
    //malloc_str3[2] = 8;
    //printf("malloc_str3[2] is %d\n",&malloc_str3[2]);
    //printf("malloc_str3[3] is %d\n",&malloc_str3[3]);
    //printf("malloc_str3[2] is %d\n",malloc_str3[2]);
    //printf("malloc_str[2] is %d\n",malloc_str[2]);

    //init display modules
    //we use shell to debug
    //init_font();
    //init_graphic();
    //start_refresh();
#endif

    init_sysclock();
    task_init(binfo);

    task_struct *task = task_create(run);
    //task_start(task);
    //start_sysclock();

    //printf("hello \n");
    //char *p = (char *)malloc(1024*1024 + 32);
    //printf("2222p is %x \n",p);

    

    while(1){}
}


void run(void *args){
    printf("task1");

    char *p = (char *)malloc(1024);
    printf("p is %x \n",p);
}
