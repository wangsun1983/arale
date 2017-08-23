#include "hdd.h"
#include "fs.h"
#include "ctype.h"
#include "fs_utils.h"
#include "list.h"
#include "fs_dir.h"
#include "mm.h"

partition_data* dir_create(const char *pathname)
{
    int length = get_path_depth(pathname);
    int find_index = 0;
    int freecount = 0;
    //char **path = (char **)kmalloc(length);
    char *path[length];
    partition_data *find_part = NULL;

    split_path(pathname,path);

    char *root = path[0];
    inode *dir = (inode *)kmalloc(sizeof(inode));

    //we should find the dir's root
    list_head *p;
    //kprintf("dir_create trace1 pathname is %s,length is %d \n",pathname,length);
    list_for_each(p,&partition_list) {
        find_part = list_entry(p,partition_data,ll);
        inode *inode_table = find_part->inode_table;
        inode *parent = &inode_table[0];

        if(inode_table != NULL
          && kstrcmp(inode_table[0].file.name,root) == 0)
        {
             //kprintf("find hit,inode_table[0] is %s,root is %s \n",inode_table[0].file.name,root);
             find_index++;
             //last one is the directory which need creat
             while(find_index < length - 1)
             {
                 bool is_find = false;
                 struct list_head *node;
                 list_for_each(node,&parent->child_list) {
                     inode *find_node = list_entry(p,inode,parent_ll);
                     if(kstrcmp(find_node->file.name,path[find_index]) == 0)
                     {
                         parent = find_node;
                         find_index ++;
                         is_find = true;
                         break;
                     }
                 }

                 if(!is_find) {
                     //kprintf("dir_create,dir not found \n");
                     find_part = NULL;
                     goto end;
                 }
             }

             int inode_no = scan_bit_condition(find_part->node_bitmap,NODE_UNUSED,MAX_FILES_PER_PART);
             set_bit(find_part->node_bitmap,inode_no,NODE_USED);
             //kprintf("find trace1,inode_no is %d \n",inode_no);
             inode *select_node = &find_part->inode_table[inode_no];
             kmemset(select_node,0,sizeof(inode));

             select_node->parent_no = parent->inode_no;
             select_node->file.inode_no = inode_no;
             select_node->file.type = FT_DIRECTORY;
             select_node->status = NODE_USED;
             kmemcpy(select_node->file.name,path[length - 1],kstrlen(path[length - 1]) + 1);

             INIT_LIST_HEAD(&select_node->child_list);
             list_add(&select_node->parent_ll,&parent->child_list);
             //kprintf("find trace3,parent file is %s,select file is %s \n",parent->file.name,select_node->file.name);
        }

    }

end:
    for(;freecount < length;freecount++)
    {
        char *freedata = path[freecount];
        free(freedata);
    }

    return find_part;
}
