#include "fs_file.h"
#include "fs_utils.h"
#include "fs.h"
#include "mm.h"

//we only allocate inode when create file.
int file_create(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node;

    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);
    //kprintf("file_create result is %d \n",result);
    if(result == MATCH_PARENT_NOT_FOUND
      || result == MATCH_SAME_FILE
      || result == MATCH_FILE_NAME_OVERFLOW)
    {
        return  -1;
    }

    char *file_name = (char *)result;
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
    select_node->file.type = FT_FILE;
    select_node->file.start_lba = -1;
    select_node->status = NODE_USED;
    kmemcpy(select_node->file.name,file_name,kstrlen(file_name) + 1);
    INIT_LIST_HEAD(&select_node->child_list);
    list_add(&select_node->parent_ll,&parent->child_list);

end:
    free(file_name);
    //kprintf("fs_create inode_no is %d \n",inode_no);
    return inode_no;
}

//we only allocate inode when create file.
int file_open(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node;

    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);

    if(result == MATCH_SAME_FILE)
    {
        //We get the file.haha
        *partition = find_part;
        return select_node->inode_no;
    }

    if(result > 0)
    {
        free((char *)result);
    }

    return -1;
}


int file_rename(const char *pathname,char *newname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node = NULL;
    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);

    if(result == MATCH_FILE_NAME_OVERFLOW
      || result != MATCH_SAME_FILE)
    {
        return -1;
    }

    if(select_node != NULL)
    {
        int ret_inode_no = select_node->inode_no;
        kmemcpy(select_node->file.name,newname,kstrlen(newname) + 1);
        return ret_inode_no;
    }

    return -1;
}

//fd is inode id.
int file_write_overlap(int inode_no,partition_data *partition,char *buff,uint32_t size)
{

    kprintf("file_write_overlap,inode_no is %d \n",inode_no);
    inode *write_node = &partition->inode_table[inode_no];
    super_block *sb = partition->super_block;
    disk *hd = partition->hd;

    int data_bitmap_length = sb->inode_bitmap_sects*SECTOR_SIZE;
    int start_lba;// = sb->data_start_lba + write_node->start_lba;
    kprintf("file_write_overlap trace1 \n");
    if(write_node->file.start_lba == -1)
    {
        //new file,need allocate from block bitmap
        int scetor_no = scan_bit_condition(partition->data_bitmap,NODE_UNUSED,data_bitmap_length);
        write_node->file.start_lba = sb->data_start_lba + scetor_no;
        write_node->file.start_offset = 0;
    }
    kprintf("file_write_overlap trace2 \n");
    start_lba = write_node->file.start_lba;
    file_content content;
    content.next_lba = -1;
    kprintf("file_write_overlap trace2_1 \n");
    while(start_lba != -1)
    {
        kmemset(&content,0,sizeof(file_content));
        //kprintf("file_write_overlap trace2_2 \n");
        int copy_length = 0;
        if(size - FILE_CONENT_LEN > 0)
        {
            //we should find a new conent
            int sector_no = scan_bit_condition(partition->data_bitmap,NODE_UNUSED,data_bitmap_length);
            //kprintf("file_write_overlap trace2_3 \n");
            set_bit(partition->data_bitmap,sector_no,NODE_USED);
            //kprintf("file_write_overlap trace2_4 \n");
            content.next_lba = sector_no;
            copy_length = FILE_CONENT_LEN;
        }
        else
        {
            content.next_lba = -1;
            copy_length = size;
        }
        //kprintf("file_write_overlap trace2_5 \n");
        kmemcpy(content.data,buff,copy_length);
        size -= copy_length;
        hdd_write(hd, start_lba, &content, 1);
        start_lba = content.next_lba;
        //kprintf("file_write_overlap trace2_6 \n");
    }
    kprintf("file_write_overlap trace3 \n");
    return 0;
}
