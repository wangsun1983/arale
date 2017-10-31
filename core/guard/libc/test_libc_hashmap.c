#include "test_libc.h"
#include "hashmap.h"
#include "log.h"
#include "test_utils.h"

typedef struct hashmap_test_data
{
    int j;
}hashmap_test_data;

int test_hashmap_1()
{
    hashmap_t *hashmap = hashmap_create(NULL,NULL,NULL,NULL);
    hashmap_test_data data1;
    data1.j = 100;
    hashmap_put(hashmap,"a",&data1);

    hashmap_test_data data2;
    data2.j = 101;
    hashmap_put(hashmap,"b",&data2);

    hashmap_test_data *data = (hashmap_test_data *)hashmap_get(hashmap,"b");
    LOGD("data j is %d \n",data->j);
    if(data->j != 101)
    {
        return  -1;
    }

    return 1;
}

int start_test_hashmap()
{
    TEST_ASSERT(test_hashmap_1);
}
