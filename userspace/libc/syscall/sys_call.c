#include "sys_call.h"

public int sys_call(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    int ret;

    asm volatile("int %1\n"
  		: "=a" (ret)
  		: "i" (0x31),
  		  "a" (num),
  		  "d" (a1),
  		  "c" (a2),
  		  "b" (a3),
  		  "D" (a4),
  		  "S" (a5)
  		: "cc", "memory");

    return ret;

}
