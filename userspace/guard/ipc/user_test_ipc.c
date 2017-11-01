#include "user_test.h"
#include "ipcthread.h"

static void onTransact(void *data,uint32_t size,void *ret,uint32_t result_length)
{
    kprintf("data is %s \n",(char *)data);
    char *result = (char *)ret;
    result[0] = 1;
    result[2] = 2;
    result[3] = 3;
    result[4] = 4;
    result[5] = 5;
}

void transact_thread(void *data)
{
    //kprintf("transact_thread \n");
    int ipc_no = ipcthread_connect("mytest");
    kprintf("transact_thread ipc no is %d \n",ipc_no);
    char *buff = "hello";
    char result[12];
    ipcthread_transact(ipc_no,buff,strlen(buff),result,12);
    //kprintf("")
    kprintf("result[0] is %d \n",result[0]);
    kprintf("result[3] is %d \n",result[3]);
    kprintf("result[5] is %d \n",result[5]);


}

int start_test_ipcthread_1()
{
    int ipc_pid = ipcthread_regist("mytest",onTransact);
    kprintf("ipc_pid is %d \n",ipc_pid);
    ksleep(100000);
    //logdisp("1 \n");
    int tid = pthread_create(transact_thread,NULL);
    //logdisp("terminal_main tid is %d\n",tid);
    kprintf("start_test_ipcthread_1 trace \n");
    pthread_start(tid);
    kprintf("start_test_ipcthread_2 trace \n");

}

int start_test_ipcthread()
{
    USER_TEST_ASSERT(start_test_ipcthread_1);
}
