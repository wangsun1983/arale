#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include "ctype.h"

#define DEFAULT_ENLARGE_SIZE 2

typedef struct array_data
{
    void **array;
    uint32_t next;
    uint32_t length;
}array_data_t;

public array_data_t *array_list_create();
public void array_list_add(array_data_t *list,void *data);
public void array_list_remove_position(array_data_t *list,uint32_t position);
public void array_list_add_head(array_data_t *list,void *data);


#endif
