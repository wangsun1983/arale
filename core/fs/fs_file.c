#include "fs_file.h"
#include "fs_utils.h"
#include "fs.h"
#include "mm.h"
#include "bitmap.h"
#include "log.h"

void file_sync_data_bitmap(partition_data *partition)
{
    super_block *sb = partition->super_block;
    char *data_bitmap = partition->data_bitmap;
    hdd_write(partition->hd,sb->block_bitmap_lba,data_bitmap,sb->block_bitmap_sects);
}

//we only allocate inode when create file.
int file_create(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node;

    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);
    //LOGD("file_create result is %d \n",result);
    if(result == MATCH_PARENT_NOT_FOUND
      || result == MATCH_SAME_FILE
      || result == MATCH_FILE_NAME_OVERFLOW)
    {
        LOGD("file create result is %d \n",result);
        return  -1;
    }

    char *file_name = (char *)result;
    int inode_no = scan_bit_condition(find_part->node_bitmap,NODE_UNUSED,MAX_FILES_PER_PART/8);
    set_bit(find_part->node_bitmap,inode_no,NODE_USED);
    *partition = find_part;
    select_node = &find_part->inode_table[inode_no];
    kmemset(select_node,0,sizeof(inode));

    select_node->inode_no = inode_no;
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
    return inode_no;
}

uint32_t file_read(uint32_t inode_no,partition_data *patition,char *buff,int buff_size,int where_to_read)
{
    inode *file_node = &patition->inode_table[inode_no];
    //LOGD("file_read start,inode_no is %d,file_node is %x,patition->inode_table is %x \n"
    //      ,inode_no,file_node,patition->inode_table);

    //LOGD("file_read 2,file_node is %x \n",file_node);
    uint32_t start_lba = file_node->file.start_lba;
    LOGD("file_node file offset is %d \n",file_node->file.offset);
    int offset_lba = where_to_read/FILE_CONENT_LEN;
    int offset_byte = where_to_read%FILE_CONENT_LEN;

    //LOGD("file_read1 is %d,offset_lba is %d \n",start_lba,offset_lba);
    //start_lba += offset_lba;
    //LOGD("file_read2 is %d ,offset_lba is %d\n",start_lba,offset_lba);

    //char *read_buff = (char *)kmalloc(sizeof(file_content));
    file_content content;
    kmemset(&content,0,sizeof(file_content));

    int read_size = 0;
    int read_lba = start_lba;
    int prev_lba = start_lba;

    //start to find start_lba
    while(offset_lba >= 0)
    {
        prev_lba = read_lba;
        if(read_lba == -1)
        {
            break;
        }
        hdd_read(patition->hd,read_lba,&content, 1);
        read_lba = content.next_lba;
        offset_lba--;
    }

    if(offset_lba > 0)
    {
         return -1;
    }

    read_lba = prev_lba;
    //hit we find the read lba
    while(buff_size > 0)
    {
        kmemset(&content,0,sizeof(file_content));

        int read_length = FILE_CONENT_LEN - offset_byte;
        //LOGD("read_length is %d \n",read_length);
        if(read_length > buff_size)
        {
            read_length = buff_size;
        }

        hdd_read(patition->hd,read_lba,&content, 1);
        kmemcpy(buff,(char *)&content,read_length);
        buff += read_length;
        buff_size -= read_length;
        offset_byte = 0;
        read_lba = content.next_lba;
        //LOGD("wangsl,red_lba is %d \n",read_lba);
        read_size += read_length;
    }

    return read_size;
}

