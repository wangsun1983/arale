#include "fs_inode.h"
#include "fs.h"
#include "mm.h"

enum SECTOR_STATUS
{
    SECTOR_SINGLE = 0,
    SECTOR_CROSS
};

typedef struct sector_position
{
    int postion_type;	       //whether inode data is writed in to sectors
    uint32_t sec_lba;	   //inode sector No
    uint32_t off_size;	   //offset of inode in sector
}sector_position;

void locate_inode_sector(partition_data *partition,int inode_no,sector_position *position)
{
    super_block *sb = partition->super_block;
    uint32_t inode_data_start_lba = sb->inode_table_lba;
    int prev_inodes_start_size = sizeof(inode)*inode_no;
    //kprintf("prev_inodes_start_size is %d \n",prev_inodes_start_size);

    int prev_inodes_start_sector = (prev_inodes_start_size/SECTOR_SIZE);
    int prev_inodes_offset = prev_inodes_start_size%SECTOR_SIZE;

    if((prev_inodes_offset + sizeof(inode)) <= SECTOR_SIZE)
    {
        position->postion_type = SECTOR_SINGLE;
    }
    else
    {
        position->postion_type = SECTOR_CROSS;
    }

    position->sec_lba = prev_inodes_start_sector;
    position->off_size = prev_inodes_offset;
}

//we should compute where to write node data;
void fsync_inode(partition_data *partition,int inode_no)
{
    //refresh inode bitmap
    //kprintf("fsync_inode start \n");
    hdd_write(partition->hd,
      partition->super_block->inode_bitmap_lba,
      partition->node_bitmap,
      1);

    //refresh inode table
    inode *node = &partition->inode_table[inode_no];

    sector_position position;
    locate_inode_sector(partition,inode_no,&position);
    //kprintf("inode_table_lba is %d node status is %d\n",partition->super_block->inode_table_lba,node->status);
    //kprintf("node file name is %s sec_lba is %d offsize is %d\n",node->file.name,position.sec_lba,position.off_size);

    //start to write sector~~~~
    //first sector should be
   char *buff = (char *)kmalloc(SECTOR_SIZE);

   if(position.postion_type == SECTOR_SINGLE)
   {
       //kprintf("fsync_inode SINGLE \n");
       hdd_read(partition->hd,
            partition->super_block->inode_table_lba + position.sec_lba,
            buff, 1);

       kmemcpy(buff + position.off_size,(char *)node,sizeof(inode));

       hdd_write(partition->hd,
            partition->super_block->inode_table_lba + position.sec_lba,
            buff,
            1);
   }
   else if(position.postion_type == SECTOR_CROSS)
   {
       kprintf("fsync_inode CROSS 1\n");
       //Step1.Read first sector
       hdd_read(partition->hd,
            partition->super_block->inode_table_lba + position.sec_lba,
            buff, 1);

       //Step2.write first data to sector
       int first_sector_write_bytes = SECTOR_SIZE - position.off_size;

       kmemcpy(buff + position.off_size,node,first_sector_write_bytes);
       hdd_write(partition->hd,
            partition->super_block->inode_table_lba + position.sec_lba,
            buff,
            1);
       position.sec_lba++;

       int write_sector = sizeof(inode)/SECTOR_SIZE;
       char *in = (char *)node + first_sector_write_bytes;
       kprintf("fsync_inode CROSS 2\n");
       while(write_sector > 1)
       {
           kmemcpy(buff,in,SECTOR_SIZE);
           hdd_write(partition->hd,
                  partition->super_block->inode_table_lba + position.sec_lba,
                  buff,
                  1);
           write_sector--;
           position.sec_lba++;
           in += SECTOR_SIZE;
       }

       //Step3.write last sector
       if((addr_t)node > (addr_t)in)
       {
           kprintf("fsync_inode CROSS 3\n");
           int size = (addr_t)node - (addr_t)in;
           hdd_read(partition->hd,
                partition->super_block->inode_table_lba + position.sec_lba,
                buff, 1);

           kmemcpy(buff,in,size);
           hdd_write(partition->hd,
                partition->super_block->inode_table_lba + position.sec_lba,
                buff,
                1);
       }
   }

   free(buff);
}
