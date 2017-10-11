/**************************************************************
 CopyRight     :No
 FileName      :fragment_allocator.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :4K memory allocator
***************************************************************/

#include "page.h"
#include "mmzone.h"
#include "list.h"
#include "mm.h"
#include "vmm.h"
#include "log.h"

/*----------------------------------------------
                local struct
----------------------------------------------*/
//because vmalloc can use uncontinous memory,
//so we can alloc 4K page
typedef struct fragment_node
{
    struct list_head ll;
    struct rb_node rb;
    mm_page page;
} fragment_node;

/*----------------------------------------------
                local data
----------------------------------------------*/
private mm_zone high_memory_zone;
private mm_page total_page;
private struct list_head free_page_list;
private struct rb_root used_page_root;

/*----------------------------------------------
                local method
----------------------------------------------*/
private void rb_insert_used_node(fragment_node *node, struct rb_root *root);
private fragment_node *rb_find_node(addr_t start_addr,struct rb_root *root);
private void rb_erase_used_fragment(fragment_node *node,struct rb_root *root);

/*----------------------------------------------
                public method
----------------------------------------------*/
public void fragment_allocator_init(addr_t start_addr,uint32_t size)
{

    if(size < PAGE_SIZE)
    {
        //LOGD("PAGE SIZE error!!!!! \n");
        return;
    }

    INIT_LIST_HEAD(&free_page_list);
    //INIT_LIST_HEAD(&used_page_list);

    total_page.start_pa = start_addr;
    total_page.size = size;
}

public void* get_fragment_page(uint32_t size,uint32_t *alloc_page)
{
    fragment_node *frag = NULL;
    *alloc_page = PAGE_SIZE;

    if(!list_empty(&free_page_list))
    {
        frag = list_entry(free_page_list.next,fragment_node,ll);
        list_del(&frag->ll);
    } else {
        //we should divide a new page to free list;
        frag = kmalloc(sizeof(fragment_node));

        uint32_t divide_size = PAGE_SIZE;

        if(total_page.size >= divide_size)
        {
            frag->page.start_pa = (uint64_t)total_page.start_pa;
            frag->page.size = divide_size;

            total_page.start_pa = (uint64_t)total_page.start_pa + divide_size;
            total_page.size -=divide_size;
        }
    }

    if(frag != NULL)
    {
        rb_insert_used_node(frag,&used_page_root);
        return (void *)frag->page.start_pa;
    }

    return NULL;
}

public int free_fragment_page(addr_t page_addr)
{
    //mm_page *del_page = page_addr - sizeof(mm_page);
    page_addr &= PAGE_MASK;

    fragment_node *node = rb_find_node(page_addr,&used_page_root);
    if(node == NULL)
    {
        LOGD("free_fragment_page error!!!!!\n");
        return -1;
    }

    rb_erase_used_fragment(node,&used_page_root);
    //list_del(&del_page->ll);
    list_add(&node->ll,&free_page_list);

    return 0;
}

/*----------------------------------------------
                private method
----------------------------------------------*/
private void rb_insert_used_node(fragment_node *node, struct rb_root *root)
{
    struct rb_node **new = &root->rb_node, *parent = NULL;
    addr_t start_pa = node->page.start_pa;

    while (*new)
    {
        parent = *new;
        if (start_pa < rb_entry(parent, fragment_node, rb)->page.start_pa)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }

    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, root);
}

private fragment_node *rb_find_node(addr_t start_addr,struct rb_root *root)
{
    struct rb_node **new = &root->rb_node, *parent = NULL;
    addr_t start_pa = start_addr;

    while (*new)
    {
        parent = *new;
        fragment_node *node = rb_entry(parent, fragment_node, rb);
        addr_t pa = node->page.start_pa;

        if (start_pa == pa)
        {
            //new = &parent->rb_left;
            return node;
        }
        else if(start_pa < pa)
        {
            new = &parent->rb_left;
        }
        else if(start_pa > pa)
        {
            new = &parent->rb_right;
        }
    }

    return NULL;
}

private void rb_erase_used_fragment(fragment_node *node,struct rb_root *root)
{
    rb_erase(&node->rb,root);
}

private void fragment_allocator_dump()
{
    struct list_head *p;
    int index = 0;
    LOGD("============free page============\n");
    list_for_each(p,&free_page_list) {
        mm_page *page = list_entry(p,mm_page,ll);
        LOGD("   %d: page size is %x \n",index,page->size);
        LOGD("   %d: page start_pa is %x,addr is %x \n",index,page->start_pa,page);
        index++;
    }

    //LOGD("============used page============\n");
    //list_for_each(p,&used_page_list) {
    //    mm_page *page = list_entry(p,mm_page,ll);
    //    LOGD("   %d: page size is %x \n",index,page->size);
    //    LOGD("   %d: page start_pa is %x,addr is %x \n",index,page->start_pa,page);
    //    index++;
    //}
}
