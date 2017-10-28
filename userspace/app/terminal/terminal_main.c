#include "common.h"
#include "pthread.h"


void start_terminal_test(void *data)
{
    logdisp("start test \n");

}

void terminal_main()
{
    //we should do
    //logdisp("dd is %x",start_terminal_test);
    int tid = pthread_create(start_terminal_test,NULL);
    //logdisp("terminal_main tid is %d\n",tid);
    pthread_start(tid);
}
