/**************************************************************
 CopyRight     :No
 FileName      :coalition_allocator.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :physical memory allocator
 20190202      :update PAGE_ALIGN algorithm
                old:  0.035S (cpu:i7 times:1024*1024)
                new:  0.007S (cpu:i7 times:1024*1024)
***************************************************************/

#include "mmzone.h"
#include "mm_common.h"
#include "ctype.h"
#include "page.h"
#include "mm.h"
#include "coalition_allocator.h"
#include "pmm.h"
#include "klibc.h"
#include "log.h"

/*----------------------------------------------
                local struct
----------------------------------------------*/
enum LIST_INSERT_TYPE {
    LIST_INSERT_FREE_LIST,
    LIST_INSERT_USED_LIST
};

typedef struct align_result
{
    int order;
    int page_size;
}align_result;

/*----------------------------------------------
                local data
----------------------------------------------*/
private mm_zone normal_zone;

/*----------------------------------------------
                local method
----------------------------------------------*/
private void _coalition_list_add(list_head *new,list_head *head);
private void _coalition_free_list_adjust(list_head *pos,list_head *head);
private void* _coalition_malloc(int type,uint32_t size,uint32_t *alloc_size);
private int _get_align_page_order(int size);

/*----------------------------------------------
                public method
----------------------------------------------*/
public void* coalition_malloc(uint32_t size,uint32_t *alloc_size)
{
    _coalition_malloc(PMM_TYPE_NORMAL,size,alloc_size);
}

public void* pmem_malloc(uint32_t size,uint32_t *alloc_size)
{
    _coalition_malloc(PMM_TYPE_PMEM,size,alloc_size);
}

//when free memory ,we should merge unused memory to a free memory
public int coalition_free(addr_t address)
{
    //LOGD("coalition_free start \n");
    pmm_stamp *stamp  = (pmm_stamp *)(address - sizeof(pmm_stamp));

    if(stamp->type != PMM_TYPE_NORMAL)
    {
        return -1;
    }

    mm_page *page = &stamp->page;

    //we should move this page to free page
    int order = _get_align_page_order(page->size);

    list_del(&page->ll);
    _coalition_list_add(&page->ll,&normal_zone.nr_area[order].free_page_list);
    _coalition_free_list_adjust(&page->ll,&normal_zone.nr_area[order].free_page_list);

    return 0;
}

public void pmem_free(addr_t address)
{
    //mm_page *page = address - PAGE_SIZE;
    int addr_shift = sizeof(pmm_stamp);
    addr_shift = PAGE_SIZE_RUND_UP(addr_shift); //pmemory should be entire page.
    pmm_stamp *stamp  = (pmm_stamp *)(address - addr_shift);
    mm_page *page = &stamp->page;

    //we should move this page to free page
    int order = _get_align_page_order(page->size);
    list_del(&page->ll);
    _coalition_list_add(&page->ll,&normal_zone.nr_area[order].free_page_list);

    //LOGD("page->ll %x,order is %d \n",&page->ll);
    //LOGD("page->ll prev is %x\n",&page->ll.prev);
    //LOGD("page->ll next is %x\n",&page->ll.next);
    //LOGD("_free_page_list is %x \n",&normal_zone.nr_area[ret.order].free_page_list);
    _coalition_free_list_adjust(&page->ll,&normal_zone.nr_area[order].free_page_list);
}

//we should use a table to  manage memory
public void coalition_allocator_init(addr_t start_address,uint32_t size)
{
    int index = 0;

    //we should alloc a memory to manage all the memory
    pmm_stamp *stamp = (pmm_stamp *)start_address;
    //LOGD("start address is %d \n",start_address);
    mm_page *_coalition_all_alloc_pages = &stamp->page;
    _coalition_all_alloc_pages->size = size;
    _coalition_all_alloc_pages->start_pa = start_address;

    for(;index < ZONE_FREE_MAX_ORDER;index++)
    {
       INIT_LIST_HEAD(&normal_zone.nr_area[index].free_page_list);
       INIT_LIST_HEAD(&normal_zone.nr_area[index].used_page_list);
    }

    //we alos need to add the memory to list
    //align_result ret;
    //GET_ALIGN_PAGE(_coalition_all_alloc_pages->size,&ret);
    int order = _get_align_page_order(_coalition_all_alloc_pages->size);
    //LOGD("coalition_allocator_init address is %d,ret.order is %d,size is %d \n",start_address,ret.order,_coalition_all_alloc_pages->size);
    //LOGD("coalition init,page is %d,size is %d \n",_coalition_all_alloc_pages,_coalition_all_alloc_pages->size);
    list_add(&_coalition_all_alloc_pages->ll,&normal_zone.nr_area[order].free_page_list);
    
    //list_head *p;
    //list_for_each(p,&normal_zone.nr_area[ret.order].free_page_list) {
    //    mm_page *page = list_entry(p,mm_page,ll);
    //    LOGD("coalition init2,page is %d,size is %d \n",page,page->size);
    //}
}

/*----------------------------------------------
                private method
----------------------------------------------*/

private int _get_align_page_order(int size)
{
    if(size == 0) 
    {
        return 0;
    }

    int val = PAGE_SIZE_RUND_UP(size);
    int order = common_lg2(val);
    // /LOGD("fast val is %d order is %d,size is %d \n",val,order,size);
    int select = 1<<order;
    if(val > select) {
        //4K = 2^12
        return (order + 1) - 12;
    } else {
        return (order - 12);
    }
}


private void _coalition_list_add(list_head *new,list_head *head)
{
    mm_page *new_page = list_entry(new,mm_page,ll);
    list_head *p;
    list_head *add_head = head;

    list_for_each(p,head) {
        mm_page *page = list_entry(p,mm_page,ll);
        add_head = p;
        
        //all the page should save in order for memory merge.
        if(new_page->start_pa < page->start_pa) {
            add_head = p->prev;
            break;
        }
    }

    list_add(&new_page->ll,add_head);
}

