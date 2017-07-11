#include "page.h"
#include "mmzone.h"
#include "list.h"
#include "mm.h"

//#define PAGE_SIZE 4096
//#define ZONE_MEMORY 1024*1024*7l

mm_zone high_memory_zone;
mm_page *total_page;
struct list_head free_page_list;
struct list_head used_page_list;

//because vmalloc can use uncontinous memory,
//so we can alloc 4K page 
void fragment_allocator_init(addr_t start_addr,uint32_t size)
{

    if(size < PAGE_SIZE) 
    {
        //printf("PAGE SIZE error!!!!! \n");
        return;
    }

    INIT_LIST_HEAD(&free_page_list);
    INIT_LIST_HEAD(&used_page_list);

    total_page = start_addr;
    total_page->size = size;
}

void* get_fragment_page(uint32_t size) 
{
    mm_page *new_page = NULL;

    if(!list_empty(&free_page_list))
    {
        new_page = list_entry(&free_page_list.next,mm_page,ll);
        list_del(&new_page->ll);
    } else {
        //we should divide a new page to free list;
        int divide_size = PAGE_SIZE + sizeof(mm_page);

        if(total_page->size >= divide_size)
        {
            new_page = total_page;
            int full_page = total_page->size;
            //printf("get:start_page is  0x%x,divide_size is %x \n",total_page,divide_size);
            new_page->start_pa = total_page;
            new_page->size = divide_size;

            total_page = total_page->start_pa + divide_size;
            total_page->size = full_page - divide_size;
            //printf("get:total_page is  0x%x,size is %x \n",total_page,total_page->size);
        }
    }

    if(new_page != NULL) 
    {
        list_add(&new_page->ll,&used_page_list);
        //printf("new_page is  0x%x \n",new_page);
        //printf("sizeofmm page is %x \n",sizeof(mm_page));
        //printf("new_page + mm_page is  %x \n",((uint64_t)new_page + sizeof(mm_page)));
        //printf("wangsl,alloc page is %x \n",(new_page + sizeof(mm_page)));
        //printf("wangsl,alloc page2 is %d \n",new_page->start_pa + sizeof(mm_page));
        printf("frag malloc addr is %x \n",new_page->start_pa + sizeof(mm_page));
        return new_page->start_pa + sizeof(mm_page);
    }

    return NULL;
}

void free_fragment_page(addr_t page_addr)
{
    mm_page *del_page = page_addr - sizeof(mm_page);
    //printf("page_addr is %x,sizeof(mm_page) is %x \n",page_addr,sizeof(mm_page));
    //printf("del_page is %x \n",del_page);
    list_del(&del_page->ll);
    list_add(&del_page->ll,&free_page_list);
}

void fragment_allocator_dump()
{
    struct list_head *p;
    int index = 0;
    printf("============free page============\n");
    list_for_each(p,&free_page_list) {
        mm_page *page = list_entry(p,mm_page,ll);
        printf("   %d: page size is %x \n",index,page->size);
        printf("   %d: page start_pa is %x,addr is %x \n",index,page->start_pa,page);
        index++;
    }

    printf("============used page============\n");
    list_for_each(p,&used_page_list) {
        mm_page *page = list_entry(p,mm_page,ll);
        printf("   %d: page size is %x \n",index,page->size);
        printf("   %d: page start_pa is %x,addr is %x \n",index,page->start_pa,page);
        index++;
    }
}

#if 0
int main()
{
    data = malloc(1024*1024*7l);
    fk_allocator_init(data,1024*1024*7l);

    page1 = get_4k_page();
    
    page2 = get_4k_page();
    page3 = get_4k_page();
    page4 = get_4k_page();

   // dump();
   printf("mm page is %d \n",sizeof(mm_page));
   free_4k_page(page1);
   free_4k_page(page2);
   free_4k_page(page3);
   free_4k_page(page4);
   dump();

}
#endif



