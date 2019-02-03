#ifndef __MM_COMMON_H__
#define __MM_COMMON_H__

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define PAGE_SIZE_RUND_UP(x) ALIGN(x,PAGE_SIZE)
#define PAGE_SIZE_RUND_DOWN(x) x>>11<<11

#endif