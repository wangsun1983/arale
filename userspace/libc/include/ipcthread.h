#ifndef __IPC_THREAD_H__
#define __IPC_THREAD_H__

#include "common.h"

typedef void (*ipcOnTransact)(void *data,uint32_t size,void *result,uint32_t result_length);

public int ipcthread_regist(char *ipcname,ipcOnTransact on_transact);
public int ipcthread_transact(int ipc_no,addr_t buffer,
   uint32_t size,addr_t result,uint32_t result_size);
public int ipcthread_connect(char *ipcname);   

#endif
