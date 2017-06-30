#include "ctype.h"

#ifndef __PAGE_H__
#define __PAGE_H__

typedef struct mm_page 
{
    struct list_head ll;
    uint32_t start_pa;
    uint32_t size;
}mm_page;


#endif