private void _coalition_free_list_adjust(list_head *pos,list_head *head)
{

    align_result align_ret;
    mm_page *page = list_entry(pos,mm_page,ll);

    //check prev,we should check whether prev_page is the header?
    mm_page *prev_page = list_entry(pos->prev,mm_page,ll);

    if(&prev_page->ll != head) //not head
    {
        if((page->start_pa - prev_page->start_pa) == prev_page->size)
        {
            list_del(&page->ll);
            list_del(&prev_page->ll);
            prev_page->size += page->size;
            //GET_ALIGN_PAGE(prev_page->size,&align_ret);
            int order = _get_align_page_order(prev_page->size);
            _coalition_list_add(&prev_page->ll,&normal_zone.nr_area[order].free_page_list);
            _coalition_free_list_adjust(&prev_page->ll,&normal_zone.nr_area[order].free_page_list);
            return;
        }
    }

    //check next
    mm_page *next_page = list_entry(pos->next,mm_page,ll);
    if(&next_page->ll != head)
    {
        if(next_page->start_pa - page->start_pa == page->size)
        {
            list_del(&page->ll);
            list_del(&next_page->ll);
            page->size += next_page->size;
            //GET_ALIGN_PAGE(page->size,&align_ret);
            int order = _get_align_page_order(page->size);
            _coalition_list_add(&page->ll,&normal_zone.nr_area[order].free_page_list);
            _coalition_free_list_adjust(&page->ll,&normal_zone.nr_area[order].free_page_list);
            return;
        }
    }
}

/*
*
* when we alloc a memory,the memory must include to part
* Page memory + real need memory
*/
private void* _coalition_malloc(int type,uint32_t size,uint32_t *alloc_size)
{
    align_result align_ret;
    int addr_shift = sizeof(pmm_stamp);

    if(type == PMM_TYPE_PMEM)
    {
        addr_shift = PAGE_SIZE_RUND_UP(addr_shift); //pmemory should be entire page.
    }

    //GET_ALIGN_PAGE((size + addr_shift),&align_ret);
    int order = _get_align_page_order((size + addr_shift));
    int align_order = order;

    addr_t alignsize = 1<<(order+12);
    
    *alloc_size = alignsize;

    list_head *p;

    //we should first find whether there is unused memory
    list_for_each(p,&normal_zone.nr_area[order].free_page_list) {
        mm_page *page = list_entry(p,mm_page,ll);
        list_del(p);
        _coalition_list_add(p,&normal_zone.nr_area[order].used_page_list);
        pmm_stamp *ret_stamp = (pmm_stamp *)page->start_pa;
        ret_stamp->type = type;
        return (void *)(page->start_pa + addr_shift);
    }

    //if we can't find a proper memory,find a bigger one.
    order++;

    while(order < ZONE_FREE_MAX_ORDER)
    {
       int current_order = order;

       if(!list_empty(&normal_zone.nr_area[order].free_page_list))
       {
           //hit we find a free page,split the page
           list_for_each(p,&normal_zone.nr_area[order].free_page_list) {
               mm_page *page = list_entry(p,mm_page,ll);

               if(page->size < alignsize)
               {
                   //if a memory is 15K,it may be save in 16K's free_page_list
                   //we need check
                   continue;
               }

               list_del(p);

               if(page->size > alignsize)
               {
                    //  divide memory into 2 part
                    //    |........|........[alignsize]...........|.......
                    //  stamp  start_pa                      another stamp(not used,need to save to free list)
                    pmm_stamp *stamp = (pmm_stamp *)(page->start_pa + alignsize);
                    mm_page *another = &stamp->page;
                    another->start_pa = (addr_t)stamp;
                    another->size = page->size - alignsize;

                    //align_result another_align_ret;
                    //GET_ALIGN_PAGE(another->size,&another_align_ret); //todo
                    int move_order = _get_align_page_order(another->size);
                    _coalition_list_add(&another->ll,&normal_zone.nr_area[move_order].free_page_list);
                    _coalition_free_list_adjust(&another->ll,&normal_zone.nr_area[move_order].free_page_list);
                    page->size = alignsize;
                    current_order = align_order;//GET_FREE_ORDER(alignsize);
               }

               _coalition_list_add(p,&normal_zone.nr_area[current_order].used_page_list);
               pmm_stamp *ret_stamp = (pmm_stamp *)page->start_pa;
               ret_stamp->type = type;
               //LOGD("coalition malloc trace2 is page is %d,order is %d,start pa is %d \n",page,order,page->start_pa);
               return (void *)(page->start_pa + addr_shift);
           }
       }
       order++;
    }

    return NULL;
}

private void dump()
{
    list_head *p;
    int order = 0;
    while(order < ZONE_FREE_MAX_ORDER)
    {
        LOGI("============= order:%d ============= \n",order);
        LOGI("free page: \n");

        int index = 0;
        list_for_each(p,&normal_zone.nr_area[order].free_page_list) {
            mm_page *page = list_entry(p,mm_page,ll);
            LOGI("   %d: page size is %x \n",index,page->size);
            LOGI("   %d: page start_pa is %x,addr is %x \n",index,page->start_pa,page);
            index++;
        }

        LOGI("used page: \n");
        index = 0;
        list_for_each(p,&normal_zone.nr_area[order].used_page_list) {
            mm_page *page = list_entry(p,mm_page,ll);
            LOGI("   %d: page size is %x \n",index,page->size);
            LOGI("   %d: page start_pa is %x,addr is %x\n",index,page->start_pa,page);
            index++;
        }
        order++;
    }
}
