#include "mmzone.h"
#include "ctype.h"
#include "page.h"
#include "mm.h"
#include "coalition_allocator.h"
#include "pmm.h"

//#define PAGE_SIZE 4096

//#define ZONE_MEMORY 1024*1024*7l


enum LIST_INSERT_TYPE {
    LIST_INSERT_FREE_LIST,
    LIST_INSERT_USED_LIST
};

typedef struct align_result 
{
    int order;
    int page_size;
}align_result;

char *full_memory;
mm_zone normal_zone;
char *start_memory; //= full_memory;
char *current_unuse_memory_index;// = start_memory;

void _coalition_list_add(list_head *new,list_head *head)
{
    mm_page *new_page = list_entry(new,mm_page,ll);
    list_head *p;
    list_head *add_head = head;

    list_for_each(p,head) {
         mm_page *page = list_entry(p,mm_page,ll);
         add_head = p;

         if(new_page->start_pa < page->start_pa) {
             add_head = p->prev;
         }
    }

    list_add(&new_page->ll,add_head);
}

void _coalition_free_list_adjust(list_head *pos,list_head *head) 
{

    align_result align_ret;
    mm_page *page = list_entry(pos,mm_page,ll);
    
    //check prev,we should check whether prev_page is the header?
    mm_page *prev_page = list_entry(pos->prev,mm_page,ll);
    
    if(&prev_page->ll != head) //dnot head
    {
        if((page->start_pa - prev_page->start_pa) == page->size) 
        {
            list_del(&page->ll);
            list_del(&prev_page->ll);
            prev_page->size += page->size;
            GET_ALIGN_PAGE(prev_page->size,&align_ret);
            _coalition_list_add(&prev_page->ll,&normal_zone.nr_area[align_ret.order].free_page_list);
            _coalition_free_list_adjust(&prev_page->ll,&normal_zone.nr_area[align_ret.order].free_page_list);
            return;
        }    
    }

    //check next
    
    mm_page *next_page = list_entry(pos->next,mm_page,ll);
    if(&next_page->ll != head) {
        if(next_page->start_pa - page->start_pa == page->size) 
        {
            list_del(&page->ll);
            list_del(&next_page->ll);
            page->size += page->size;
            GET_ALIGN_PAGE(page->size,&align_ret);
            _coalition_list_add(&page->ll,&normal_zone.nr_area[align_ret.order].free_page_list);
            _coalition_free_list_adjust(&page->ll,&normal_zone.nr_area[align_ret.order].free_page_list);
            return;
        }
    }
}

/*
*
* when we alloc a memory,the memory must include to part
* Page memory + real need memory
*/

