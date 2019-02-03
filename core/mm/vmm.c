/**************************************************************
 CopyRight     :No
 FileName      :vmm.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :virtual memory interface
 History       
 20190201    * more vmm_init to mm_initiator.c
***************************************************************/
#include <klibc.h>
#include <error.h>
#include "mm.h"
#include "vmm.h"
#include "mm_common.h"
#include "mmzone.h"
#include "cache_allocator.h"
#include "sys_observer.h"
#include "pmm.h"
#include "log.h"


extern vm_root * vm_allocator_init(addr_t start_addr,uint32_t size);
extern addr_t vm_allocator_alloc(uint32_t size,vm_root *vmroot);
extern int vm_allocator_free(addr_t addr,vm_root *vmroot);

void *vmalloc_alloc_bytes(mm_struct *mm,int type,size_t size)
{
    int block = bytes_to_blocks(size);
    uint32_t page_size = block*PAGE_SIZE;
    int i = 0;

    vm_root *vmroot = NULL;
    switch(type)
    {
        case MEM_USR:
            vmroot = mm->userroot;
            break;

        case MEM_CORE:
            vmroot = mm->vmroot;
            break;

        default:
            return NULL;
    }

    addr_t va = vm_allocator_alloc(page_size,vmroot);

    addr_t start_pgd = va_to_pt_idx((addr_t)va);
    addr_t start_pte = va_to_pte_idx((addr_t)va);
    addr_t pgd = va_to_pt_idx(va + page_size);
    addr_t pte = va_to_pte_idx(va + page_size);

    //LOGD("vmalloc_alloc_bytes trace1 va is %x,start_pgd is %d,start_pte is %d,end_pgd is %d,end_pte is %d,\n",
    //       va,start_pgd,start_pte,pgd,pte);

    switch(type)
    {
        case MEM_USR:
            start_pgd = start_pgd - memory_range_user.start_pgd;
            start_pte = start_pte - memory_range_user.start_pte;
            pgd -= memory_range_user.start_pgd;
            pte -= memory_range_user.start_pte;
            //LOGD("vmalloc_alloc_bytes trace2 va is %x,start_pgd is %d,start_pte is %d,end_pgd is %d,end_pte is %d,\n",
            //     va,start_pgd,start_pte,pgd,pte);

            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = (addr_t)zone_get_page(ZONE_HIGH,PAGE_SIZE);
                //LOGD("mem is %x,i is %d \n",mem,i);
                mm->pte_user[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
                size -= PAGE_SIZE;
                if(size < 0) {
                    break;
                }
            }
            //LOGD("vmalloc_alloc_bytes trace2 \n");
            break;

        case MEM_CORE:
            for (i = PD_ENTRY_CNT*start_pgd + start_pte; i <= PD_ENTRY_CNT*pgd + pte; i++) {
                addr_t mem = (addr_t)zone_get_page(ZONE_HIGH,PAGE_SIZE);
                LOGD("vmalloc_alloc_bytes mem is %x,va is %x \n",mem,va);
                mm->pte_core[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
                size -= PAGE_SIZE;
                if(size < 0) {
                    break;
                }
            }
            break;

        default:
            return NULL;
    }

    load_pd((addr_t)mm->pgd);

    return (void *)va;
}


