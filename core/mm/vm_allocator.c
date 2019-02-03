/**************************************************************
 CopyRight     :No
 FileName      :vm_allocator.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :virtual memory allocator
 History       :
 20190203      1.rewrite virtual memory alloc al
***************************************************************/


#include "ctype.h"
#include "rbtree.h"
#include "vmm.h"
#include "vm_allocator.h"
#include "mm_common.h"
#include "log.h"

#define MAX_FREE_FRAGMENT 16

static void rb_insert_free_node(vm_node *node, vm_root *vmroot)
{
    struct rb_node **new = &vmroot->free_root.rb_node, *parent = NULL;
    uint32_t page_num = node->page_num;

    while (*new)
    {
        parent = *new;
        if (page_num < rb_entry(parent, struct vm_node, rb)->page_num)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }

    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, &vmroot->free_root);
}

static void rb_insert_used_node(vm_node *node, vm_root *vmroot)
{
    struct rb_node **new = &vmroot->used_root.rb_node, *parent = NULL;
    addr_t start_va = node->start_va;

    while (*new)
    {
        parent = *new;
        if (start_va < rb_entry(parent, struct vm_node, rb)->start_va)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }

    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, &vmroot->used_root);
}

void rb_erase_free_node(vm_node *node, vm_root *vmroot)
{
    rb_erase(&node->rb, &vmroot->free_root);
}

void rb_erase_used_node(vm_node *node, vm_root *vmroot)
{
    rb_erase(&node->rb, &vmroot->used_root);
}

void add_free_fragments_nodes(vm_node *node, vm_root *vmroot)
{
    list_head *p;
    list_head *head = &vmroot->free_nodes;
    list_head *add_head = head;

    list_for_each(p,head) {
         vm_node *list_node = list_entry(p,vm_node,ll);
         add_head = p;
         if(node->start_va < list_node->start_va ) {
              add_head = p->prev;
              break;
         }
    }

    list_add(&node->ll,add_head);
    vmroot->free_fragments++;
}


void remove_free_fragments_nodes(vm_node *node,vm_root *vmroot)
{
    list_del(&node->ll);
    vmroot->free_fragments--;
}

void switch_process(addr_t start_addr,uint32_t size)
{
    //TODO
}

vm_root * vm_allocator_init(addr_t start_addr,uint32_t size)
{
    //LOGD("vm_allocator_init trace1 \n");
    vm_root *root = (vm_root *)kmalloc(sizeof(vm_root));
    //LOGD("vm_allocator_init trace1_1 \n");
    kmemset(root,0,sizeof(vm_root));
    //LOGD("vm_allocator_init trace1_2 \n");
    root->start_va = start_addr;
    root->size = size;
    INIT_LIST_HEAD(&root->free_nodes);
    //LOGD("vm_allocator_init trace2 \n");
    //the first node is full virtual memory.
    vm_node *node = (vm_node *)kmalloc(sizeof(vm_node));
    kmemset(node,0,sizeof(vm_node));

    node->page_num = size/PAGE_SIZE;
    node->start_va = start_addr;
    node->end_va = start_addr + size;
    rb_insert_free_node(node,root);
    //LOGD("vm_allocator_init trace3 \n");
    //add to free list.......to large
    add_free_fragments_nodes(node,root);

    return root;

}

int vm_allocator_free(addr_t addr,vm_root *vmroot)
{
    struct rb_node **new = &vmroot->used_root.rb_node, *parent = NULL;
    addr_t start_va = addr;
    int freePageNum = 0;
    
    while (*new)
    {
        parent = *new;
        vm_node *node = rb_entry(parent, struct vm_node, rb);
        if(node->start_va == addr)
        {
            freePageNum = node->page_num;
            LOGD("wangsl,vm_allocator_free start,node->page_num is %d \n",node->page_num);
            rb_erase_used_node(node,vmroot);
            RB_CLEAR_NODE(&node->rb);
            rb_insert_free_node(node,vmroot);
            add_free_fragments_nodes(node,vmroot);
            break;
        }

        if (start_va < rb_entry(parent, struct vm_node, rb)->start_va)
        {
            new = &parent->rb_left;
        }
        else
        {
            new = &parent->rb_right;
        }
    }

    if(vmroot->free_fragments > MAX_FREE_FRAGMENT)
    {
        LOGD("wangsl,vm_allocator_free scan \n");
        vm_scan_merge(vmroot);
    }
    LOGD("wangsl,vm_allocator_free end \n");
    return freePageNum;

}

/*
* get virtual memory
*/
addr_t vm_allocator_alloc(uint32_t size,vm_root *vmroot)
{
    int page_num = PAGE_SIZE_RUND_UP(size)/PAGE_SIZE;
    LOGD("page num is %d,size is %d \n",page_num,size);
    //start find pages.
    struct rb_node **new = &vmroot->free_root.rb_node, *parent = NULL;

    vm_node *select = NULL;

    while (*new) {
        parent = *new;
        vm_node *vmnode = rb_entry(parent, struct vm_node, rb);
        if(vmnode->page_num >= page_num)
        {
            select = vmnode;
            new = &parent->rb_left;
        }
        else
        {
            new = &parent->rb_right;
        }
    }

    if(select)
    {
        //remove select node;
        rb_erase_free_node(select,vmroot);

        //remove node from list.
        remove_free_fragments_nodes(select,vmroot);

        //start splite this node
        vm_node *new_node = (vm_node *)kmalloc(sizeof(vm_node));
        kmemset(new_node,0,sizeof(vm_node));

        new_node->start_va = select->start_va;
        new_node->end_va = new_node->start_va + page_num*PAGE_SIZE;
        new_node->page_num = page_num;

        select->start_va = new_node->end_va + 1;
        select->page_num -= page_num;

        //re_insert node.
        rb_insert_free_node(select,vmroot);
        rb_insert_used_node(new_node,vmroot);

        //we should also insert free list.
        add_free_fragments_nodes(select,vmroot);
        return new_node->start_va;
    }

    return NULL;
}

