#include "ctype.h"
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
#include "sys_observer.h"
#include "key_dispatcher.h"
#include "log.h"
#include "mouse.h"
#include "ipcmanager.h"

#define GUARD_TEST
#ifdef GUARD_TEST
#include "test_fs.h"
#include "test_main.h"
#endif

extern void init_font();
extern void init_graphic();
extern void start_refresh();
extern void hdd_init();

extern void helloworld_main();

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

#ifdef ENABLE_GRAPHIC
    init_font();
    init_graphic();
    start_refresh();
#else
    screen_init();
#endif

    init_gdt();

    x86_init();

    mm_init(binfo);

    //LOGD("mem_size is %d,krnl_size is %d,krnl_loc is %d \n",binfo->mem_size,binfo->krnl_size,binfo->krnl_loc);

    //init_sysclock();
    task_init(binfo);
    sys_observer_init();
    task_start_sched();
    //cache_allocator_start_monitor();

    hdd_init();

    fs_init();

    start_sysclock();

    init_timer();

#ifdef GUARD_TEST
    start_test();
#endif
    //init device
    //ipcmanager_init();

    //key_dispatcher_init();
    //console_dispatcher_init();
    LOGD("start successfully!!!!!! \n");

//#ifdef GUARD_USER_TEST
    //start_user_test();
//#endif
    //terminal_main();
    //helloworld_main();
    //mouse_init();
    //LOGE("aaa \n");
    while(1){}
}
