#include "arraylist.h"
#include "klibc.h"
#include "log.h"
#include "mm.h"

static void array_list_realloc(array_data_t *list);

array_data_t *array_list_create()
{
    array_data_t *data = (array_data_t *)kmalloc(sizeof(array_data_t));
    kmemset(data,0,sizeof(array_data_t));

    return data;
}

void array_list_add(array_data_t *list,void *data)
{
    if(list->length == 0)
    {
        array_list_realloc(list);
    }

    list->array[list->next] = data;
    list->length--;
    list->next++;
}

void array_list_remove_position(array_data_t *list,uint32_t position)
{
    if(list->next < position)
    {
        LOGE("array_list_remove_position error! \n");
    }

    if(position != list->next - 1)
    {
        void *copy_buf[list->next - position];
        kmemcpy((char *)copy_buf,(char *)&list->array[position + 1],(list->next - position)*sizeof(void *));
        kmemcpy((char *)&list->array[position],(char *)copy_buf,(list->next - position)*sizeof(void *));
    }

    list->next--;
    list->length++;
}

void array_list_add_head(array_data_t *list,void *data)
{
    list->length++;
    uint32_t size = list->next + list->length;

    void **array = kmalloc(size * sizeof(void *));
    kmemset(array,0,sizeof(void *)*size);

    array[0] = data;
    kmemcpy((char *)&array[1],(char *)list->array,sizeof(void *)*list->next);
    free(list->array);
    list->array = array;
    list->next++;
    list->length--;
}

void array_list_realloc(array_data_t *list)
{
    uint32_t enlargesize = list->next;
    if(enlargesize == 0)
    {
        enlargesize = DEFAULT_ENLARGE_SIZE;
    }

    uint32_t length = list->next + enlargesize;
    void **array = kmalloc(length*sizeof(void *));
    kmemset(array,0,length*sizeof(void *));

    void **old_array = list->array;
    list->array = array;
    list->length = enlargesize;

    if(old_array != NULL)
    {
        //start copy
        kmemcpy((char *)array,(char *)old_array,list->next*sizeof(void *));

        if(old_array != NULL)
        {
            free(old_array);
        }
    }
}
