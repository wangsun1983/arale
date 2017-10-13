#ifndef __TEST_SYSCALL_H__
#define __TEST_SYSCALL_H__

#include "ctype.h"

int start_test_syscall();
int start_test_syscall_getpid();

int test_sys_call(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

#endif
