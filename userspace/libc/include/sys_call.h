#ifndef __SYS_CALL_H__
#define __SYS_CALL_H__
#include "const.h"
#include "common.h"


enum _SYS_CALL_ {
    SYS_GETPID = 0,
    SYS_MALLOC,
    SYS_FREE,
    SYS_PUTCHAR,
    SYS_THREAD_CREAT,
    SYS_THREAD_START,
    SYS_THREAD_STOP,
    SYS_THREAD_DESTROY,
    SYS_THREAD_JOIN,
    SYS_IPC_REGIST,
    SYS_IPC_TRANSACT,
    SYS_IPC_CONNECT,
    //TODO
};

public int sys_call(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
public void sys_putchar(char msg);
public int sys_call_thread_create(addr_t runnable,void *data);
public int sys_call_thread_start(uint32_t tid);
public int sys_call_thread_stop(uint32_t tid);
public int sys_call_thread_destroy(int tid);
public int sys_call_thread_join(int tid);

public int sys_call_ipc_regist(char *ipcname,addr_t on_transact);
public int sys_call_ipc_transact(int ipc_no,addr_t buffer,
  uint32_t size,addr_t result,uint32_t result_size);
public int sys_call_ipc_connect(char *ipcname);

#endif
