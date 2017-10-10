#include "fifo_list.h"
#include "mm.h"
#include "log.h"

static void update_next_tail(fifo_list_t *list)
{
    list->tail++;
    if(list->tail == list->max_num)
    {
        list->tail = 0;
    }
}

static void update_next_head(fifo_list_t *list)
{
    list->head++;
    if(list->head == list->max_num)
    {
        list->head = 0;
    }
}

uint32_t fifo_list_get_num(fifo_list_t *list)
{
    return list->num;
}

fifo_list_t *fifo_list_create(uint32_t num,uint32_t objsize)
{
    fifo_list_t *list = (fifo_list_t *)kmalloc(sizeof(fifo_list_t));
    kmemset(list,0,sizeof(fifo_list_t));

    list->data = kmalloc(num * objsize);
    kmemset(list->data,0,num * objsize);

    list->max_num = num;
    list->obj_len = objsize;
    list->head = 0;
    list->tail = -1;
    return list;
}

void fifo_list_push(fifo_list_t *list,char *data)
{
    //printf("push 1 \n");
    if(fifo_list_get_num(list) == list->max_num)
    {
        LOGD("push full \n");
        return;
    }
    //printf("push 2 \n");
    update_next_tail(list);
    kmemcpy(list->data + list->tail*list->obj_len,data,list->obj_len);
    list->num++;
    //list->tail = get_next_tail(list);
}

char *fifo_list_pop(fifo_list_t *list)
{
    if(fifo_list_get_num(list) == 0)
    {
        return NULL;
    }

    char *data = list->data + list->head *list->obj_len;
    //list->head = get_next_head(list);
    update_next_head(list);
    list->num--;
    return data;
}

void fifo_list_destroy(fifo_list_t *list)
{
    free(list->data);
    free(list);
}

#if 0
typedef struct test_data
{
    int data;
}test_data_t;

void fifo_list_dump(fifo_list_t *list)
{
    printf("================= fifo list dump start ================== \n");
    test_data_t *data = fifo_list_pop(list);
    while(data != NULL)
    {
        printf("data is %d \n",data->data);
        data = fifo_list_pop(list);
    }
    printf("================= fifo list dump end   ================== \n");
}

int main()
{
    fifo_list_t *fifo = fifo_list_create(4,sizeof(test_data_t));
    test_data_t t1;
    t1.data = 1;
    fifo_list_push(fifo,&t1);
    printf(":::num is %d \n",get_num(fifo));

    t1.data = 2;
    fifo_list_push(fifo,&t1);
    printf(":::num is %d \n",get_num(fifo));

    t1.data = 3;
    fifo_list_push(fifo,&t1);
    printf(":::num is %d \n",get_num(fifo));

    fifo_list_pop(fifo);

    t1.data = 4;
    fifo_list_push(fifo,&t1);
    printf(":::num is %d \n",get_num(fifo));

    t1.data = 5;
    fifo_list_push(fifo,&t1);
    printf(":::num is %d \n",get_num(fifo));

    t1.data = 6;
    fifo_list_push(fifo,&t1);
    printf(":::num is %d \n",get_num(fifo));

    fifo_list_pop(fifo);
    fifo_list_pop(fifo);

    t1.data = 7;
    fifo_list_push(fifo,&t1);

    t1.data = 8;
    fifo_list_push(fifo,&t1);

    t1.data = 9;
    fifo_list_push(fifo,&t1);

    t1.data = 10;
    fifo_list_push(fifo,&t1);

    fifo_list_dump(fifo);
}
#endif
