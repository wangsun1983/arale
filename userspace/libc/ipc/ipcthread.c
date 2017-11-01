#include "ipcthread.h"
#include "sys_call.h"
#include "common.h"

public int ipcthread_regist(char *ipcname,ipcOnTransact on_transact)
{
    //logdisp("ipcthread_regist len is %d \n",strlen(ipcname));
    int ipc_no =  sys_call_ipc_regist(ipcname,(addr_t)on_transact);
    kprintf("ipcthread_regist,ipc_no is %d \n",ipc_no);
    return ipc_no;
}

public int ipcthread_transact(int ipc_no,addr_t buffer,uint32_t size,addr_t result,uint32_t result_size)
{
    //kprintf("ipcthread_transact start \n");
    return sys_call_ipc_transact(ipc_no,buffer,size,result,result_size);
}

public int ipcthread_connect(char *ipcname)
{
   return sys_call_ipc_connect(ipcname);
}
