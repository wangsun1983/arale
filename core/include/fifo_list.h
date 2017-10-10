#ifndef __FIFO_LIST_H__
#define __FIFO_LIST_H__
#include "ctype.h"

typedef struct _fifo_list
{
    char *data;
    uint32_t max_num;
    uint32_t obj_len;
    uint32_t head;
    uint32_t tail;
    uint32_t num;
}fifo_list_t;

public fifo_list_t *fifo_list_create(uint32_t num,uint32_t objsize);
public void fifo_list_push(fifo_list_t *list,char *data);
public char *fifo_list_pop(fifo_list_t *list);
public void fifo_list_destroy(fifo_list_t *list);
public uint32_t fifo_list_get_num(fifo_list_t *list);

#endif
