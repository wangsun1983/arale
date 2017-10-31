#include "task.h"
#include "ipcmanager.h"
#include "hashmap.h"
#include "semaphore.h"
#include "mm.h"

#define IPC_LIST_LENGTH 1024

private hashmap_t *ipc_table;

typedef struct ipc_data {
    semaphore *in_sem;
    semaphore *out_sem;

    uint32_t index;
    onTransact on_transact;
    void *data;
    int data_length;

    void *result;
    int result_length;
}ipc_data_t;

private char *key_list[IPC_LIST_LENGTH];
private int key_index;
//ipc_data_t ipc_list[IPC_LIST_LENGTH];
//int ipc_index = 0;

private void ipc_run(void *data);

public void ipcmanager_init()
{
    ipc_table = hashmap_create(NULL,NULL,NULL,NULL);
    key_index = 0;
    kmemset(key_list,0,sizeof(char *)*IPC_LIST_LENGTH);
}

public int ipcmanager_regist(char *ipcname,onTransact on_transact)
{
    ipc_data_t *_ipc_data = (ipc_data_t *)kmalloc(sizeof(ipc_data_t));
    int ipc_no = key_index;
    char *key = kmalloc(kstrlen(ipcname) + 1);
    kmemset(key,0,kstrlen(ipcname) + 1);
    hashmap_put(ipc_table,key,_ipc_data);
    key_list[ipc_no] = key;
    key_index++;

    _ipc_data->index = ipc_no;
    _ipc_data->on_transact = on_transact;
    _ipc_data->in_sem = sem_create();
    _ipc_data->out_sem = sem_create();

    task_struct *ipctask = task_create_user_process(ipc_run,_ipc_data,TASK_TYPE_DEPENDENT);
    task_start(ipctask);

    return  ipc_no;
}

public int ipcmanager_transact(int ipc_no,addr_t buffer,uint32_t size,addr_t result,uint32_t result_size)
{
    char *key = key_list[ipc_no];
    ipc_data_t *_ipc_data = hashmap_get(ipc_table,key);
    if(_ipc_data == NULL)
    {
        LOGE("ipc transact:ipc find fail \n");
        return -1;
    }

    //we should copy data from user to kernel
    char *core_data = copy_from_user(buffer,size);
    _ipc_data->data = core_data;
    _ipc_data->data_length = size;

    char *result_data = copy_from_user(result,size);
    _ipc_data->result = result_data;
    _ipc_data->result_length = result_size;

    sem_up(_ipc_data->in_sem);
    sem_down(_ipc_data->out_sem);

    //we get result
    copy_to_user_withdata(result,_ipc_data->result,_ipc_data->result_length);
    free(_ipc_data->data);
    free(_ipc_data->result);
    return IPC_RESULT_SUCCESS;
}

public int ipcmanager_search(char *key)
{
    ipc_data_t *_ipc_data = hashmap_get(ipc_table,key);
    return _ipc_data->index;
}

private void ipc_run(void *data)
{
    ipc_data_t *data_t = (ipc_data_t *)data;

    while(true)
    {
        sem_down(data_t->in_sem);

        data_t->on_transact(data_t->data,data_t->data_length,
          data_t->result,data_t->result_length);

        sem_up(data_t->out_sem);
    }
}