void* _coalition_malloc(int type,uint32_t size) 
{   
    align_result align_ret;
    //kprintf("_coalition_malloc trace1 \n");
    int addr_shift = sizeof(pmm_stamp);

    if(type == PMM_TYPE_PMEM)
    {
        addr_shift = PAGE_SIZE;
    }

    GET_ALIGN_PAGE((size + addr_shift),&align_ret);
    //kprintf("coalition_malloc size is %x \n",size);
    int alignsize = align_ret.page_size;
    int order = align_ret.order;
    
    list_head *p;
    //we should first find whether there is unused memory
    list_for_each(p,&normal_zone.nr_area[order].free_page_list) {
        mm_page *page = list_entry(p,mm_page,ll);
        //we get free page
        list_del(p);
        _coalition_list_add(p,&normal_zone.nr_area[order].used_page_list);
        pmm_stamp *ret_stamp = (pmm_stamp *)page->start_pa;
        ret_stamp->type = type;
        kprintf("trace1 page->start_pa is %x,type is %x \n",page->start_pa,type);
        return (void *)(page->start_pa + addr_shift);
    }
    //kprintf("_coalition_malloc trace2 \n");

    //else we should divide a memory from Larger order memory
    order++;
    while(order < ZONE_FREE_MAX_ORDER)
    {
       int current_order = order;

       //if(normal_zone.nr_area[order].nr_free_pages >  0)
       if(!list_empty(&normal_zone.nr_area[order].free_page_list))
       {
           //hit we find a free page,split the page 
           list_for_each(p,&normal_zone.nr_area[order].free_page_list) {
               mm_page *page = list_entry(p,mm_page,ll);
               if(page->size < alignsize) 
               {
                   continue;
               } 

               list_del(p);
               if(page->size > alignsize) 
               {
                   current_order--;
                   //divide to 2 part,one is used ,another is free.
                   //uint32_t start_pa = get_workable_pa(page);
                   //kprintf("wangsl1,page->start_pa is %x,alignsize is %x \n",page->start_pa,alignsize);
                   //mm_page *another = page->start_pa + alignsize;
                   pmm_stamp *stamp = (pmm_stamp *)(page->start_pa + alignsize);
                   mm_page *another = &stamp->page;
                   //kprintf("wangsl2,anotheris %x,alignsize is %x \n",another,alignsize);
                   another->start_pa = (addr_t)stamp;
                   another->size = page->size - alignsize;

                   align_result another_align_ret;
                   GET_ALIGN_PAGE(another->size,&another_align_ret); //todo
                   int move_order = another_align_ret.order;
                   _coalition_list_add(&another->ll,&normal_zone.nr_area[move_order].free_page_list);
                   _coalition_free_list_adjust(&another->ll,&normal_zone.nr_area[move_order].free_page_list);

                   page->size = alignsize;
                   current_order = align_ret.order;//GET_FREE_ORDER(alignsize);
                   //list_add(p,&normal_zone.nr_area[order - 1].used_page_list);
               }
               
               //list_add(p,&normal_zone.nr_area[current_order].used_page_list);
               _coalition_list_add(p,&normal_zone.nr_area[current_order].used_page_list);
               //kprintf("wangsl2,page->start_pa %x,sizeof(mm_page) is %x \n",page->start_pa,sizeof(mm_page));
               //kprintf("wangsl2,result is  %x \n",page->start_pa + sizeof(mm_page));
               //kprintf("malloc page->start_pa is %x,sizeof(mm_page) is %x \n",page->start_pa , sizeof(mm_page));
               //page->type = type;
               //kprintf("trace2 page->start_pa is %x,page is %x,type is %x,pmm size is %d \n",page->start_pa,page,type,sizeof(pmm_stamp));
               pmm_stamp *ret_stamp = (pmm_stamp *)page->start_pa;
               //kprintf("trace3 ret_stamp  is %x \n",ret_stamp);
               ret_stamp->type = type;
               return (void *)(page->start_pa + addr_shift);
           }      
       } 
       order++;
    }

    return NULL;
}

void* coalition_malloc(uint32_t size)
{
    _coalition_malloc(PMM_TYPE_NORMAL,size); 
} 

void* pmem_malloc(uint32_t size)
{
    _coalition_malloc(PMM_TYPE_PMEM,size); 
} 

//when free memory ,we should merge unused memory to a free memory
int coalition_free(addr_t address) 
{
    //kprintf("free address is %x \n",address);
    pmm_stamp *stamp  = (pmm_stamp *)(address - sizeof(pmm_stamp));

    if(stamp->type != PMM_TYPE_NORMAL)
    {
        return -1;
    }

    mm_page *page = &stamp->page;
    
    //we should move this page to free page
    align_result ret;

    GET_ALIGN_PAGE(page->size,&ret);
    
    //kprintf("_coalition_free1,page-size is %x,order is %d \n",page->size,ret.order);
    list_del(&page->ll);
    _coalition_list_add(&page->ll,&normal_zone.nr_area[ret.order].free_page_list);
    //kprintf("page->ll %x,order is %d \n",&page->ll);
    //kprintf("page->ll prev is %x\n",&page->ll.prev);
    //kprintf("page->ll next is %x\n",&page->ll.next);

    //kprintf("_free_page_list is %x \n",&normal_zone.nr_area[ret.order].free_page_list);
    _coalition_free_list_adjust(&page->ll,&normal_zone.nr_area[ret.order].free_page_list);

    return 0;
}

