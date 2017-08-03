#ifndef __SYS_CLOCK_H__
#define __SYS_CLOCK_H__

#include "klibc.h"
#include "cmos.h"
#include "list.h"

extern struct time_t hw_time;

struct time_t {
    /* day start with 0 on startup and gets incremented
     * after every hour overflow */
    unsigned int day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned int mm;
};

typedef unsigned long milis_t;

#define RTC_TO_SEC(t)   \
    ((t) & 0xFF)
#define RTC_TO_MIN(t)   \
    ((t) >> 8)
#define RTC_TO_HOUR(t)  \
    ((t) >> 16)

typedef void (*sys_clock_handler)();

typedef struct _sys_clock_handler_list_ {
    sys_clock_handler handler;
    list_head ll;
}sys_clock_notifer;

void print_clock();
void update_clock_pit(unsigned int pit_hz);
void clock_init();
void msdelay(unsigned int delay);
void sys_clock_notify();
unsigned long long get_jiffy();

//sys_clock_register

void reg_sys_clock_handler(sys_clock_handler handler);

void init_sysclock();

//sys_clock_handler *clock_ptr;


#endif
