#include "sys_call.h"

public int sys_call_ipc_regist(char *ipcname,addr_t on_transact)
{
    return sys_call(SYS_IPC_REGIST,(uint32_t)ipcname,on_transact,0,0,0);
}

public int sys_call_ipc_transact(int ipc_no,addr_t buffer,
  uint32_t size,addr_t result,uint32_t result_size)
{
    return sys_call(SYS_IPC_TRANSACT,ipc_no,buffer,size,result,result_size);
}

public int sys_call_ipc_connect(char *ipcname)
{
    return sys_call(SYS_IPC_CONNECT,(uint32_t)ipcname,0,0,0,0);
}
