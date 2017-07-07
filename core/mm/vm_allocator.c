

#include "ctype.h"
#include "rbtree.h"
#include "vmm.h"


static void insert_free_node(vm_node *node, vm_root *vmroot)
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

static void insert_used_node(vm_node *node, vm_root *vmroot)
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

void erase_free_node(vm_node *node, vm_root *vmroot)
{
    rb_erase(&node->rb, &vmroot->free_root);
}

void erase_used_node(vm_node *node, vm_root *vmroot)
{
    rb_erase(&node->rb, &vmroot->used_root);
}

void switch_process(uint64_t start_addr,uint32_t size) 
{


}

vm_root * vm_init(uint64_t start_addr,uint32_t size)
{
    vm_root *root = (vm_root *)malloc(sizeof(vm_root));
    root->start_va = start_addr;
    root->size = size;
    INIT_LIST_HEAD(&root->used_nodes);
    
    //the first node is full virtual memory.haha
    vm_node *node = (vm_node *)malloc(sizeof(vm_node));
    node->page_num = size/PAGE_SIZE;
    node->start_va = start_addr;
    node->end_va = start_addr + size;
    insert_free_node(node,root);

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
            printf("hint,addr is %x \n",addr);
            erase_used_node(node,vmroot);
            RB_CLEAR_NODE(&node->rb);
            //insert_free_node(node,vmroot);
            return;
        }

        if (start_va < rb_entry(parent, struct vm_node, rb)->start_va)
            new = &parent->rb_left;
        else
            new = &parent->rb_right;
    }

}

uint64_t vm_alloc(uint32_t size,vm_root *vmroot)
{
    int page_num = size/PAGE_SIZE + 1;
    printf("vm_allocator trace need page_num is %x \n",page_num);
    //start find pages~~~~.
    struct rb_node **new = &vmroot->free_root.rb_node, *parent = NULL;
    //uint32_t page_num = node->page_num;

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
        printf("vm_allocator selected page_num is %x \n",select->page_num);
        //remove select node;
        erase_free_node(&select->rb,&vmroot->free_root.rb_node);

        //start splite this node
        vm_node *new_node = (vm_node *)malloc(sizeof(vm_node));
        new_node->start_va = select->start_va;
        new_node->end_va = new_node->start_va + page_num*PAGE_SIZE - 1;
        new_node->page_num = page_num;

        select->start_va = new_node->end_va + 1;
        select->page_num -= page_num;

        //re_insert node.
        insert_free_node(select,vmroot);
        insert_used_node(new_node,vmroot);
        //add_used_nodes(new_node,vmroot);
        //used node should insert to used list~~~~.
        return new_node->start_va;
    }
}

void add_used_nodes(vm_node *node, vm_root *vmroot)
{
    list_head *p;
    list_head *head = &vmroot->used_nodes;
    list_head *add_head = head;

    list_for_each(p,head) {
         vm_node *node = list_entry(p,vm_node,ll);
         add_head = p;
         //printf("list_add 2: new_page.ll is %x, page.ll is %x head is %x\n",&new_page->ll,&page->ll,head);
         if(node->start_va < node->end_va ) {
              add_head = p->prev;
              break;     
         }
    }

    list_add(&node->ll,add_head);
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
        printf("node page_num is %d start_va is %x \n",node->page_num,node->start_va);
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

#if 0
int main()
{
    char *test = malloc(1024*1024*7l);
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

    printf("f1 is 0x%x \n",f1);
    printf("f2 is 0x%x \n",f2);
    printf("f3 is 0x%x \n",f3);
    printf("f4 is 0x%x \n",f4);
    printf("f5 is 0x%x \n",f5);
    printf("f6 is 0x%x \n",f6);
    printf("f7 is 0x%x \n",f7);
    printf("f8 is 0x%x \n",f8);
    printf("f9 is 0x%x \n",f9);

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
 
    struct rb_node **new2 = &vmroot->used_root.rb_node;
    struct rb_node *parent2;
    parent2 = *new2;
    vm_node *node2 = rb_entry(parent2, vm_node, rb);

    struct dump_data *list2 = malloc(sizeof(struct dump_data));
    list2->node = node2;
    dump(list2);

}
#endif
