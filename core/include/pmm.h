#ifndef _PMM_H_
#define _PMM_H_


/* PMM */
addr_t pmm_init(unsigned int mem_kb, addr_t bitmap_loc);
int pmm_init_region(unsigned int addr, size_t size);
void *pmm_alloc(unsigned int bytes);
void pmm_dealloc_page(addr_t addr);
size_t get_total_mem_b();
size_t get_free_mem_b();
size_t get_used_mem_b();
size_t get_krnl_size();


#endif /* end of include guard: MM_ZPVRK7R1 */
