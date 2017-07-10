#include <stdio.h>

#include "ctype.h"
#include "rbtree.h"
#include "vmm.h"

#define MAX_FREE_FRAGMENT 5


static void rb_insert_free_node(vm_node *node, vm_root *vmroot)
{
    struct rb_node **new = &vmroot->free_root.rb_node, *parent = NULL;
    uint32_t page_num = node->page_num;
    printf("insert_free_node node is %llx \n",node);

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
    uint64_t start_va = node->start_va;

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

void switch_process(uint64_t start_addr,uint32_t size) 
{


}

vm_root * vm_init(uint64_t start_addr,uint32_t size)
{
    vm_root *root = (vm_root *)malloc(sizeof(vm_root));
    memset(root,0,sizeof(vm_root));

    root->start_va = start_addr;
    root->size = size;
    INIT_LIST_HEAD(&root->free_nodes);
    
    //the first node is full virtual memory.haha
    vm_node *node = (vm_node *)malloc(sizeof(vm_node));
    printf("malloc node trace1,node is %llx \n",node);
    memset(node,0,sizeof(vm_node));

    node->page_num = size/PAGE_SIZE;
    node->start_va = start_addr;
    node->end_va = start_addr + size;
    rb_insert_free_node(node,root);

    //add to free list.......to large
    add_free_fragments_nodes(node,root);

    printf("vm_init dumplist,node is %llx \n",node);
    //dump_free_list(root);

    return root;
}

void vm_free(uint64_t addr,vm_root *vmroot)
{
    struct rb_node **new = &vmroot->used_root.rb_node, *parent = NULL;
    uint64_t start_va = addr;

    while (*new) 
    {
        parent = *new;
        vm_node *node = rb_entry(parent, struct vm_node, rb);
        if(node->start_va == addr)
        {
            printf("hint,addr is %llx \n",addr);
            rb_erase_used_node(node,vmroot);
            RB_CLEAR_NODE(&node->rb);
            rb_insert_free_node(node,vmroot);
            add_free_fragments_nodes(node,vmroot);
            break;
        }

        if (start_va < rb_entry(parent, struct vm_node, rb)->start_va)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }

    if(vmroot->free_fragments > MAX_FREE_FRAGMENT) 
    {
        printf("wo qu,scan la!!!!!vmroot->free_fragments is %d\n",vmroot->free_fragments);
        vm_scan_merge(vmroot);
    }

}

uint64_t vm_alloc(uint32_t size,vm_root *vmroot)
{
    int page_num = size/PAGE_SIZE + 1;
    printf("vm_allocator trace need page_num is %d \n",page_num);
    //start find pages~~~~.
    struct rb_node **new = &vmroot->free_root.rb_node, *parent = NULL;
    //uint32_t page_num = node->page_num;

    vm_node *select = NULL;

    while (*new) {
        parent = *new;
        vm_node *vmnode = rb_entry(parent, struct vm_node, rb);
        if(vmnode->page_num >= page_num) 
        { 
            //printf("vm_allocator trace2 vmnode is %x\n",vmnode);
            select = vmnode;
            new = &parent->rb_left;
        }
        else 
        {
            printf("vm_allocator trace3 \n");
            new = &parent->rb_right;
        }
    }

    printf("vm_allocator trace2 \n");
    if(select) 
    {
        printf("vm_allocator selected page_num is %llx,select is %llx \n",select->page_num,select);
        //remove select node;
        rb_erase_free_node(select,vmroot);

        //remove node from list.
        remove_free_fragments_nodes(select,vmroot);

        //start splite this node
        vm_node *new_node = (vm_node *)malloc(sizeof(vm_node));
        memset(new_node,0,sizeof(vm_node));
        printf("malloc node trace2 new_node is %llx \n",new_node);
        new_node->start_va = select->start_va;
        new_node->end_va = new_node->start_va + page_num*PAGE_SIZE - 1;
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
}


void vm_scan_merge(vm_root *vmroot)
{
    struct list_head *p;
    vm_node *merge_start = NULL;
    vm_node *merge_end = NULL;
    int fragement = 0;

    printf("==================vm_scan_merge start================= \n");
    dump_free_list(vmroot);
     
    list_for_each(p,&vmroot->free_nodes) {

        vm_node *node = list_entry(p,vm_node,ll);
        printf("trace1:vm_scan_merge node->start_va is %llx,node->page_num is %d \n",node->start_va,node->page_num);
        if(node->ll.next != NULL && node->ll.next != &vmroot->free_nodes)
        {
            vm_node *next = list_entry(node->ll.next,vm_node,ll);
            //check whether it can be merged
            printf("trace2:vm_scan_merge next->start_va is %llx,next->end_va is %llx,next->page_num is %d \n",
                     next->start_va,next->end_va,next->page_num);

            if(node->end_va + 1 == next->start_va) 
            {
                printf("trace3:node->start_va is %llx,next->start_va is %llx \n",node->start_va,next->start_va);

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
            printf("trace4:merge_start is %llx \n",merge_start);
            merge_end->start_va = merge_start->start_va;
            merge_end->page_num = (merge_end->end_va - merge_end->start_va)/PAGE_SIZE;

            list_del_range(merge_start->ll.prev,p);
            //special
            vmroot->free_fragments -= (fragement-1); //because we reuse merge_end,so free_fragments should minus 1

            printf("trace5:merge_start is %llx merge_end is %llx \n",merge_start->start_va,merge_end->start_va);

            struct list_head *cursor = &merge_start->ll;
            while(cursor != &merge_end->ll) 
            {
                vm_node *rm_node = list_entry(cursor,vm_node,ll);
                rb_erase_free_node(rm_node,vmroot);
                cursor = cursor->next;
                free(rm_node);
            }

            printf("trace6:merge_start is %llx \n",merge_start);
            rb_erase_free_node(merge_end,vmroot);
            rb_insert_free_node(merge_end,vmroot);

            merge_start == NULL;
            merge_end == NULL;
            fragement = 0;
        }       
    }

}


struct dump_data {
    vm_node *node;
    struct dump_data *next;
};

void dump(struct dump_data *list)
{
    struct dump_data *hh = list;
    struct dump_data *next_list = NULL;
    struct dump_data *next_list_tail = next_list;

    while(hh != NULL)
    {
        vm_node *node = hh->node; 
        printf("node page_num is %d start_va is %llx \n",node->page_num,node->start_va);
        if(node->rb.rb_left)
        {
            vm_node *child = rb_entry(node->rb.rb_left, vm_node, rb);

            if(next_list == NULL)
            {
                next_list = malloc(sizeof(struct dump_data));
                next_list->node = child;
                next_list_tail = next_list;
            } 
            else 
            {
                struct dump_data *new_data = malloc(sizeof(struct dump_data));
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
                next_list = malloc(sizeof(struct dump_data));
                next_list->node = child;
                next_list_tail = next_list;
            } 
            else 
            {
                struct dump_data *new_data = malloc(sizeof(struct dump_data));
                new_data->node = child;
                next_list_tail->next = new_data;
                next_list_tail = new_data;
            }
        }
        
        hh = hh->next;
    }

    if(next_list != NULL) {
        dump(next_list);
    }
}

void dump_free_list(vm_root *vmroot)
{
    struct list_head *p;
    int index = 0;
    list_for_each(p,&vmroot->free_nodes) {
        vm_node *node = list_entry(p,vm_node,ll);
        printf("   %d: start_va is %llx,end_va is %llx,page_num is %d \n",index,node->start_va,node->end_va,node->page_num);
        index++;
    }
}


int main()
{
    char *test = malloc(1024*1024*7l);
    memset(test,0,1024*1024*7l);

    vm_root *vmroot = vm_init(test,1024*1024*7l);
    

    char *f1 = vm_alloc(1024*4+8,vmroot);   
    char *f2 = vm_alloc(1024*7+8,vmroot);
    char *f3 = vm_alloc(1024*9+8,vmroot);
    char *f4 = vm_alloc(1024*12+8,vmroot);
    char *f5 = vm_alloc(1024*24+8,vmroot);
    char *f6 = vm_alloc(1024*77+8,vmroot);
    char *f7 = vm_alloc(1024*90+8,vmroot);
    char *f8 = vm_alloc(1024*36+8,vmroot);
    char *f9 = vm_alloc(8,vmroot);

    printf("f1 is 0x%llx \n",f1);
    printf("f2 is 0x%llx \n",f2);
    printf("f3 is 0x%llx \n",f3);
    printf("f4 is 0x%llx \n",f4);
    printf("f5 is 0x%llx \n",f5);
    printf("f6 is 0x%llx \n",f6);
    printf("f7 is 0x%llx \n",f7);
    printf("f8 is 0x%llx \n",f8);
    printf("f9 is 0x%llx \n",f9);

    struct rb_node **new = &vmroot->used_root.rb_node;
    struct rb_node *parent;
    parent = *new;
    vm_node *node = rb_entry(parent, vm_node, rb);

    struct dump_data *list = malloc(sizeof(struct dump_data));
    list->node = node;
    printf("===================1==================== \n");
    dump(list);

    vm_free(f1,vmroot);
    vm_free(f2,vmroot);
    vm_free(f3,vmroot);
    vm_free(f4,vmroot);
    vm_free(f5,vmroot);
    vm_free(f6,vmroot);
    vm_free(f7,vmroot);
    vm_free(f8,vmroot);
    vm_free(f9,vmroot);
    printf("===================2==================== \n");
    printf("2:root fragment is %d \n",vmroot->free_fragments);

    struct rb_node **new2 = &vmroot->used_root.rb_node;
    struct rb_node *parent2;
    parent2 = *new2;
    vm_node *node2 = rb_entry(parent2, vm_node, rb);

    if(!RB_EMPTY_ROOT(&vmroot->used_root))
    {
        struct dump_data *list2 = malloc(sizeof(struct dump_data));
        list2->node = node2;
        dump(list2);
    }

    printf("===================3==================== \n");
    printf("3:root fragment is %d \n",vmroot->free_fragments);
    struct rb_node **new3 = &vmroot->free_root.rb_node;
    struct rb_node *parent3;
    parent3 = *new3;
    vm_node *node3 = rb_entry(parent3, vm_node, rb);

    if(!RB_EMPTY_ROOT(&vmroot->free_root))
    {
        struct dump_data *list3 = malloc(sizeof(struct dump_data));
        list3->node = node3;
        dump(list3);
    }

    printf("===================4==================== \n");
    vm_scan_merge(vmroot); 

    printf("===================5==================== \n");
    dump_free_list(vmroot);

    printf("5:root fragment is %d \n",vmroot->free_fragments);



}
