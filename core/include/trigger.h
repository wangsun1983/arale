#ifndef __TRIGGER_H__
#define __TRIGGER_H__
#include "list.h"
#include "ctype.h"

#define TRIGGER_IDLE 0
#define TRIGGER_INITED 1

typedef struct global_listener {
    struct list_head ll;
    void (*g_listener)(void *data);
}global_listener;

typedef void (*g_listener)(void *data);

typedef struct trigger_data {
    //struct list_head global_listener_list[GLOBAL_LISTENER_MAX];
    struct list_head *global_listener_list;
    char *bitmap;
    uint32_t trigger_num;
}trigger_t;

trigger_t *creat_trigger(int max_trigger);
void register_trigger(trigger_t *trigger,uint32_t type,g_listener listener);
void notify_trigger(trigger_t *trigger,uint32_t type,void *data);
void remove_trigger(trigger_t *trigger,uint32_t type,g_listener l);
void destroy_trigger(trigger_t *trigger);

#endif
