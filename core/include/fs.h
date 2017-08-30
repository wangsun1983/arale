#ifndef __FS_H__
#define __FS_H__
#include "list.h"
#include "ctype.h"
#include "fs_inode.h"
#include "super_block.h"
#include "hdd.h"

#define FILE_MAGIC 0x19590318
#define MAX_FILES_PER_PART 4096
#define BITS_PER_SECTOR 4096
#define SECTOR_SIZE 512

#define FILE_BLOCK_SIZE 4096

#define SECTOR_NUM_SB 1
#define SECTOR_NUM_FILE 4
//SECTOR_SIZE is 512
#define FILE_CONENT_LEN (SECTOR_SIZE - sizeof(uint32_t))

enum FILE_TYPE {
    FT_UNKNOWN,
    FT_FILE,
    FT_DIRECTORY
};

enum NODE_STATUS {
    NODE_UNUSED = 0,
    NODE_USED
};

/* 打开文件的选项 */
enum FILE_OPERATION_TYPE {
    O_RDONLY,	  // 只读
    O_WRONLY,	  // 只写
    O_RDWR,	  // 读写
    O_CREAT = 4	  // 创建
};


enum FILE_WRITE_MODE {
    WRITE_NORMAL = 0,
    WRITE_APPEND
};

typedef struct file_content {
    uint8_t data[FILE_CONENT_LEN];
    uint32_t next_lba;
}file_content;

//global inode_table
typedef struct partition_data {
    struct list_head ll;
    int patition_index;
    inode *inode_table;
    super_block *super_block;
    disk *hd;
    char *node_bitmap;
    char *data_bitmap;
}partition_data;

struct list_head partition_list;

void fs_init();
void fs_write(uint32_t fd,char *buffer,uint32_t size,int mode);

uint32_t fs_create(const char *pathname,int type);
void fsync_inode(partition_data *partition,int inode_no);

#endif
