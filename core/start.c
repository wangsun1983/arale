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
#include "fs.h"
#include "sysclock.h"

extern void init_font();
extern void init_graphic();
extern void start_refresh();
extern void hdd_init();

static int screen_init()
{
    set_color(VID_CLR_BLACK, VID_CLR_LIGHT_BLUE);
    clear_screen();
    goto_xy(0, 0);

    return 0;
}


void start_core(struct boot_info bootinfo)
{
    struct boot_info * binfo = &bootinfo;

    screen_init();

    init_gdt();

    x86_init();

    mm_init(binfo);

    init_sysclock();

    task_init(binfo);

    hdd_init();

    fs_init();

    start_sysclock();

    init_timer();

    //test
    //kprintf("start fs_create");
    //fs_create("root0/abc/",FT_DIRECTORY);
    kprintf("start test \n");

#if 0
    //1.fs_write

    //char *mytest = "11111111111111111111111111";
    //fs_write(fd0,mytest,kstrlen(mytest),WRITE_NORMAL);
    //uint32_t fd1 = fs_open("root0/abc10.txt",FT_FILE);
    kprintf("fd1 is %x \n",fd0);
    char *mytest2 = "123456789";
    fs_write(fd0,mytest2,kstrlen(mytest2),WRITE_APPEND);
#endif

    uint32_t fd0 = fs_create("root0/abc7.txt",FT_FILE);
    //2.fs_read
    char *file = (char *)kmalloc(1024 *8);
    kmemset(file,0,1024*8);
    int index = 0;

    for(;index <1024*8 - 1;index ++)
    {
        file[index] = 'd';
        /*file[index] = (index+1)%255;

        if(file[index] == 0)
        {
             file[index] = 2;
        }
        */
    }

    //uint32_t fd0 = fs_create("root0/abc7.txt",FT_FILE);
    uint32_t fd = fs_open("root0/abc7.txt");
    //kprintf("file length is %d \n",kstrlen(file));
    fs_write(fd,file,kstrlen(file),WRITE_NORMAL);
    char *append = "fley";
    kprintf("file write append start \n");
    fs_write(fd,append,kstrlen(append),WRITE_APPEND);
    kprintf("file write append end \n");
    //TODO
#if 0
    char *read = (char *)kmalloc(1024 *8);
    kmemset(read,0,1024*8);
    fs_read(fd,read,1024*8,0);
    int check = 0;
    for(;check < 1024*8;check++)
    {
        if(file[check] != read[check])
        {
            kprintf("read error,check is %d,file[check] is %d,read[check] is %d \n"
                    ,check,file[check],read[check]);
            break;
        }
    }
#endif

    kprintf("start successfully!!!!!! \n");

    while(1){}
}
