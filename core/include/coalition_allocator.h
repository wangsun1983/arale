#ifndef __COALITION_ALLOCATOR_H__
#define __COALITION_ALLOCATOR_H__

#define COALITION_TYPE_NORMAL 1
#define COALITION_TYPE_PMEM 2

void *coalition_malloc(uint32_t size);
void coalition_free(uint32_t address);
void coalition_allocator_init(uint32_t start_address,uint32_t size);
void pmem_free(addr_t address);
void *pmem_malloc(uint32_t size);

#endif
