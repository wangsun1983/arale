#include "hdd.h"
#include "fs.h"
#include "ctype.h"
#include "fs_utils.h"
#include "list.h"
#include "fs_dir.h"
#include "mm.h"

int dir_create(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    //kprintf("dir_create trace1 \n");
    inode *parent;
    inode *select_node;
    //char *create_dir = file_path_match(pathname,&find_part,&parent,&select_node);
    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);
    if(result == MATCH_PARENT_NOT_FOUND
      || result == MATCH_SAME_FILE
      || result == MATCH_FILE_NAME_OVERFLOW)
    {
        return  -1;
    }

    char *create_dir = (char *)result;
    int inode_no = scan_bit_condition(find_part->node_bitmap,NODE_UNUSED,MAX_FILES_PER_PART);
    set_bit(find_part->node_bitmap,inode_no,NODE_USED);
    *partition = find_part;
    select_node = &find_part->inode_table[inode_no];
    kmemset(select_node,0,sizeof(inode));
    //kprintf("dir_create trace3 \n");

    select_node->inode_no = inode_no;
    //kprintf("dir_create parent is %x,parent->inode_no is %d \n",parent,parent->inode_no);
    select_node->parent_no = parent->inode_no;
    select_node->file.inode_no = inode_no;
    select_node->file.type = FT_DIRECTORY;
    select_node->status = NODE_USED;
    kmemcpy(select_node->file.name,create_dir,kstrlen(create_dir) + 1);
    INIT_LIST_HEAD(&select_node->child_list);
    list_add(&select_node->parent_ll,&parent->child_list);
    //kprintf("dir_create trace4 \n");

end:
    free(create_dir);
    //kprintf("dir_create trace5 \n");
    return inode_no;
}

int dir_remove(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node = NULL;
    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);

    if(result == MATCH_FILE_NAME_OVERFLOW
      ||result != MATCH_SAME_FILE)
    {
        return -1;
    }

    if(select_node != NULL)
    {
        //we should check whether there are files stored in this directory.
        if(list_empty(&select_node->child_list))
        {
            return REMOVE_FAIL_FILE_EXIT;
        }

        int ret_inode_no = select_node->inode_no;
        set_bit(find_part->node_bitmap,select_node->inode_no,NODE_UNUSED);
        *partition = find_part;
        list_del(&select_node->parent_ll);
        kmemset(select_node,0,sizeof(inode));
        return ret_inode_no;
    }

    return -1;
}

int dir_rename(const char *pathname,char *newname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node = NULL;

    if(kstrlen(newname) >= MAX_FILE_NAME_LEN)
    {
        return -1;
    }

    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);
    if(result == MATCH_FILE_NAME_OVERFLOW
      ||result != MATCH_SAME_FILE)
    {
        return -1;
    }

    if(select_node != NULL)
    {
        int ret_inode_no = select_node->inode_no;
        kmemset(select_node->file.name,0,MAX_FILE_NAME_LEN);
        kmemcpy(select_node->file.name,newname,kstrlen(newname) + 1);
        return ret_inode_no;
    }

    return -1;
}
