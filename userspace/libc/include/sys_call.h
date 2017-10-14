#ifndef __SYS_CALL_H__
#define __SYS_CALL_H__
#include "const.h"
#include "common.h"


enum _SYS_CALL_ {
    SYS_GETPID = 0,
    SYS_MALLOC,
    SYS_FREE,
    SYS_PUTCHAR
    //TODO
};

public int sys_call(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
public void sys_putchar(char msg);
#endif