//we only allocate inode when create file.
int file_open(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node;
    LOGD("file_open path is %s \n",pathname);
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

int file_write_append(int inode_no,partition_data *partition,char *buff,uint32_t size)
{
    //we shuld find the offset
    inode *write_node = &partition->inode_table[inode_no];
    //LOGD("file_write_append start,inode_no is %d,write_node is %x,partition->inode_table is %x \n"
    //        ,inode_no,write_node,partition->inode_table);

    super_block *sb = partition->super_block;
    disk *hd = partition->hd;
    int data_bitmap_length = sb->inode_bitmap_sects*SECTOR_SIZE;

    //LOGD("file_write_append trace1,write_node is %x \n",write_node);
    //1.find the last sector to write.
    int start_lba = write_node->file.start_lba;
    //LOGD("file start_lba is %x \n",start_lba);
    //LOGD("file_write_append trace1_0 \n");
    file_content content;
    //LOGD("file_write_append trace1_1 \n");
    int prev = start_lba;
    while(start_lba != -1)
    {
        prev = start_lba;
        //LOGD("file_write_append trace1_2 \n");
        int sector_no = start_lba - partition->super_block->block_bitmap_lba;
        kmemset(&content,0,sizeof(file_content));
        //LOGD("file_write_append trace1_3 \n");
        hdd_read(partition->hd,start_lba,&content, 1);
        //LOGD("file_write_append trace1_4 \n");
        start_lba = content.next_lba;
        //LOGD("file_write_append trace1_5 \n");
    }
    start_lba = prev;

    //LOGD("file_write_append trace2,start_lba is %x \n",start_lba);
    //we should read the file
    hdd_read(partition->hd,start_lba,&content, 1);
    char *write_buff = (char *)content.data;
    int copy_length = FILE_CONENT_LEN - write_node->file.offset;
    if(copy_length > size)
    {
        copy_length = size;
    }

    if(copy_length > 0)
    {
        kmemcpy(write_buff + write_node->file.offset,buff,copy_length);
        size -= copy_length;
        buff += copy_length;
        write_node->file.offset += copy_length;
    }

    if(size == 0)
    {
         hdd_write(hd, start_lba, &content, 1);
         goto end;
    }

    while(size > 0)
    {
        int sector_no = scan_bit_condition(partition->data_bitmap,NODE_UNUSED,data_bitmap_length);
        set_bit(partition->data_bitmap,sector_no,NODE_USED);
        content.next_lba = sb->block_bitmap_lba + sector_no;
        hdd_write(hd, start_lba, &content, 1);

        start_lba = content.next_lba;
        kmemset(&content,0,sizeof(file_content));
        content.next_lba = -1;

        if(size >= FILE_CONENT_LEN )
        {
            copy_length = FILE_CONENT_LEN;
        }
        else
        {
            copy_length = size;
        }

        write_buff = (char *)content.data;
        kmemcpy(write_buff,buff,copy_length);
        size -= copy_length;
        buff += copy_length;
    }
    //last data
    LOGD("file_write_append trace4 \n");
    hdd_write(hd, start_lba, &content, 1);
    file_sync_data_bitmap(partition);

end:
    return 0;
}

//fd is inode id.
int file_write_overlap(int inode_no,partition_data *partition,char *buff,uint32_t size)
{

    //LOGD("file_write_overlap,inode_no is %d \n",inode_no);
    inode *write_node = &partition->inode_table[inode_no];
    super_block *sb = partition->super_block;
    disk *hd = partition->hd;
    //because write overlap,so we clean first;

    //LOGD("file_write_overlap start,inode_no is %d,write_node is %x,partition->inode_table is %x \n"
    //        ,inode_no,write_node,partition->inode_table);

    int data_bitmap_length = sb->inode_bitmap_sects*SECTOR_SIZE;
    uint32_t start_lba;
    uint32_t next_lba;
    uint32_t read_length;// = sb->data_start_lba + write_node->start_lba;
    //LOGD("file_write_overlap trace1,sb->data_start_lba is %d \n",sb->data_start_lba);
    bool is_first_write = false;
    if(write_node->file.start_lba == -1)
    {
        //new file,need allocate from block bitmap
        int sector_no = scan_bit_condition(partition->data_bitmap,NODE_UNUSED,data_bitmap_length);
        set_bit(partition->data_bitmap,sector_no,NODE_USED);
        write_node->file.start_lba = sb->data_start_lba + sector_no;
        write_node->file.offset = 0;
        //sync data_bitmap
        is_first_write = true;
    }

    start_lba = write_node->file.start_lba;
    //LOGD("file_write_overlap trace2,start lba is %d,sb->data_start_lba is %d \n",start_lba,sb->data_start_lba);
    file_content content;
    kmemset(&content,0,sizeof(file_content));

    while(size > 0)
    {
        if(start_lba != -1 && start_lba >= sb->data_start_lba)
        {
            hdd_read(hd,start_lba,&content, 1);
        }

        next_lba = content.next_lba;
        //LOGD("file_write_overlap trace2_1,next_lba is %d \n",next_lba);
        if(size > FILE_CONENT_LEN)
        {
            read_length = FILE_CONENT_LEN;
            if(next_lba < sb->data_start_lba || next_lba == -1)
            {
                //we should find a new conent
                int sector_no = scan_bit_condition(partition->data_bitmap,NODE_UNUSED,data_bitmap_length);
                //LOGD("file_write_overlap trace2_3,sector_no is %d,next_lba is %d \n",sector_no,next_lba);
                set_bit(partition->data_bitmap,sector_no,NODE_USED);
                //LOGD("file_write_overlap trace2_4 \n");
                content.next_lba = sb->data_start_lba + sector_no;
                next_lba = content.next_lba;
            }
            write_node->file.offset = 0;
        }
        else
        {
            read_length = size;
            content.next_lba = -1;
            write_node->file.offset = read_length;
        }
        kmemcpy((char *)&content,buff,read_length);
        //LOGD("file_write ,start_lba is %d,read_length is %d,content next lba is %d \n",start_lba,read_length,content.next_lba);
        hdd_write(hd,start_lba,&content,1);
        kmemset(&content,0,sizeof(file_content));

        buff += read_length;
        size -= read_length;
        start_lba = next_lba;
    }

    //need clean the other data bitmap
    char *clear_buff = (char *)kmalloc(SECTOR_SIZE);
    kmemset(clear_buff,0,SECTOR_SIZE);
    LOGD("wangsl,next_lba is %d,sb->data_start_lba is %d \n",next_lba,sb->data_start_lba);

    while(next_lba != -1 && next_lba >= sb->data_start_lba)
    {
        LOGD("wangsl,next_lba trace1 \n");
        kmemset(&content,0,sizeof(file_content));
        int sector = next_lba - sb->data_start_lba;
        LOGD("wangsl,sector trace1 sector is %d \n",sector);
        set_bit(partition->data_bitmap,sector,NODE_UNUSED);
        hdd_read(hd,next_lba,&content, 1);
        hdd_write(hd,next_lba,clear_buff,1);
        next_lba = content.next_lba;
        LOGD("wangsl,sector trace1 next_lba is %d \n",next_lba);
    }

    //LOGD("file_write_overlap trace3,offset is %d \n",write_node->file.offset);
    file_sync_data_bitmap(partition);
    return 0;
}


int file_remove(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    inode *select_node = NULL;
    addr_t result = file_path_match(pathname,&find_part,&parent,&select_node);
    //LOGD("file_remove result is %d \n",result);
    if(result == MATCH_FILE_NAME_OVERFLOW
      ||result != MATCH_SAME_FILE)
    {
        return -1;
    }

    if(select_node != NULL)
    {
        //use a loop to delete all the data
        //LOGD("file_remove trace1\n");
        int start_lba = select_node->file.start_lba;
        file_content content;
        char *clear_buff = (char *)kmalloc(SECTOR_SIZE);
        kmemset(clear_buff,0,SECTOR_SIZE);
        //LOGD("file_remove trace1_1,start_lba\n");
        while(start_lba != -1)
        {
            //LOGD("file_remove trace1_2\n");
            int sector_no = start_lba - find_part->super_block->block_bitmap_lba;
            //LOGD("file_remove trace1_3\n");
            set_bit(find_part->data_bitmap,sector_no,NODE_USED);
            //LOGD("file_remove trace1_4\n");
            kmemset(&content,0,sizeof(file_content));
            //LOGD("file_remove trace1_5\n");
            hdd_read(find_part->hd,start_lba,&content, 1);
            hdd_write(find_part->hd,start_lba,clear_buff,1);
            start_lba = content.next_lba;
            //LOGD("file_remove trace1_6,start_lba is %d\n",start_lba);
        }
        //LOGD("file_remove trace2\n");
        free(clear_buff);
        set_bit(find_part->data_bitmap,select_node->inode_no,NODE_UNUSED);
        *partition = find_part;
        list_del(&select_node->parent_ll);
        kmemset(select_node,0,sizeof(inode));
        file_sync_data_bitmap(find_part);
        LOGD("file_remove trace3\n");
        return select_node->inode_no;
    }

    return -1;
}
