#include "test_task.h"
#include "task.h"
#include "test_utils.h"
#include "pmm.h"
#include "klibc.h"
#include "time.h"
#include "log.h"

extern void dump_task_info(int pid,char *msg);

extern task_struct *main_thread;

//public task_struct *task_create(task_entry_fun runnable,void *data,int type);
int test_single_2_data = 2;
void test_single_2_fun(void *args)
{

    LOGD("--------------------------test_single_2_fun \n");
    test_single_2_data--;

//#ifdef DEBUG_ESP
    //addr_t val1;
    //addr_t val2;
    //__asm __volatile("movl %%esp,%0" : "=r" (val1));
    //__asm __volatile("movl %%ds,%0" : "=r" (val2));

    //task_struct *mtask = GET_CURRENT_TASK();
//LOGD("task1,ds is %x,mtask->context is %x \n",val2,mtask->context);
//#endif
    //#ifdef DEBUG_CS
    //task_struct *mtask = GET_CURRENT_TASK();
    //LOGD("ffffffff1 result is %x \n",mtask->context->pad);
    ksleep(100000);
    //LOGD("task3,pad is %x \n",mtask->context->pad);
    //task_struct *mtask = GET_CURRENT_TASK();
    //LOGD("task2,ds is %x,mtask->context is %x \n",val2,mtask->context);
    //LOGD("ffffffff2 result \n");

#ifdef DEBUG_ESP
    addr_t val1;
    addr_t val2;

    __asm __volatile("movl %%esp,%0" : "=r" (val1));
    __asm __volatile("movl %%ecx,%0" : "=r" (val2));
    task_struct *mtask = GET_CURRENT_TASK();

    LOGD("2 task[%d],esp is %x,context is %x,ecx is %x",mtask->pid,val1,&mtask->context,val2);
#endif

    //#endif

#if 0
    LOGD("cs is %x,ds is %x \n",main_thread->context->cs,main_thread->context->ds);

    addr_t val;
    __asm __volatile("movl %%cs,%0" : "=r" (val));
    LOGD("ff task cs is %x \n",val);
    __asm __volatile("movl %%ds,%0" : "=r" (val));
    LOGD("ff task ds is %x \n",val);
#endif

}

void test_single_3_fun(void *args)
{
    LOGD("--------------------------test_single_3_fun \n");
    test_single_2_data--;
}

int start_test_single_2thread()
{
    LOGD("start_test_single_2thread 1 \n");
    task_struct *task1 = task_create(test_single_2_fun,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task2 = task_create(test_single_3_fun,NULL,TASK_TYPE_DEPENDENT);

    task_start(task1);
    task_start(task2);
    LOGD("start_test_single_2thread 2 \n");
    ksleep(100000);
    if(test_single_2_data == 0)
    {
        return 1;
    }

    return -1;
}

int start_test_single_2thread_from_pool()
{
    test_single_2_data = 2;
    task_struct *task1 = task_create(test_single_2_fun,NULL,TASK_TYPE_DEPENDENT);
    task_struct *task2 = task_create(test_single_3_fun,NULL,TASK_TYPE_DEPENDENT);
    task_start(task1);
    task_start(task2);

    ksleep(100000);
    if(test_single_2_data == 0)
    {
        return 1;
    }

    return -1;
}


int start_test_dependent()
{
    TEST_ASSERT(start_test_single_2thread);
    //TEST_ASSERT(start_test_single_2thread_from_pool);
}
