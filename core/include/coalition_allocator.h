#ifndef __COALITION_ALLOCATOR_H__
#define __COALITION_ALLOCATOR_H__

enum COALITION_TYPE {
    COALITION_TYPE_NORMAL = 0,
    COALITION_TYPE_PMEM,
    COALITION_TYPE_CACHE
};

void *coalition_malloc(uint32_t size);
int coalition_free(addr_t address);
void coalition_allocator_init(uint32_t start_address,uint32_t size);
void pmem_free(addr_t address);
void *pmem_malloc(uint32_t size);

#endif
