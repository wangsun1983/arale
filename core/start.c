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

//extern void write_mem8(int addr,int data);
extern void init_font();
extern void init_graphic();
extern void start_refresh();

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
    //struct boot_info * binfo = &bootinfo;
    //int krnl_loc = binfo->krnl_loc;

    /* set segment values */
    //__asm__ __volatile__("movw $0x10, %%ax \n"
    //                     "movw %%ax, %%ds \n"
    //                     "movw %%ax, %%es \n"
    //                     "movw %%ax, %%fs \n"
    //                     "movw %%ax, %%gs \n"
                         /* stack at the top of the kernel's PTE */
    //                     "movl $0xC0400000, %%eax \n"
    //                     "movl %%eax, %%esp \n"
    //                     : : : "eax");

    if (screen_init())
    {

    }

    if (x86_init())
    {
        //TODO exception
    }

    struct boot_info * binfo = &bootinfo;
    
    mm_init(binfo);

    task_init(binfo);
    task_set_root();

    char *malloc_str = (char *)malloc(1024);
    //printf("malloc_str is 0x%x\n",malloc_str);
    malloc_str[2] = 1;
    //printf("valloc_str after is %d\n",&malloc_str[0]);
    //printf("malloc_str after is %d\n",&malloc_str[2]);
    printf("malloc_str after is %d\n",malloc_str[2]);


    char *malloc_str3 = (char *)malloc(1024);
    //printf("malloc_str3 is %x\n",malloc_str3);
    malloc_str3[2] = 8;
    //printf("malloc_str3[2] is %d\n",&malloc_str3[2]);
    //printf("malloc_str3[3] is %d\n",&malloc_str3[3]);
    printf("malloc_str3[2] is %d\n",malloc_str3[2]);
    printf("malloc_str[2] is %d\n",malloc_str[2]);

    //init display modules
    //we use shell to debug
    //init_font();
    //init_graphic();
    //start_refresh();


}
