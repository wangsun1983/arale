#include "core_sys_call.h"
#include "ipcmanager.h"

public int core_sys_call_ipc_regist(char *ipcname,addr_t on_transact)
{
    return ipcmanager_regist(ipcname,(onTransact)on_transact);
}

public int core_sys_call_ipc_transact(int ipc_no,addr_t buffer,
  uint32_t size,addr_t result,uint32_t result_size)
{
    return ipcmanager_transact(ipc_no,buffer,size,result,result_size);
}

public int core_sys_call_ipc_connect(char *ipcname)
{
    return ipcmanager_search(ipcname);
}
