#include "ctype.h"
#include "list.h"
#include "mm.h"

#ifndef __PAGE_H__
#define __PAGE_H__

typedef struct mm_page 
{
    struct list_head ll;
    addr_t start_pa;
    uint32_t size;
}mm_page;

//typedef struct _mm_page mm_page;

#endif
