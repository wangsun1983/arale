#ifndef __STACK_H__
#define __STACK_H__

#include "ctype.h"

typedef struct _stack
{
    char *data;
    uint32_t max_num;
    uint32_t obj_len;
    int current;
}stack_t;

public stack_t *stack_create(int32_t num,int32_t objsize);
public void stack_push(stack_t *stack,void *data);
public char *stack_pop(stack_t *stack);
public void stack_destroy(stack_t *stack);


#endif
