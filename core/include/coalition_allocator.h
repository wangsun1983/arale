#ifndef __COALITION_ALLOCATOR_H__
#define __COALITION_ALLOCATOR_H__
#include "const.h"

public void *coalition_malloc(uint32_t size,uint32_t *alloc_size);
public int coalition_free(addr_t address);
public void coalition_allocator_init(uint32_t start_address,uint32_t size);
public void pmem_free(addr_t address);
public void *pmem_malloc(uint32_t size,uint32_t *alloc_size);

#endif