/*
* scan free list and try merge to one block.
*/
void vm_scan_merge(vm_root *vmroot)
{
    struct list_head *p;
    vm_node *merge_start = NULL;
    vm_node *merge_end = NULL;

    list_for_each(p,&vmroot->free_nodes) {
        vm_node *node = list_entry(p,vm_node,ll);
        //mark scan start tag
        if(merge_start == NULL)
        {
            merge_start = node;
            continue;
        }

        //mark scan end tag
        if(merge_start->end_va + 1 == node->start_va)
        {
            merge_end = node;
        }
        else
        {
            //start merge from merge_start to merge_end
            struct list_head *cursor = &merge_start->ll;
            int totalsize = 0;
            int page_num = 0;
            //if merge_end is not null,we should merge free memory to one block
            if(merge_end != NULL) {
                while(cursor != &merge_end->ll)
                {
                    vm_node *rm_node = list_entry(cursor,vm_node,ll);
                    list_del(cursor);
                    rb_erase_free_node(rm_node,vmroot);
                    cursor = cursor->next;
                    if(rm_node != merge_start) {
                        totalsize += (rm_node->end_va - rm_node->start_va);
                        page_num += rm_node->page_num;
                        free(rm_node);
                    }
                }
                merge_start->page_num += page_num;
                merge_start->end_va += totalsize;
                rb_insert_free_node(merge_start,vmroot);
            }
            else
            {
                list_del(cursor);
            }
            
            merge_start = node;
            merge_end = NULL;
        }
    }
}

//wangsl
/*
void old_vm_scan_merge(vm_root *vmroot)
{
    struct list_head *p;
    vm_node *merge_start = NULL;
    vm_node *merge_end = NULL;
    int fragement = 0;

    list_for_each(p,&vmroot->free_nodes) {

        vm_node *node = list_entry(p,vm_node,ll);

        if(node->ll.next != NULL && node->ll.next != &vmroot->free_nodes)
        {
            vm_node *next = list_entry(node->ll.next,vm_node,ll);
            //check whether it can be merged

            if(node->end_va + 1 == next->start_va)
            {
                if(merge_start == NULL)
                {
                    merge_start = node;
                    fragement++;
                }

                merge_end = next;
                fragement++;
            }
        }

        if(merge_end != NULL && p == &merge_end->ll)
        {
            merge_end->start_va = merge_start->start_va;
            merge_end->page_num = (merge_end->end_va - merge_end->start_va)/PAGE_SIZE;

            list_del_range(merge_start->ll.prev,p);
            //special
            vmroot->free_fragments -= (fragement-1); //because we reuse merge_end,so free_fragments should minus 1

            struct list_head *cursor = &merge_start->ll;
            while(cursor != &merge_end->ll)
            {
                vm_node *rm_node = list_entry(cursor,vm_node,ll);
                rb_erase_free_node(rm_node,vmroot);
                cursor = cursor->next;
                //free(rm_node); TODO free!!!!!!
            }

            rb_erase_free_node(merge_end,vmroot);
            rb_insert_free_node(merge_end,vmroot);

            merge_start == NULL;
            merge_end == NULL;
            fragement = 0;
        }
    }

}
*/

struct vm_dump_data {
    vm_node *node;
    struct vm_dump_data *next;
};

void vm_dump(struct vm_dump_data *list)
{
    struct vm_dump_data *hh = list;
    struct vm_dump_data *next_list = NULL;
    struct vm_dump_data *next_list_tail = next_list;

    while(hh != NULL)
    {
        vm_node *node = hh->node;
        //LOGD("node page_num is %d start_va is %llx \n",node->page_num,node->start_va);
        if(node->rb.rb_left)
        {
            vm_node *child = rb_entry(node->rb.rb_left, vm_node, rb);

            if(next_list == NULL)
            {
                next_list = kmalloc(sizeof(struct vm_dump_data));
                next_list->node = child;
                next_list_tail = next_list;
            }
            else
            {
                struct vm_dump_data *new_data = kmalloc(sizeof(struct vm_dump_data));
                new_data->node = child;
                next_list_tail->next = new_data;
                next_list_tail = new_data;
            }
        }

        if(node->rb.rb_right)
        {
            vm_node *child = rb_entry(node->rb.rb_right, vm_node, rb);

            if(next_list == NULL)
            {
                next_list = kmalloc(sizeof(struct vm_dump_data));
                next_list->node = child;
                next_list_tail = next_list;
            }
            else
            {
                struct vm_dump_data *new_data = kmalloc(sizeof(struct vm_dump_data));
                new_data->node = child;
                next_list_tail->next = new_data;
                next_list_tail = new_data;
            }
        }

        hh = hh->next;
    }

    if(next_list != NULL) {
        vm_dump(next_list);
    }
}

void dump_free_list(vm_root *vmroot)
{
    struct list_head *p;
    int index = 0;
    list_for_each(p,&vmroot->free_nodes) {
        vm_node *node = list_entry(p,vm_node,ll);
        //LOGD("   %d: start_va is %llx,end_va is %llx,page_num is %d \n",index,node->start_va,node->end_va,node->page_num);
        index++;
    }
}
