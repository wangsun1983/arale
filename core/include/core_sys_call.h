#ifndef __CORE_SYS_CALL_H__
#define __CORE_SYS_CALL_H__
#include "const.h"
#include "ctype.h"

enum _SYS_CALL_ {
    CORE_SYS_CALL_GETPID = 0,
    CORE_SYS_CALL_MALLOC,
    CORE_SYS_CALL_FREE,
    CORE_SYS_PUTCHAR,
    CORE_SYS_THREAD_CREAT,
    CORE_SYS_THREAD_START,
    CORE_SYS_THREAD_STOP,
    CORE_SYS_THREAD_DESTROY,
    CORE_SYS_THREAD_JOIN,
    CORE_SYS_IPC_REGIST,
    CORE_SYS_IPC_TRANSACT,
    CORE_SYS_IPC_CONNECT,
    //TODO
};

public int core_syscall_handler(uint32_t sys_call_id,uint32_t a1, uint32_t a2,
  uint32_t a3, uint32_t a4, uint32_t a5);

public uint32_t core_sys_call_getpid();
public void core_sys_call_putchar(char *msg);
public int core_sys_call_thread_create(addr_t runnable,void *data);
public int core_sys_call_thread_destroy(int tid);
public int core_sys_call_thread_join(int tid);
public int core_sys_call_thread_start(uint32_t tid);
public int core_sys_call_thread_stop(uint32_t tid);

public int core_sys_call_ipc_regist(char *ipcname,addr_t on_transact);
public int core_sys_call_ipc_transact(int ipc_no,addr_t buffer,
  uint32_t size,addr_t result,uint32_t result_size);
public int core_sys_call_ipc_connect(char *ipcname);

#endif
