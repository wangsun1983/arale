#include "hdd.h"
#include "fs.h"
#include "super_block.h"
#include "inode.h"
#include "fs_dir.h"
#include "mm.h"
#include "klibc.h"

extern int hdd_channel_cnt;
extern ide_channel hdd_channels[2];

char *root_name = "root";

void partition_format(partition* part,int no);
partition_data* partition_load(partition* part);

void fs_init()
{
    uint8_t channel_no = 0;
    uint8_t dev_no = 0;
    uint8_t part_idx = 0;

    //init partition_list
    INIT_LIST_HEAD(&partition_list);

    super_block* sb_buf = (super_block*)kmalloc(SECTOR_SIZE);
    int patition_no = 0;

    while (channel_no < hdd_channel_cnt)
    {
        dev_no = 0;
        while(dev_no < 2)
        {
            disk* hd = &hdd_channels[channel_no].devices[dev_no];
            partition* part = hd->prim_parts;

            int index = 0;
            for(;index < HDD_PARTION_PRIME_NUM;index++)
            {
                if(part->sec_cnt > 0)
                {
                    kmemset(sb_buf, 0, SECTOR_SIZE);
	                  hdd_read(hd, part->start_lba + 1, sb_buf, 1);
                    if (sb_buf->magic != FILE_MAGIC)
                    {
                    	  partition_format(part,patition_no);
                        patition_no++;
                    }

                    partition_data *partion = partition_load(part);
                    partion->hd = hd;
                    list_add(&partion->ll,&partition_list);
                }
                part++;
            }

            part = hd->logic_parts;
            for(index =0;index < HDD_PARTION_LOGIC_NUM;index++)
            {
                 if(part->sec_cnt > 0)
                 {
                      kmemset(sb_buf, 0, SECTOR_SIZE);
                      hdd_read(hd, part->start_lba + 1, sb_buf, 1);
                      if (sb_buf->magic != FILE_MAGIC)
                      {
                          partition_format(part,patition_no);
                          patition_no++;
                      }

                      partition_data *partion = partition_load(part);
                      partion->hd = hd;
                      list_add(&partion->ll,&partition_list);
                 }
            }
            dev_no++;
        }
        channel_no++;
    }
}


/*
*
*     |mbr|super block|block_bitmap|inode_bitmap|inode data|data_start|
*/
void partition_format(partition* part,int no)
{
    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    //uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
    uint32_t inode_bitmap_sects = 1; //we use 1 sector to inode_bitmap,so the max file is 4096
    uint32_t inode_table_sects = DIV_ROUND_UP(((sizeof(inode) * MAX_FILES_PER_PART)), SECTOR_SIZE);

    uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    uint32_t block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);

    super_block sb;
    sb.magic = 0x19590318;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2;
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;

    //kprintf("wangsl,write sb.inode_bitmap_lba is %d \n",sb.inode_bitmap_lba);

    //sb.dir_entry_size = sizeof(dir_entry);
    struct disk* hd = part->my_disk;
    //step1:write super block info into sector 1
    hdd_write(hd, part->start_lba + 1, &sb, 1);

    //we create root inode
    inode *root = (inode *)kmalloc(sizeof(inode));
    root->file.type = FT_DIRECTORY;
    //root->file.name = root_name;
    //kmemcpy(root->file.name,root_name,kstrlen(root_name) + 1);
    ksprintf(root->file.name,"root%d",no);
    //kprintf("write root name is %s \n",root->file.name);
    root->inode_no = 0;
    root->status = NODE_USED;

    //step2:write block_bitmap
    char *buff = kmalloc(block_bitmap_sects * SECTOR_SIZE);
    kmemset(buff,0,block_bitmap_sects * SECTOR_SIZE);
    //because only root directory exists,so there is no need to write
    //block data.
    hdd_write(hd,sb.block_bitmap_lba,buff,block_bitmap_sects);
    free(buff);

    //step3:write inode bitmap
    buff = kmalloc(inode_bitmap_sects * SECTOR_SIZE);
    kmemset(buff,0,inode_bitmap_sects * SECTOR_SIZE);
    //clean sectors

    hdd_write(hd,sb.inode_bitmap_lba,buff,inode_bitmap_sects);
    free(buff);

    char *inode_bitmap = (char *)create_bitmap(MAX_FILES_PER_PART);
    set_bit(inode_bitmap,0,NODE_USED);
    hdd_write(hd,sb.inode_bitmap_lba,inode_bitmap,1);

    //step4:write root node
    buff = kmalloc(inode_table_sects * SECTOR_SIZE);
    kmemset(buff,0,inode_table_sects * SECTOR_SIZE);
    hdd_write(hd,sb.inode_table_lba,buff,inode_table_sects);
    //write root node
    hdd_write(hd,sb.inode_table_lba,root,1);
    free(root);
}

