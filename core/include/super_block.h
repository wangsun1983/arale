#ifndef __SUPER_BLOCK_H__
#define __SUPER_BLOCK_H__

#include "ctype.h"



struct _super_block_ {
   uint32_t magic;
   uint32_t sec_cnt;
   uint32_t inode_cnt;
   uint32_t part_lba_base;            //this partion start lba

   uint32_t block_bitmap_lba;
   uint32_t block_bitmap_sects;

   uint32_t inode_bitmap_lba;
   uint32_t inode_bitmap_sects;

   uint32_t inode_table_lba;
   uint32_t inode_table_sects;

   uint32_t data_start_lba;
   uint32_t root_inode_no;           //root directory node no.
   uint32_t dir_entry_size;

   uint8_t  pad[460];

}__attribute__ ((packed));

typedef struct _super_block_ super_block;

#endif
