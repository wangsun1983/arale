#ifndef __VM_ALLOCATOR_H__
#define __VM_ALLOCATOR_H__

int vm_allocator_free(addr_t addr,vm_root *vmroot);
vm_root * vm_allocator_init(addr_t start_addr,uint32_t size);
addr_t vm_allocator_alloc(uint32_t size,vm_root *vmroot);
void vm_scan_merge(vm_root *vmroot);

#endif
