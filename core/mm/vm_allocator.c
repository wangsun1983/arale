#include "ctype.h"
#include "rbtree.h"
#include "vmm.h"

#define MAX_FREE_FRAGMENT 5

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
    printf("wangsl,rb_erase_free_node start \n");
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
    //printf("vm_allocator_init start_addr is %x,size is %x \n",start_addr,size);

    vm_root *root = (vm_root *)kmalloc(sizeof(vm_root));

    memset(root,0,sizeof(vm_root));

    root->start_va = start_addr;
    root->size = size;
    INIT_LIST_HEAD(&root->free_nodes);
    

    //the first node is full virtual memory.haha
    //printf("vm_allocator_init start \n");
    vm_node *node = (vm_node *)kmalloc(sizeof(vm_node));
    memset(node,0,sizeof(vm_node));
    //printf("vm_allocator_init node is %d \n",node);

    printf("vm_allocator_init root is %x,node is %x \n",root,node);
    
    node->page_num = size/PAGE_SIZE;
    node->start_va = start_addr;
    node->end_va = start_addr + size;
    rb_insert_free_node(node,root);
    //printf("vm_allocator_init node->page_num is %x \n",node->page_num);
    //add to free list.......to large
    add_free_fragments_nodes(node,root);
    //dump_free_list(root);
    //printf("vm_allocator_init trace3");
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
        vm_scan_merge(vmroot);
    }

    return freePageNum;

}

addr_t vm_allocator_alloc(uint32_t size,vm_root *vmroot)
{
    int page_num = size/PAGE_SIZE + 1;
    //start find pages~~~~.
    struct rb_node **new = &vmroot->free_root.rb_node, *parent = NULL;
    //uint32_t page_num = node->page_num;
    //printf("wangsl,vm_allocator_alloc start \n");
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
            new = &parent->rb_right;
        }
    }
    
    printf("vm_allocator_alloc select is %x \n",select);
    if(select) 
    {
        //printf("wangsl,vm_allocator_alloc trace1,vmroot is %x \n",vmroot);
        //remove select node;
        rb_erase_free_node(select,vmroot);
        //printf("wangsl,vm_allocator_alloc trace2 \n");
        //remove node from list.
        remove_free_fragments_nodes(select,vmroot);
        //printf("wangsl,vm_allocator_alloc trace3 \n");
        //start splite this node
        vm_node *new_node = (vm_node *)kmalloc(sizeof(vm_node));
        memset(new_node,0,sizeof(vm_node));
        //printf("wangsl,vm_allocator_alloc trace4 \n");
        new_node->start_va = select->start_va;
        new_node->end_va = new_node->start_va + page_num*PAGE_SIZE - 1;
        new_node->page_num = page_num;

        select->start_va = new_node->end_va + 1;
        select->page_num -= page_num;

        //re_insert node.
        //printf("wangsl,vm_allocator_alloc trace5 \n");
        rb_insert_free_node(select,vmroot);
        rb_insert_used_node(new_node,vmroot);
        //printf("wangsl,vm_allocator_alloc trace6 \n");
        //we should also insert free list.
        add_free_fragments_nodes(select,vmroot);
        //printf("wangsl,vm_allocator_alloc trace5 \n");
        //printf("wangsl,vm_allocator_alloc trace7 \n");
        return new_node->start_va;
    }
}


void vm_scan_merge(vm_root *vmroot)
{
    struct list_head *p;
    vm_node *merge_start = NULL;
    vm_node *merge_end = NULL;
    int fragement = 0;

    //dump_free_list(vmroot);
     
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
        //printf("node page_num is %d start_va is %llx \n",node->page_num,node->start_va);
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
        dump(next_list);
    }
}

void dump_free_list(vm_root *vmroot)
{
    struct list_head *p;
    int index = 0;
    list_for_each(p,&vmroot->free_nodes) {
        vm_node *node = list_entry(p,vm_node,ll);
        //printf("   %d: start_va is %llx,end_va is %llx,page_num is %d \n",index,node->start_va,node->end_va,node->page_num);
        index++;
    }
}