partition_data* partition_load(partition* part)
{
    struct disk* hd = part->my_disk;

    partition_data *partion = (partition_data*)kmalloc(sizeof(partition_data));

    //load super_block;
    super_block *sb = (super_block *)kmalloc(sizeof(super_block));
    hdd_read(hd, part->start_lba + 1, sb, 1);

    char *node_bitmap = (char *)kmalloc(MAX_FILES_PER_PART);

    int inode_count = count_bit(node_bitmap,MAX_FILES_PER_PART,NODE_USED);
    kprintf("wangsl,trace1 inode_count is %d,node_bitmap[0] is %d \n",inode_count,node_bitmap[0]);

    hdd_read(hd,sb->inode_bitmap_lba,node_bitmap,1);
    inode_count = count_bit(node_bitmap,MAX_FILES_PER_PART,NODE_USED);
    kprintf("wangsl,trace2 inode_count is %d,node_bitmap[0] is %d \n",inode_count,node_bitmap[0]);

    inode *node_table = (inode *)kmalloc(sizeof(inode) *MAX_FILES_PER_PART);
    hdd_read(hd,sb->inode_table_lba,node_table,sb->inode_table_sects);

    //first node is root_node
    inode *root_node = &node_table[0];

//#ifdef DUMP_ALL_NODE
    kprintf("root name is %s ,type is %d,status is %d \n",root_node->file.name,root_node->file.type,root_node->status);
    int dumpIndex = 0;
    for(;dumpIndex < MAX_FILES_PER_PART;dumpIndex++) {
        inode *node = &node_table[dumpIndex];
        if(node->status == NODE_USED)
        {
            kprintf("node name is %s ,type is %d \n",node->file.name,node->file.type);
        }
    }
//#endif

    //start init inode table,init all the list_head
    int index = 0;
    for(;index < MAX_FILES_PER_PART;index++)
    {
        if(get_bit(node_bitmap,index) == NODE_USED)
        {
            inode *node = &node_table[index];

            if(node->init_status == INODE_IDLE)
            {
                 INIT_LIST_HEAD(&node->child_list);
                 node->init_status = INODE_INITED;
            }

            if(node->inode_no != 0)
            {
                inode *parent = &node_table[node->parent_no];
                if(parent->init_status == INODE_IDLE)
                {
                    INIT_LIST_HEAD(&parent->child_list);
                    parent->init_status = INODE_INITED;
                }
                list_add(&node->parent_ll,&parent->child_list);
            }
        }
    }

    partion->inode_table = node_table;
    partion->super_block = sb;
    partion->node_bitmap = node_bitmap;

    return partion;
}

void fsync_inode(partition_data *partition)
{
    //refresh inode bitmap
    hdd_write(partition->hd,
      partition->super_block->inode_bitmap_lba,
      partition->node_bitmap,
      1);

    //refresh inode table

}

uint32_t fs_open(const char* pathname, uint8_t flags)
{


}

uint32_t fs_create(const char *pathname,int type)
{
    switch(type)
    {
        case FT_FILE:
        //TODO
        break;

        case FT_DIRECTORY:
        {
            partition_data *patition = dir_create(pathname);
            fsync_inode(patition);
        }
        break;
    }

    return 0;
}

void fs_write(file_struct *file,char *buffer,uint32_t size,int mode)
{
    //TODO
}
