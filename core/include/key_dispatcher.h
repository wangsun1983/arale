#ifndef __KEY_DISPATCHER_H__
#define __KEY_DISPATCHER_H__

#include "list.h"
#include "ctype.h"

typedef struct key_event{
    uint32_t event;
}key_event;

void key_dispatcher_init();

#endif
