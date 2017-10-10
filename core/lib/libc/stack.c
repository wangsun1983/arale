#include "stack.h"
#include "mm.h"
#include "log.h"

stack_t *stack_create(int32_t num,int32_t objsize)
{
    stack_t *stack = (stack_t *)kmalloc(sizeof(stack_t));
    kmemset(stack,0,sizeof(stack_t));
    stack->max_num = num;
    stack->obj_len = objsize;
    stack->data = (char *)kmalloc(num*objsize);
    stack->current = 0;
    kmemset(stack->data,0,num*objsize);
    return stack;
}

void stack_push(stack_t *stack,void *data)
{
    if(stack->current == stack->max_num)
    {
        return;
    }

    kmemcpy(stack->data + stack->current*stack->obj_len,data,stack->obj_len);
    stack->current++;
}

char *stack_pop(stack_t *stack)
{
    if(stack->current == 0)
    {
        return NULL;
    }

    char *data = stack->data + (stack->current - 1)*stack->obj_len;
    stack->current--;
    return data;
}

void stack_destroy(stack_t *stack)
{
    free(stack->data);
    free(stack);
}

#if 0
void stack_dump(stack_t *stack)
{
    int index = 0;
    for(;index < stack->current;index ++)
    {
        test_data_t *data = stack->data + index*stack->obj_len;
        LOGD("data is %d \n",data->data);
    }
}

void stack_pop_dump(stack_t *stack)
{
    LOGD("============ pop dump start ============ \n");
    stack_t *stack_data = stack_pop(stack);
    while(stack_data != NULL)
    {
        test_data_t *p = (test_data_t *)stack_data->data;
        LOGD("pop data is %d \n",stack_data->data);
        stack_data = stack_pop(stack);
    }
    LOGD("============ pop dump end ============ \n");
}
#endif
