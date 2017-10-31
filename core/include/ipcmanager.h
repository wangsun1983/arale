#ifndef __IPCMANAGER_H__
#define __IPCMANAGER_H__

#include "ctype.h"

enum IPC_RESULT {
    IPC_RESULT_FAIL = -1,
    IPC_RESULT_SUCCESS,
};

typedef void (*onTransact)(void *data,uint32_t size,void *result,uint32_t result_length);

public void ipcmanager_init();
public int ipcmanager_regist(char *ipcname,onTransact transact);
public int ipcmanager_transact(int ipc_no,addr_t buffer,uint32_t size,addr_t result,uint32_t result_size);
public int ipcmanager_search(char *key);

#endif
