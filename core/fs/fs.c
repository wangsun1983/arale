#include "hdd.h"
#include "fs.h"
#include "super_block.h"
#include "inode.h"
#include "dir.h"
#include "mm.h"
#include "klibc.h"

extern int hdd_channel_cnt;
extern ide_channel hdd_channels[2];

void partition_format(partition* part);

void fs_init()
{
    uint8_t channel_no = 0;
    uint8_t dev_no = 0;
    uint8_t part_idx = 0;

    super_block* sb_buf = (super_block*)kmalloc(SECTOR_SIZE);
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
                    	  partition_format(part);
                    }
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
                          partition_format(part);
                      }
                 }
            }
            dev_no++;
        }
        channel_no++;
    }
}


/*
*
*     |mbr|super block|block_bitmap|inode_bitmap|data_start|
*/
void partition_format(partition* part)
{
    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
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
    sb.dir_entry_size = sizeof(dir_entry);

    struct disk* hd = part->my_disk;
    //write super block info into sector 1
    hdd_write(hd, part->start_lba + 1, &sb, 1);
    //write block bitmap
    uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects ? sb.block_bitmap_sects : sb.inode_bitmap_sects);
    buf_size = (buf_size >= sb.inode_table_sects ? buf_size : sb.inode_table_sects) * SECTOR_SIZE;
    uint8_t* buf = (uint8_t*)kmalloc(buf_size);
    buf[0] |= 0x01;
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
    uint8_t  block_bitmap_last_bit  = block_bitmap_bit_len % 8;
    uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);
    kmemset(&buf[block_bitmap_last_byte], 0xff, last_size);

    uint8_t bit_idx = 0;
    while (bit_idx <= block_bitmap_last_bit)
    {
        buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
    }

    hdd_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    kmemset(buf, 0, buf_size);
    hdd_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

    buf[0] |= 0x1;

    kmemset(buf, 0, buf_size);
    struct inode* i = (struct inode*)buf;
    i->i_size = sb.dir_entry_size * 2;
    i->i_no = 0;
    i->i_sectors[0] = sb.data_start_lba;
    hdd_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

    kmemset(buf, 0, buf_size);
    struct dir_entry* p_de = (struct dir_entry*)buf;

    kmemcpy(p_de->filename, ".", 1);
    p_de->inode_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

    kmemcpy(p_de->filename, "..", 2);
    p_de->inode_no = 0;
    p_de->f_type = FT_DIRECTORY;

    hdd_write(hd, sb.data_start_lba, buf, 1);

    free(buf);
}

void fs_write(file_struct *file,char *buffer,uint32_t size,int mode)
{
    //TODO
}
