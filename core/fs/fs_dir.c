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
    inode *parent;
    char *create_dir = file_path_match(pathname,&find_part,&parent);

    if(create_dir == NULL)
    {
        return -1;
    }

    int inode_no = scan_bit_condition(find_part->node_bitmap,NODE_UNUSED,MAX_FILES_PER_PART);
    set_bit(find_part->node_bitmap,inode_no,NODE_USED);
    *partition = find_part;
    inode *select_node = &find_part->inode_table[inode_no];
    kmemset(select_node,0,sizeof(inode));

    select_node->inode_no = inode_no;
    select_node->parent_no = parent->inode_no;
    select_node->file.inode_no = inode_no;
    select_node->file.type = FT_DIRECTORY;
    select_node->status = NODE_USED;
    kmemcpy(select_node->file.name,create_dir,kstrlen(create_dir) + 1);
    INIT_LIST_HEAD(&select_node->child_list);
    list_add(&select_node->parent_ll,&parent->child_list);

end:
    free(create_dir);
    return inode_no;
}
