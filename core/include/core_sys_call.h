#ifndef __CORE_SYS_CALL_H__
#define __CORE_SYS_CALL_H__
#include "const.h"
#include "ctype.h"

enum _SYS_CALL_ {
    CORE_SYS_CALL_GETPID = 0,
    CORE_SYS_CALL_MALLOC,
    CORE_SYS_CALL_FREE,
    CORE_SYS_PUTCHAR,
    //TODO
};

public int core_syscall_handler(uint32_t sys_call_id,uint32_t a1, uint32_t a2,
  uint32_t a3, uint32_t a4, uint32_t a5);

public uint32_t core_sys_call_getpid();
public void core_sys_call_putchar(int msg);


#endif