void pmem_free(addr_t address)
{
    //mm_page *page = address - PAGE_SIZE;
    pmm_stamp *stamp  = (pmm_stamp *)(address - PAGE_SIZE);
    mm_page *page = &stamp->page;
    
    //we should move this page to free page
    align_result ret;

    GET_ALIGN_PAGE(page->size,&ret);
    
    //kprintf("_coalition_free1,page-size is %x,order is %d \n",page->size,ret.order);
    list_del(&page->ll);
    _coalition_list_add(&page->ll,&normal_zone.nr_area[ret.order].free_page_list);
    //kprintf("page->ll %x,order is %d \n",&page->ll);
    //kprintf("page->ll prev is %x\n",&page->ll.prev);
    //kprintf("page->ll next is %x\n",&page->ll.next);

    //kprintf("_free_page_list is %x \n",&normal_zone.nr_area[ret.order].free_page_list);
    _coalition_free_list_adjust(&page->ll,&normal_zone.nr_area[ret.order].free_page_list);
}

void dump() 
{
    list_head *p;
    int order = 0;
    while(order < ZONE_FREE_MAX_ORDER) 
    {
        kprintf("============= order:%d ============= \n",order);
        kprintf("free page: \n");

        int index = 0;
        list_for_each(p,&normal_zone.nr_area[order].free_page_list) {
            mm_page *page = list_entry(p,mm_page,ll);
            kprintf("   %d: page size is %x \n",index,page->size);
            kprintf("   %d: page start_pa is %x,addr is %x \n",index,page->start_pa,page);
            index++;
        }

        kprintf("used page: \n");
        index = 0;
        list_for_each(p,&normal_zone.nr_area[order].used_page_list) {
            mm_page *page = list_entry(p,mm_page,ll);
            kprintf("   %d: page size is %x \n",index,page->size);
            kprintf("   %d: page start_pa is %x,addr is %x\n",index,page->start_pa,page);
            index++;
        }
        order++;
    }
}

//void memory_split( )

//we should use a table to  manage memory
void coalition_allocator_init(addr_t start_address,uint32_t size)
{
    int index = 0;

    //pre-init
    //full_memory = start_address;

    //start_memory = full_memory;
    //current_unuse_memory_index = start_memory;

    //_coalition_pre_alloc_pages = _coalition_malloc_(sizeof(page));
    //INIT_LIST_HEAD(_coalition_pre_alloc_pages);
    //mm_zone *myzone = &normal_zone;
    //myzone->size = size;
    //myzone->start_pa = full_memory;

    //we should alloc a memory to manage all the memory
    pmm_stamp *stamp = (pmm_stamp *)start_address;
    mm_page *_coalition_all_alloc_pages = &stamp->page;
    _coalition_all_alloc_pages->size = size;
    _coalition_all_alloc_pages->start_pa = start_address;

    //mm_page *another = _coalition_all_alloc_pages->start_pa + ZONE_MEMORY/2;
    //another->
    //INIT_LIST_HEAD(&_coalition_pre_alloc_pages->ll);

    for(;index < ZONE_FREE_MAX_ORDER;index++)
    {
       INIT_LIST_HEAD(&normal_zone.nr_area[index].free_page_list);
       INIT_LIST_HEAD(&normal_zone.nr_area[index].used_page_list);
       //normal_zone.nr_area[index].nr_free_pages = 0;
    }
    
    //we alos need to add the memory to list

    //int order = GET_FREE_ORDER(_coalition_all_alloc_pages->size);
    align_result ret;
    GET_ALIGN_PAGE(_coalition_all_alloc_pages->size,&ret);

    //normal_zone.nr_area[ret.order].nr_free_pages = 1;

    list_add(&_coalition_all_alloc_pages->ll,&normal_zone.nr_area[ret.order].free_page_list);
}
